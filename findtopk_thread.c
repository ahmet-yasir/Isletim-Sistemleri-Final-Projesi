#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>


// Global değişkenler
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int k;
int result[5];
int result_index = 0;

// Fonksiyon imzaları
int selectionSort(int arr[], int size, int k);
void *process_function(void *arg);

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

void *process_function(void *arg) {
    char *readfilename = (char *)arg;

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
    fseek(fp,0,SEEK_SET);  // okuma konumunu en başa almak için 
    int *arr = (int *)malloc(size * sizeof(int));  // Bulunan boyuta göre bir dizi oluşturmak için
    for (int i = 0; i < size; i++) {  // Sayıları bulup diziye kaydetmek için
        fscanf(fp, "%d", &value);
        arr[i] = value;
    }

    fclose(fp);

    pthread_mutex_lock(&mutex);    // Kritik bölgeye giriş

    int num = selectionSort(arr, size, k);
    result[result_index++] = num;

    pthread_mutex_unlock(&mutex);    // Kritik bölgeden çıkış

    pthread_exit(NULL); // Thread'in işi bittiğinde sonlanması
}

int main(int argc, char *argv[]) {  // argc değeri fonksiyonu çalıştırırken eklediğimiz parametrelerin sayısı argv parametrelerin string hali
    if (argc < 5) {
        printf("Kullanım: findtopk_thread <k> <N> <infile1> ...<infileN> <outfile>\n");
        return 1;
    }

    k = atoi(argv[1]);  // k değerini kontrol etmek için
    if (k < 1 || k > 1000) {
        printf("K değeri 1 ile 1000 arasında olmalıdır.\n");
        return 1;
    }

    int N = atoi(argv[2]);  // N değerini kontrol etmek için
    if (N < 1 || N > 5) {
        printf("N değeri 1 ile 5 arasında olmalıdır.\n");
        return 1;
    }

    pthread_t threads[N];    // Thread dizisi

    // Giriş dosyaları için thread'leri oluşturmak için
    for (int i = 0; i < N; i++) {
        if (pthread_create(&threads[i], NULL, process_function, (void *)argv[i + 3]) != 0) {
            perror("Thread oluşturma hatası");
            exit(1);
        }
    }

    // Thread'leri bekleyerek programın bitmesini sağlamak için
    for (int i = 0; i < N; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Thread beklenemedi");
            exit(1);
        }
    }

    // Sonuçları dosyaya yazmak için
    FILE *fp1 = fopen(argv[argc - 1], "w");
    if (!fp1) {
        perror("Dosya açılamadı");
        exit(1);
    }
    int minIndex;    // elde ettiğimiz diziyi sıralamak için 
    for (int i = 0; i < N-1; i++) {
        minIndex = i;
        for (int j = i+1; j < N; j++) {
            if (result[j] > result[minIndex]) {
                minIndex = j;
            }
        }
        // Swap işlemi
        int temp = result[minIndex];
        result[minIndex] = result[i];
        result[i] = temp;
    }
    for (int i = 0; i < result_index; i++) { // diziyi çıkış dosyasına yazdırmak için 
        fprintf(fp1, "%d\n", result[i]);
    }

    fclose(fp1);

    return 0;
}





