build: findtopk.c findtopk_mqueue.c findtopk_thread.c
	gcc findtopk_thread.c -o findtopk_thread.exe -pthread
	gcc findtopk_mqueue.c -o findtopk_mqueue.exe -lrt
	gcc findtopk.c -o findtopk.exe
