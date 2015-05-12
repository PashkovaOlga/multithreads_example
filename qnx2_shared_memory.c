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


#define SEMAPHORE_NAME "/my_named_semaphore" //��� ������������ ��������
#define SHARED_MEMORY_OBJECT_NAME "shared_memory" // ��� ����������� ������
#define CLOSE  4



//��������� ������� �������� � ����������� ������ ����� ����������
struct shared_info{
	pthread_rwlock_t rwl;  //���������� ������ ������
	bool stop;  //���� ��������� ���������
	sem_t *sem; //��������� �� ��. �������
};

//�������� ����� ����������� ������
void closeShare(int fd_share)
{
	close(fd_share);
	//�������� ����������� ������
	if ( fd_share == CLOSE ) {
	    shm_unlink(SHARED_MEMORY_OBJECT_NAME);
	}
}

void reader()
{
	pid_t pid = getpid();
	//������ ��������� ��������
	struct sched_param param;
	 sched_getparam(pid, &param);
	 //������������� ��������������� RR
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

		//��������� �� ����������� ������
			struct shared_info *ptr_share;
			int fd_share = shm_open( SHARED_MEMORY_OBJECT_NAME, O_RDWR | O_CREAT, 0777 );
			    if( fd_share == -1 ) {
			        fprintf( stderr, "Open failed:%s\n",
			            strerror( errno ) );
			        exit( EXIT_FAILURE);
			    }

			    //����������� ��������� ����� ����������� ����������� ������
				ptr_share = mmap( 0, sizeof( *ptr_share ),
			            PROT_READ | PROT_WRITE,
			            MAP_SHARED, fd_share, 0 );
			    if( ptr_share == MAP_FAILED ) {
			        fprintf( stderr, "incrementor: mmap failed: %s\n",
			            strerror( errno ) );
			        exit( EXIT_FAILURE);
			    }

			    FILE *file;
			   //��������� ���� ��� ������ �����
			   if ((file = fopen("output_lab2.txt","r")) == NULL){
			    	printf("Error can't create output file\n");
			    	exit (EXIT_FAILURE);}

			   	int c;
			   	//���� ������� �� ����������
			    while(ptr_share->stop)
			    {
			    	//���� �� ��������
			    	sem_wait(ptr_share->sem);
			    	//���������� �� ������
			    	pthread_rwlock_rdlock(&ptr_share->rwl);

			    	//������������� ����������� ������ �����
			    	fseek(file, 0, SEEK_SET);
			    	printf("\nRead pid: %d-------------------------------------------\n", pid);

			    	//��������� ����, ���� �� ������ �� �����
			        while( (c = getc( file )) != EOF ) {
			            printf("%c", c);
			        }

			        printf("\nStop read ------------------------------------------------\n");
			    	//������� ���������� �� �/�
			        pthread_rwlock_unlock(&ptr_share->rwl);
			        //������ �������
			    	sem_post(ptr_share->sem);

			    	//�������� ��������
			    	T = rand_r(&seek) % D;
			    	delay(T);
			    }

			    //��������� ����
			    fclose(file);
			    //���������� ���������� �/�
			    pthread_rwlock_destroy(&ptr_share->rwl);
			    closeShare(fd_share);


				//�������� ��������
				if (sem_close(ptr_share->sem) == -1 )
				{
					//perror("sem_close");
					exit( EXIT_FAILURE);
				}

				//����������� ��������
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

	//��������� �� ����������� ������
		struct shared_info *ptr_share;
		int fd_share = shm_open( SHARED_MEMORY_OBJECT_NAME, O_RDWR | O_CREAT, 0777 );
		    if( fd_share == -1 ) {
		        fprintf( stderr, "Open failed:%s\n",
		            strerror( errno ) );
		        exit( EXIT_FAILURE);
		    }

		    //����������� ��������� ����� ����������� ����������� ������
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
		    	option = 'a'; //����� ��� �������� �����, ��� ������ �� ��������� ��������
		    }
		    else if(proc_no == 3)
		    {
		    	option = 'w'; //����� ��� �������� ����� ��� ����������
		    }
			//��������� ���� ��� ������ �����
			if ((file = fopen("output_lab2.txt", &option)) == NULL){
			        printf("Error can't create output file\n");
			        exit (EXIT_FAILURE);}

		    while(ptr_share->stop)
		    {
		    	//�������� ��������� ����� � ������
		    	if(proc_no == 3)
		    		fseek(file, 0, SEEK_SET);
		    	//���� ������ � ������
		    	int i;
		    	for(i=0; i<60 ; i++)
		    	{
		    		//���������� �� ������
		    		pthread_rwlock_wrlock(&ptr_share->rwl);
		    		//������ ����
		    		fprintf(file, "%d ", proc_no);
		    		//����� �������� 60 �������� ��������� �� ����� ������
		    		if((proc_no == 1) && i == 59)
		    		{
		    			fprintf(file, "\n");
		    		}

		    		//�������� ��������� ����� �� 1 ����,
		    		//����� �������� 1 � 2 ������ ���� �� ������
		    		if(proc_no < 3)
		    			fseek(file, 1, SEEK_CUR);
		    		//������ ����������
		    		pthread_rwlock_unlock(&ptr_share->rwl);

		    		usleep(100000);

		    		//���� ������� ����������, ������� �� �����
		    		pthread_rwlock_rdlock(&ptr_share->rwl);
		    		if(!ptr_share->stop) break;
		    		pthread_rwlock_unlock(&ptr_share->rwl);

		    		//����� ������������ ������� � �������
		    		//���� j = �������� i
		    		int j;
		    		for(j = 0; j<i; j++)
		    		printf("%d ", proc_no);
		    		printf("\n");
		    	}
		    	//�������� ��������
		    	T = rand_r(&seek) % D;
		    	delay(T);
		    }

		    fclose(file);
		    pthread_rwlock_destroy(&ptr_share->rwl);
		    closeShare(fd_share);
			exit (EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {

	//���� �������� ��������� � �����������
		if (argc > 1)
		{
			//������ ����� ������������ ��������
		    char proc;
		    strcpy(&proc, argv[1]);
		    int proc_no = atoi(&proc);

		    //�������� �������� 1, 2, 3
		    if (proc_no <= 3)
		    	writer(proc_no);
		    //3 �������� ���������
		    else if(proc_no == 4)
		    	reader();
		}
		//������ �������� ��� ����������
		else
		{
			printf("Lab2. Variant 22. Task 2.7\n\n");

			//��� ������ ������� ������� ����������� ������
		    shm_unlink( SHARED_MEMORY_OBJECT_NAME );

		    //������� ����������� ������
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
		    //������� ���� ��� ������ �����
		    if ((file = fopen("output_lab2.txt","w")) == NULL){
		        printf("Error can't create output file\n");
		        exit (EXIT_FAILURE);}
		    fclose(file);

		    //������� ����������� �������
			if ( (ptr_share->sem = sem_open(SEMAPHORE_NAME, O_CREAT, 0777, 0)) == SEM_FAILED ) {
				perror("sem_open");
				return EXIT_FAILURE;
			}

		    ptr_share->stop = 1;
		    //������������� ���������� ������ ������
		    pthread_rwlock_init(&ptr_share->rwl, NULL);

			//������ ���������
			//�������� ���������� ����� ��������

		    //�������� ��������
			char *arg_list[] = { "./qnx_22_2_7", "1",  NULL };
			spawnv( P_NOWAIT, "./qnx_22_2_7", arg_list );

			char *arg_list2[] = { "./qnx_22_2_7", "2", NULL };
			spawnv( P_NOWAIT, "./qnx_22_2_7", arg_list2 );

			char *arg_list3[] = { "./qnx_22_2_7", "3", NULL };
			spawnv( P_NOWAIT, "./qnx_22_2_7", arg_list3 );

			//�������� ��������
			char *arg_list4[] = { "./qnx_22_2_7", "4", NULL };
			int i;
			for(i=0; i<3; i++)
				spawnv( P_NOWAIT, "./qnx_22_2_7", arg_list4 );


			sleep(65);

			//��������� ���������� ��������� ���������, ���������
		   	pthread_rwlock_wrlock(&ptr_share->rwl);
		   	ptr_share->stop = 0;
			pthread_rwlock_unlock(&ptr_share->rwl);

		}

	return EXIT_SUCCESS;
}
