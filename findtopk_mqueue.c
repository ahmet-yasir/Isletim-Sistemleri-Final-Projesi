#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <mqueue.h>


int selectionSort(int arr[], int size,int k) {  // diziyi sıralayıp en büyük k. elemenı döndürmek için
    int i, j, minIndex;
    for (i = 0; i < size-1; i++) {
        minIndex = i;
        for (j = i+1; j < size; j++) {
            if (arr[j] < arr[minIndex]) {
                minIndex = j;
            }
        }
        // Swap işlemi
        int temp = arr[minIndex];
        arr[minIndex] = arr[i];
        arr[i] = temp;
    }
    return arr[size-k];
}


void process_function(int process_id, int k, mqd_t mq, char *readfilename) {
    int size = 0;
    int value;

    FILE *fp = fopen(readfilename, "r"); // okunacak dosya
    if (!fp) {
        perror("Dosya açılamadı");
        exit(1);
    }
    while (fscanf(fp, "%d", &value) == 1) {  // Okunacak dosyanın boyutunu bulmak için 
        size++;
    }
    fseek(fp,0,SEEK_SET); // okuma konumunu en başa almak için 
    int *arr = (int *)malloc(size * sizeof(int));  // Bulunan boyuta göre bir dizi oluşturmak için
    for (int i = 0; i < size; i++) { // Sayıları bulup diziye kaydetmek için
        fscanf(fp, "%d", &value);
        arr[i] = value;
    }
    int num = selectionSort(arr, size, k);  // k. en büyük sayıyı bulup geri döndürmek için
    char message[20];   // Bulduğumuz değerin string halini saklamak için 
    snprintf(message, 20, "%d", num); // int değeri stringe çevirmek için 
    if(mq_send(mq, message, sizeof(message), 0)== -1){ // Bulduğumuz değeri parent processe göndermek için 
	printf("hata");
    }
    fclose(fp);
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Kullanım: findtopk_mqueue <k> <N> <infile1> ...<infileN> <outfile>\n");
        return 1;
    }

    int k = atoi(argv[1]);
    if (k < 1 || k > 1000) {
        printf("K değeri 1 ile 1000 arasında olmalıdır.\n");
        return 1;
    }

    int N = atoi(argv[2]);
    if (N < 1 || N > 5) {
        printf("N değeri 1 ile 5 arasında olmalıdır.\n");
        return 1;
    }
    struct mq_attr attr;      // Kuyruğun yapısal özellikleri 
    attr.mq_flags = 0;        // 0 veya O_NONBLOCK
    attr.mq_maxmsg = 5;       // Kuyruktaki maksimum mesaj sayısı
    attr.mq_msgsize = 20;     // Her bir mesajın maksimum boyutu
    attr.mq_curmsgs = 0;      // Başlangıçta kuyrukta bulunan mesaj sayısı
    mqd_t mq = mq_open("/a_mq", O_CREAT | O_RDWR, 0666, &attr); // Kuyruğu oluşturmak için 
    if (mq == (mqd_t)-1) {
        perror("Mesaj kuyruğu oluşturma başarısız oldu");
        exit(1);
    }
    
    pid_t pids[N];
    for (int i = 0; i < N; i++) { // Child processleri oluşturmak için
        pids[i] = fork();   //parent processden süreci kopyalayıp child process olarak atamak için 
        if (pids[i] == 0) {// pids[i]==0 olması bu processin child process olduğunu gösterir 0dan küçükse hata mesajı 0dan büyükse parent process olduğunu gösterir
            process_function(i, k, mq, argv[i + 3]); // Child process
            exit(0);
        }
    }

    for (int i = 0; i < N; i++) { //Child processlerin bitmesini beklemek için 
        wait(NULL);
    }
    
    char *outfilename = argv[argc-1];
        FILE *fp = fopen(outfilename, "w");
    if (!fp) {
        perror("Dosya açılamadı");
        exit(1);
    }
    char message[20]; // kuyruktan gelen değeri yazmak için 
    int arr[N];
    for (int i = 0; i < N; i++) {
        if(mq_receive(mq, message , 20, NULL)==-1){ perror("mq_receive");} // kuyruktan mesajı almak için 
	arr[i] = atoi(message); // stringi int değere çevirmek ve diziye kaydetmek için 
    }
    int minIndex;
    for (int i = 0; i < N-1; i++) { // elde ettiğimiz diziyi sıralamak için
        minIndex = i;
        for (int j = i+1; j < N; j++) {
            if (arr[j] > arr[minIndex]) {
                minIndex = j;
            }
        }
        // Swap işlemi
        int temp = arr[minIndex];
        arr[minIndex] = arr[i];
        arr[i] = temp;
    }
    for(int i =0 ; i< N; i++){ // elde edilen diziyi çıkış dosyasına yazdırmak için 
        fprintf(fp,"%d\n",arr[i]);
    }
    fclose(fp);

    mq_close(mq); // kuyruğu kapatmak için 
    mq_unlink("/a_mq"); // kuyruğu kaldırmak için 

    return 1;
}



