#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

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

void process_function(int process_id,int k, char *outfilename, char *readfilename) {
  int size=0;
  int value;
  
  FILE *fp = fopen(readfilename, "r");  // okunacak dosya
  if (!fp) {
    perror("Dosya açılamadı");
    exit(1);
  }
  while (fscanf(fp, "%d", &value) == 1) {   // Okunacak dosyanın boyutunu bulmak için 
    size++;
  }
  fseek(fp,0,SEEK_SET);  // okuma konumunu en başa almak için
  int *arr=(int*)malloc(size*sizeof(int)); // Bulunan boyuta göre bir dizi oluşturmak için
  for(int i=0;i<size;i++ ) {   // Sayıları bulup intermediate dosyaya ve diziye kaydetmek için
    fscanf(fp, "%d",&value);
    arr[i]=value;
  }
  int num=selectionSort(arr,size,k); // k. en büyük sayıyı bulup geri döndürmek için
  FILE *fp1 = fopen(outfilename, "w");  // Ara dosya
  if (!fp1) {
    perror("Dosya açılamadı");
    exit(1);
  }
  fprintf(fp1,"%d\n",num); // Bulunan değeri ara dosyaya yazmak için
  fclose(fp1);
  fclose(fp);
}


int main(int argc, char *argv[]) {  // argc değeri fonksiyonu çalıştırırken eklediğimiz parametrelerin sayısı argv parametrelerin string hali
  if (argc < 5) {
    printf("Kullanım: findtopk <k> <N> <infile1> ...<infileN> <outfile>\n");
    return 1;
  }
  int k = atoi(argv[1]); // k değerini kontrol etmek için 
  if (k < 1 || k > 1000) {
    printf("K değeri 1 ile 1000 arasında olmalıdır.\n");
    return 1;
  }


  int N = atoi(argv[2]);  // N değerini kontrol etmek için 
  if (N < 1 || N > 5) {
    printf("N değeri 1 ile 5 arasında olmalıdır.\n");
    return 1;
  }

  
  char *intermediate_filenames[N]; // ara dosyaların adını tutmak için 
  for (int i = 0; i < N; i++) {   // ara dosyaları oluşturmak için 
    intermediate_filenames[i] = malloc(100 * sizeof(char));  // dosya ismi için bellekte yer ayırmak için
    sprintf(intermediate_filenames[i], "intermediate%d", i);  // dosya ismini belirlemek için 
    FILE *fp = fopen(intermediate_filenames[i], "w"); // dosya yoksa oluşturması için 
    fclose(fp);
  }

  pid_t pids[N];   // Child processleri oluşturmak için 
  for (int i = 0; i < N; i++) {
    pids[i] = fork();  //parent processden süreci kopyalayıp child process olarak atamak için 
    if (pids[i] == 0) { // pids[i]==0 olması bu processin child process olduğunu gösterir 0 dan küçükse hata mesajı 0dan büyükse parent process olduğunu gösterir
      process_function(i, k, intermediate_filenames[i], argv[i+3]); // Child process
      exit(0);
    }
  }

  for (int i = 0; i < N; i++) {   //Child processlerin bitmesini beklemek için 
    wait(NULL);
  }
  int arr[N];
  char *outfile=argv[argc-1];
  FILE *fp1 = fopen(outfile, "w"); 
  for (int i = 0; i < N; i++) {   //oluşturulan tüm ara dosyaları okuyup K. en büyük sayıyı bulmak için
    FILE *fp = fopen(intermediate_filenames[i], "r");
    int value;
    fscanf(fp, "%d", &value);  // ara dosyalardaki k. en büyük değeri okuyup value olarak kaydetmek için
    arr[i]=value;
    fclose(fp);
  }
  int minIndex;           // ara dosyalardan elde edilen diziyi sıralamak için 
    for (int i = 0; i < N-1; i++) {
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
    for(int i =0 ; i< N; i++){   // diziyi çıkış dosyasına yazdırmak için 
        fprintf(fp1,"%d\n",arr[i]);
    }
    for(int i = 0; i<N; i++){   // ara dsoyaları silmek için 
    	remove(intermediate_filenames[i]);
    }


fclose(fp1);
return 1;
} 


