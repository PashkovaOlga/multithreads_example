#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <process.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int N = 0;
pthread_mutex_t mutex;

//структура для передачи дочернему потоку
struct th_param
{
	pthread_t id_th; //ид потока
	pthread_t id_th_another; //ид параллельного дочернего потока
	struct sched_param *param; //параметры потока
	struct sched_param *param_another; //параметры параллельного потока
	int no; //номер потока
};

//дочерний поток 1 процесса
void *daughter()
{
	//запуск 2 процесса с параметром, ждать завершения
	spawnl( P_WAIT, "./22_1_3", "./22_1_3", "1" );
	printf ("----- Parent1 end -----\n");

	exit( EXIT_SUCCESS);
}

//дочерние потоки 2 процесса
void *daughter12(void* param)
{
	//передача параметров потока по указателю
	struct th_param *dau2_param = (struct th_param *)param;


	int policy; //диспечерезация
	//узнаем параметры потока
	pthread_getschedparam( dau2_param->id_th, &policy, dau2_param->param );

	//номер потока 1 или 2
	int no = dau2_param->no;
	//printf("---> no = %d policy %d \n", no, policy);

	while(1)
		{
			//блокировка на мютексе
			int res = pthread_mutex_lock(&mutex);
			if (res != 0) {
			  perror("dau1: Mutex initialization failed");
			  exit(EXIT_FAILURE);
			 }
			//если 1 поток
			if(no == 1)
				N++;
			//если 2 поток
			else if(no == 2)
				N--;

			//printf("dau%d: N = %d\n", no, N);

			int rez = 0;
			if(N == 11000){
				if(no == 1){
					//если N=11000 и выполняется 1 поток
					//увеличиваем приоритет
					dau2_param ->param->sched_priority++;
					//устанавливаем собственные параметры
					rez = pthread_setschedparam( dau2_param->id_th, policy, dau2_param->param );}
				else if(no == 2){
					//если N=11000 и выполняется 2 поток
					//увеличиваем приоритет параллельного потока
					dau2_param->param_another->sched_priority++;
					//устанавливаем параметры в параллельный поток
					rez = pthread_setschedparam( dau2_param->id_th_another, policy, dau2_param->param_another );}
			}
			else if(N == -11000){
					if(no == 2){
						//если N=-11000 и выполняется 2 поток
						//уменьшаем приоритет
						dau2_param ->param->sched_priority--;
						//устанавливаем собственные параметры
						rez = pthread_setschedparam( dau2_param->id_th, policy, dau2_param->param );}
					else if(no == 1){
						//если N=-11000 и выполняется 1 поток
						//уменьшаем приоритет параллельного потока
						dau2_param->param_another->sched_priority--;
						//устанавливаем параметры в параллельный поток
						rez = pthread_setschedparam( dau2_param->id_th_another, policy, dau2_param->param_another );}
				}

				//если параметры не установились
				if(rez == -1)
				{
					fprintf( stderr, "set scheduler failed:%s\n",
					strerror( errno  ));
					exit( EXIT_FAILURE);
				}
			//разблокируем мютекс
			int res2 = pthread_mutex_unlock(&mutex);
			if (res2 != 0) {
			  perror("dau1: Mutex initialization failed");
			  exit(EXIT_FAILURE);
			 }
			usleep(100000);
		}

	exit( EXIT_SUCCESS);
}


