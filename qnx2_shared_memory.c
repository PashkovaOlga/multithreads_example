#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <process.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/time.h>
#include <sched.h>


#define SEMAPHORE_NAME "/my_named_semaphore" //имя иненованного семафора
#define SHARED_MEMORY_OBJECT_NAME "shared_memory" // имя разделяемой памяти
#define CLOSE  4



//структура которая хранится в разделяемой памяти между процессами
struct shared_info{
	pthread_rwlock_t rwl;  //блокировка чтения записи
	bool stop;  //флаг остановки процессов
	sem_t *sem; //указатель на им. семафор
};

//закрытие файла разделяемой памяти
void closeShare(int fd_share)
{
	close(fd_share);
	//очищение разделяемой памяти
	if ( fd_share == CLOSE ) {
	    shm_unlink(SHARED_MEMORY_OBJECT_NAME);
	}
}

void reader()
{
	pid_t pid = getpid();
	//узнаем параметры процесса
	struct sched_param param;
	 sched_getparam(pid, &param);
	 //устанавливаем диспетчеризацию RR
	int rez = sched_setscheduler( pid, SCHED_RR , &param );
	if(rez == -1)
	{
		fprintf( stderr, "set scheduler failed:%s\n",
		strerror( errno ) );
		exit( EXIT_FAILURE);
	}

	int D = 6000;
	uint T;
	uint seek = 0;

		//указатель на разделяемую память
			struct shared_info *ptr_share;
			int fd_share = shm_open( SHARED_MEMORY_OBJECT_NAME, O_RDWR | O_CREAT, 0777 );
			    if( fd_share == -1 ) {
			        fprintf( stderr, "Open failed:%s\n",
			            strerror( errno ) );
			        exit( EXIT_FAILURE);
			    }

			    //присваеваем указателю адрес открываемой разделяемой памяти
				ptr_share = mmap( 0, sizeof( *ptr_share ),
			            PROT_READ | PROT_WRITE,
			            MAP_SHARED, fd_share, 0 );
			    if( ptr_share == MAP_FAILED ) {
			        fprintf( stderr, "incrementor: mmap failed: %s\n",
			            strerror( errno ) );
			        exit( EXIT_FAILURE);
			    }

			    FILE *file;
			   //открываем файл для чтения чисел
			   if ((file = fopen("output_lab2.txt","r")) == NULL){
			    	printf("Error can't create output file\n");
			    	exit (EXIT_FAILURE);}

			   	int c;
			   	//пока процесс не остановлен
			    while(ptr_share->stop)
			    {
			    	//ждем на семафоре
			    	sem_wait(ptr_share->sem);
			    	//блокировка на чтение
			    	pthread_rwlock_rdlock(&ptr_share->rwl);

			    	//устанавливаем указательна начало файла
			    	fseek(file, 0, SEEK_SET);
			    	printf("\nRead pid: %d-------------------------------------------\n", pid);

			    	//считываем файл, пока не дойдем до конца
			        while( (c = getc( file )) != EOF ) {
			            printf("%c", c);
			        }

			        printf("\nStop read ------------------------------------------------\n");
			    	//снимаем блокировку на ч/з
			        pthread_rwlock_unlock(&ptr_share->rwl);
			        //отдаем семафор
			    	sem_post(ptr_share->sem);

			    	//задержка процесса
			    	T = rand_r(&seek) % D;
			    	delay(T);
			    }

			    //закрываем файл
			    fclose(file);
			    //уничтожаем блокировку ч/з
			    pthread_rwlock_destroy(&ptr_share->rwl);
			    closeShare(fd_share);


				//закрытие семафора
				if (sem_close(ptr_share->sem) == -1 )
				{
					//perror("sem_close");
					exit( EXIT_FAILURE);
				}

				//уничтожение семафора
				sem_unlink(SEMAPHORE_NAME);
				{
					//perror("sem_unlink");
					exit( EXIT_FAILURE);
				}

				exit (EXIT_SUCCESS);
}
void writer (int proc_no)
{
	int D = 2000;
	uint T;
	uint seek = 0;

	//указатель на разделяемую память
		struct shared_info *ptr_share;
		int fd_share = shm_open( SHARED_MEMORY_OBJECT_NAME, O_RDWR | O_CREAT, 0777 );
		    if( fd_share == -1 ) {
		        fprintf( stderr, "Open failed:%s\n",
		            strerror( errno ) );
		        exit( EXIT_FAILURE);
		    }

		    //присваеваем указателю адрес открываемой разделяемой памяти
			ptr_share = mmap( 0, sizeof( *ptr_share ),
		            PROT_READ | PROT_WRITE,
		            MAP_SHARED, fd_share, 0 );
		    if( ptr_share == MAP_FAILED ) {
		        fprintf( stderr, "incrementor: mmap failed: %s\n",
		            strerror( errno ) );
		        exit( EXIT_FAILURE);
		    }

		    FILE *file;
		    char option;

		    if(proc_no < 3){
		    	option = 'a'; //опция для открытия файла, для записи за последним символом
		    }
		    else if(proc_no == 3)
		    {
		    	option = 'w'; //опция для открытия файла для перезаписи
		    }
			//открываем файл для записи чисел
			if ((file = fopen("output_lab2.txt", &option)) == NULL){
			        printf("Error can't create output file\n");
			        exit (EXIT_FAILURE);}

		    while(ptr_share->stop)
		    {
		    	//смещение указателя файла в начало
		    	if(proc_no == 3)
		    		fseek(file, 0, SEEK_SET);
		    	//цикл работы с файлом
		    	int i;
		    	for(i=0; i<60 ; i++)
		    	{
		    		//блокировка на запись
		    		pthread_rwlock_wrlock(&ptr_share->rwl);
		    		//запись файл
		    		fprintf(file, "%d ", proc_no);
		    		//когда записано 60 символов переходим на новую строку
		    		if((proc_no == 1) && i == 59)
		    		{
		    			fprintf(file, "\n");
		    		}

		    		//смещение указателя файла на 1 байт,
		    		//чтобы процессы 1 и 2 писали друг за другом
		    		if(proc_no < 3)
		    			fseek(file, 1, SEEK_CUR);
		    		//снятие блокировки
		    		pthread_rwlock_unlock(&ptr_share->rwl);

		    		usleep(100000);

		    		//если процесс остановлен, выходим из цикла
		    		pthread_rwlock_rdlock(&ptr_share->rwl);
		    		if(!ptr_share->stop) break;
		    		pthread_rwlock_unlock(&ptr_share->rwl);

		    		//вывод напечатанной цепочки в консоль
		    		//макс j = текущему i
		    		int j;
		    		for(j = 0; j<i; j++)
		    		printf("%d ", proc_no);
		    		printf("\n");
		    	}
		    	//задержка процесса
		    	T = rand_r(&seek) % D;
		    	delay(T);
		    }

		    fclose(file);
		    pthread_rwlock_destroy(&ptr_share->rwl);
		    closeShare(fd_share);
			exit (EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {

	//если запущена программа с параметрами
		if (argc > 1)
		{
			//узнаем номер запускаемого процесса
		    char proc;
		    strcpy(&proc, argv[1]);
		    int proc_no = atoi(&proc);

		    //процессы писатели 1, 2, 3
		    if (proc_no <= 3)
		    	writer(proc_no);
		    //3 процесса читателей
		    else if(proc_no == 4)
		    	reader();
		}
		//запуск процесса без параметров
		else
		{
			printf("Lab2. Variant 22. Task 2.7\n\n");

			//при первом запуске очищаем разделяемую память
		    shm_unlink( SHARED_MEMORY_OBJECT_NAME );

		    //создаем разделяемую память
		    struct shared_info *ptr_share;
		    /* Create a new memory object */
		    int fd;
		    fd = shm_open( SHARED_MEMORY_OBJECT_NAME, O_RDWR | O_CREAT, 0777 );
		    if( fd == -1 ) {
		        fprintf( stderr, "Open failed:%s\n",
		            strerror( errno ) );
		        return EXIT_FAILURE;
		    }

		    /* Set the memory object's size */
		    if( ftruncate( fd, sizeof( *ptr_share ) ) == -1 ) {
		        fprintf( stderr, "ftruncate: %s\n",
		            strerror( errno ) );
		        return EXIT_FAILURE;
		    }

		    /* Map the memory object */
		    ptr_share = mmap( 0, sizeof( *ptr_share ),
		            PROT_READ | PROT_WRITE,
		            MAP_SHARED, fd, 0 );
		    if( ptr_share == MAP_FAILED ) {
		        fprintf( stderr, "mmap failed: %s\n",
		            strerror( errno ) );
		        return EXIT_FAILURE;
		    }

		    FILE *file;
		    //создаем файл для записи чисел
		    if ((file = fopen("output_lab2.txt","w")) == NULL){
		        printf("Error can't create output file\n");
		        exit (EXIT_FAILURE);}
		    fclose(file);

		    //создаем именованный семафор
			if ( (ptr_share->sem = sem_open(SEMAPHORE_NAME, O_CREAT, 0777, 0)) == SEM_FAILED ) {
				perror("sem_open");
				return EXIT_FAILURE;
			}

		    ptr_share->stop = 1;
		    //инициализация блокировки чтения записи
		    pthread_rwlock_init(&ptr_share->rwl, NULL);

			//запуск процессов
			//параметр обозначает номер процесса

		    //процессы писатели
			char *arg_list[] = { "./qnx_22_2_7", "1",  NULL };
			spawnv( P_NOWAIT, "./qnx_22_2_7", arg_list );

			char *arg_list2[] = { "./qnx_22_2_7", "2", NULL };
			spawnv( P_NOWAIT, "./qnx_22_2_7", arg_list2 );

			char *arg_list3[] = { "./qnx_22_2_7", "3", NULL };
			spawnv( P_NOWAIT, "./qnx_22_2_7", arg_list3 );

			//процессы читатели
			char *arg_list4[] = { "./qnx_22_2_7", "4", NULL };
			int i;
			for(i=0; i<3; i++)
				spawnv( P_NOWAIT, "./qnx_22_2_7", arg_list4 );


			sleep(65);

			//запрещаем выполнение процессов писателей, читателей
		   	pthread_rwlock_wrlock(&ptr_share->rwl);
		   	ptr_share->stop = 0;
			pthread_rwlock_unlock(&ptr_share->rwl);

		}

	return EXIT_SUCCESS;
}
