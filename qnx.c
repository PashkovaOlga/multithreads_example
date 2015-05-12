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

//��������� ��� �������� ��������� ������
struct th_param
{
	pthread_t id_th; //�� ������
	pthread_t id_th_another; //�� ������������� ��������� ������
	struct sched_param *param; //��������� ������
	struct sched_param *param_another; //��������� ������������� ������
	int no; //����� ������
};

//�������� ����� 1 ��������
void *daughter()
{
	//������ 2 �������� � ����������, ����� ����������
	spawnl( P_WAIT, "./22_1_3", "./22_1_3", "1" );
	printf ("----- Parent1 end -----\n");

	exit( EXIT_SUCCESS);
}

//�������� ������ 2 ��������
void *daughter12(void* param)
{
	//�������� ���������� ������ �� ���������
	struct th_param *dau2_param = (struct th_param *)param;


	int policy; //��������������
	//������ ��������� ������
	pthread_getschedparam( dau2_param->id_th, &policy, dau2_param->param );

	//����� ������ 1 ��� 2
	int no = dau2_param->no;
	//printf("---> no = %d policy %d \n", no, policy);

	while(1)
		{
			//���������� �� �������
			int res = pthread_mutex_lock(&mutex);
			if (res != 0) {
			  perror("dau1: Mutex initialization failed");
			  exit(EXIT_FAILURE);
			 }
			//���� 1 �����
			if(no == 1)
				N++;
			//���� 2 �����
			else if(no == 2)
				N--;

			//printf("dau%d: N = %d\n", no, N);

			int rez = 0;
			if(N == 11000){
				if(no == 1){
					//���� N=11000 � ����������� 1 �����
					//����������� ���������
					dau2_param ->param->sched_priority++;
					//������������� ����������� ���������
					rez = pthread_setschedparam( dau2_param->id_th, policy, dau2_param->param );}
				else if(no == 2){
					//���� N=11000 � ����������� 2 �����
					//����������� ��������� ������������� ������
					dau2_param->param_another->sched_priority++;
					//������������� ��������� � ������������ �����
					rez = pthread_setschedparam( dau2_param->id_th_another, policy, dau2_param->param_another );}
			}
			else if(N == -11000){
					if(no == 2){
						//���� N=-11000 � ����������� 2 �����
						//��������� ���������
						dau2_param ->param->sched_priority--;
						//������������� ����������� ���������
						rez = pthread_setschedparam( dau2_param->id_th, policy, dau2_param->param );}
					else if(no == 1){
						//���� N=-11000 � ����������� 1 �����
						//��������� ��������� ������������� ������
						dau2_param->param_another->sched_priority--;
						//������������� ��������� � ������������ �����
						rez = pthread_setschedparam( dau2_param->id_th_another, policy, dau2_param->param_another );}
				}

				//���� ��������� �� ������������
				if(rez == -1)
				{
					fprintf( stderr, "set scheduler failed:%s\n",
					strerror( errno  ));
					exit( EXIT_FAILURE);
				}
			//������������ ������
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

	//���� �������� ��������� ��� ����������
	//parent1
	if (argc == 1){

	printf ("Lab1. Variant 22. Task 1.3\n\n");
	printf ("Run parent1\n");

	//������ �����
	struct tm tm_ptr;
	time_t the_time;
	(void) time (&the_time);
	localtime_r (&the_time, &tm_ptr);

	char *c_time = (char*) malloc(200 * sizeof(char));
	strftime( c_time, 200, "%c", &tm_ptr );

	printf ("Parent1: Local time: %s \nParent1: Pid: %d\n", c_time, getpid());
	free(c_time);
	// ����� �������� � ������ �������� �����
	pthread_attr_t attr;
    struct sched_param param;
	pthread_attr_init( &attr );
	//�� ��������� �������� �� ��������
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	param.sched_priority = 10;
	pthread_attr_setschedparam(&attr, &param); // ��������� 10
	pthread_attr_setschedpolicy(&attr, SCHED_RR);// ���������� ��������������� Round-Robin
	pthread_t dau;
	struct th_param dau_param; //��������� ������������ � �����
	dau_param.id_th = dau;
	dau_param.param = &param;
	//������ ������
	pthread_create( &dau, &attr, &daughter, (void*)&dau_param );

	//���� ���������� ��������� ������
	pthread_join(dau, NULL);


	}
	//parent2
	if (argc > 1)
	{
		printf("\nRun Parent2\n");

		//������ ��������� �����
		struct tm tm_ptr;
		time_t the_time;
		(void) time (&the_time);
		localtime_r (&the_time, &tm_ptr);
		char *c_time = (char*) malloc(200 * sizeof(char));
		strftime( c_time, 200, "%c", &tm_ptr );

		//�������������� ������ �� ���������
		int res = pthread_mutex_init(&mutex, NULL );
		if (res != 0) {
		  perror("Mutex initialization failed");
		  exit(EXIT_FAILURE);
		 }

		printf ("Parent2: Local time: %s \nParent2: Pid: %d\n\n", c_time, getpid());
		free(c_time);
		printf("Wait ...\n");
		// ����� �������� � ������ �������� �����
		pthread_attr_t attr;
	    struct sched_param param;
		pthread_attr_init( &attr );// ����� �������� � ������ ����� ����1
		pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
		param.sched_priority = 10;
		pthread_attr_setschedparam(&attr, &param); // ��������� 10
		pthread_attr_setschedpolicy(&attr, SCHED_RR);// ���������� ��������������� Round-Robin
		pthread_t dau;
		struct th_param dau_param;
		dau_param.id_th = dau;
		dau_param.param = &param;
		dau_param.no = 1;


		pthread_attr_t attr2;
	    struct sched_param param2;
		pthread_attr_init( &attr2 );// ����� �������� � ������ ����� ����2
		pthread_attr_setinheritsched(&attr2, PTHREAD_EXPLICIT_SCHED);
		param2.sched_priority = 10;
		pthread_attr_setschedparam(&attr2, &param2); // ��������� 10
		pthread_attr_setschedpolicy(&attr2, SCHED_FIFO);// ���������� ��������������� fifo
		pthread_t dau2;
		struct th_param dau2_param;
		dau2_param.id_th = dau2;
		dau2_param.param = &param2;
		dau2_param.no = 2;

		//������������� �� ������������� ������
		dau_param.id_th_another = dau2;
		dau2_param.id_th_another = dau;
		//������������� ��������� ������������� ������
		dau_param.param_another = dau2_param.param;
		dau2_param.param_another = dau_param.param;
		pthread_create( &dau, &attr, &daughter12, (void*)&dau_param );
		pthread_create( &dau2, &attr2, &daughter12, (void*)&dau2_param );

		sleep( 45 );  // �������� �� 45 ������, ���� �������� ������-�������
		pthread_cancel(dau); // ������������� ���������� ���������� �������
	    pthread_cancel(dau2);

	    //�������� N
	    pthread_mutex_lock(&mutex);
	    printf("Parent2: N = %d\n", N);
	    pthread_mutex_unlock(&mutex);

	    //���������� ������
	    pthread_mutex_destroy( &mutex);
	    printf ("----- Parent2 end -----\n");

	}

	return EXIT_SUCCESS;
}