int main(int argc, char *argv[]) {

	//если запущена программа без параметров
	//parent1
	if (argc == 1){

	printf ("Lab1. Variant 22. Task 1.3\n\n");
	printf ("Run parent1\n");

	//узнаем время
	struct tm tm_ptr;
	time_t the_time;
	(void) time (&the_time);
	localtime_r (&the_time, &tm_ptr);

	char *c_time = (char*) malloc(200 * sizeof(char));
	strftime( c_time, 200, "%c", &tm_ptr );

	printf ("Parent1: Local time: %s \nParent1: Pid: %d\n", c_time, getpid());
	free(c_time);
	// Задаём атрибуты и создаём дочерний поток
	pthread_attr_t attr;
    struct sched_param param;
	pthread_attr_init( &attr );
	//не наследуем парамеры от процесса
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	param.sched_priority = 10;
	pthread_attr_setschedparam(&attr, &param); // Приоритет 10
	pthread_attr_setschedpolicy(&attr, SCHED_RR);// Дисциплина диспетчеризации Round-Robin
	pthread_t dau;
	struct th_param dau_param; //структура передающаяся в поток
	dau_param.id_th = dau;
	dau_param.param = &param;
	//запуск потока
	pthread_create( &dau, &attr, &daughter, (void*)&dau_param );

	//ждем завершения дочернего потока
	pthread_join(dau, NULL);


	}
	//parent2
	if (argc > 1)
	{
		printf("\nRun Parent2\n");

		//узнаем локальное время
		struct tm tm_ptr;
		time_t the_time;
		(void) time (&the_time);
		localtime_r (&the_time, &tm_ptr);
		char *c_time = (char*) malloc(200 * sizeof(char));
		strftime( c_time, 200, "%c", &tm_ptr );

		//инициализируем мютекс по умолчанию
		int res = pthread_mutex_init(&mutex, NULL );
		if (res != 0) {
		  perror("Mutex initialization failed");
		  exit(EXIT_FAILURE);
		 }

		printf ("Parent2: Local time: %s \nParent2: Pid: %d\n\n", c_time, getpid());
		free(c_time);
		printf("Wait ...\n");
		// Задаём атрибуты и создаём дочерний поток
		pthread_attr_t attr;
	    struct sched_param param;
		pthread_attr_init( &attr );// Задаём атрибуты и создаём поток внук1
		pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
		param.sched_priority = 10;
		pthread_attr_setschedparam(&attr, &param); // Приоритет 10
		pthread_attr_setschedpolicy(&attr, SCHED_RR);// Дисциплина диспетчеризации Round-Robin
		pthread_t dau;
		struct th_param dau_param;
		dau_param.id_th = dau;
		dau_param.param = &param;
		dau_param.no = 1;


		pthread_attr_t attr2;
	    struct sched_param param2;
		pthread_attr_init( &attr2 );// Задаём атрибуты и создаём поток внук2
		pthread_attr_setinheritsched(&attr2, PTHREAD_EXPLICIT_SCHED);
		param2.sched_priority = 10;
		pthread_attr_setschedparam(&attr2, &param2); // Приоритет 10
		pthread_attr_setschedpolicy(&attr2, SCHED_FIFO);// Дисциплина диспетчеризации fifo
		pthread_t dau2;
		struct th_param dau2_param;
		dau2_param.id_th = dau2;
		dau2_param.param = &param2;
		dau2_param.no = 2;

		//устанавливаем ид параллельного потока
		dau_param.id_th_another = dau2;
		dau2_param.id_th_another = dau;
		//устанавливаем параметры параллельного потока
		dau_param.param_another = dau2_param.param;
		dau2_param.param_another = dau_param.param;
		pthread_create( &dau, &attr, &daughter12, (void*)&dau_param );
		pthread_create( &dau2, &attr2, &daughter12, (void*)&dau2_param );

		sleep( 45 );  // Засыпаем на 45 секунд, пока работают потоки-потомки
		pthread_cancel(dau); // Принудительно прекращаем выполнение потоков
	    pthread_cancel(dau2);

	    //печатаем N
	    pthread_mutex_lock(&mutex);
	    printf("Parent2: N = %d\n", N);
	    pthread_mutex_unlock(&mutex);

	    //уничтожаем мютекс
	    pthread_mutex_destroy( &mutex);
	    printf ("----- Parent2 end -----\n");

	}

	return EXIT_SUCCESS;
}
