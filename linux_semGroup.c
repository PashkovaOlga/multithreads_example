#include <stdio.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};


int sem_id; //ид группы семафоров


void del_semvalue(int sem_id);
void sem_ini(void);
int set_semvalue(int sem_id,int num, int val);
int sem_lock(int sem_id, int sem_num, int sem_op) ;
int sem_unlock(int sem_id, int sem_num, int sem_op) ;

void make_A(int idSem)
{
    //устанавливаем значение семафора детали А
   if (!set_semvalue(idSem, 0, 1)) {
       exit(0);}

    while(1)
    {
        //блокируем семафор А
        if (!sem_lock(idSem, 0, -1)) exit(0);

        sleep(2);
        printf("Detail A\n");

        //разблокируем семафор А
        if (!sem_unlock(idSem, 0, 1)) exit(0);

        //разблокируем семафор модуля
        if (!sem_unlock(idSem, 2, 1)) exit(0);

        sleep(3);
    }
}

void make_B(int idSem)
{
    if (!set_semvalue(idSem, 1, 1)) {
        exit(0);}

    while(1){
        if (!sem_lock(idSem, 1, -1)) exit(0);

        sleep(3);
        printf("Detail B\n");

        if (!sem_unlock(idSem, 1, 1)) exit(0);

        //разблокируем семафор модуля
        if (!sem_unlock(idSem, 2, 1)) exit(0);

        sleep(3);
    }
}

void make_M(int idSem, int Num)
{
    //счетчик виджетов
    int wid_counter = 1;

    if (!set_semvalue(idSem, 2, 0)) {
        exit(0);
    }

    while(1){
        //блокируем семафоры модуля
        if (!sem_lock(idSem, 2, -2)) exit(0);

        //блокируем семафоры деталей А и В
        if (!sem_lock(idSem, 0, -1)) exit(0);
        if (!sem_lock(idSem, 1, -1)) exit(0);

        //производство модуля 1
        printf("Module_1\n");

        //производство виджета:

        //блокируем семафоры детали С
        if (!sem_lock(idSem, 3, -1)) exit(0);

        printf("----Widget %d-----\n\n", wid_counter);
        //если все виджеты изготовлены
        //удаляем группу семафоров
        if(wid_counter++ == Num){
            del_semvalue(idSem);
            exit(0);}

        //разблокируем семафоры деталей А, В, С
        if (!sem_unlock(idSem, 0, 1)) exit(0);
        if (!sem_unlock(idSem, 1, 1)) exit(0);
        if (!sem_unlock(idSem, 3, 1)) exit(0);

        sleep(3);
    }
}

void make_C(int idSem)
{
    if (!set_semvalue(idSem, 3, 1)) {
        exit(0);}

    while(1){

        if (!sem_lock(idSem, 3, -1)) exit(0);
        sleep(4);
        printf("Detail C\n");
        if (!sem_unlock(idSem, 3, 1)) exit(0);

        sleep(3);
    }
}


//запускаем программу без параметров,
//затем она сама запускает 4 процесса изготовления деталей
int main(int argc, char *argv[])
{

    //запуск с параметрами командной строки
    if (argc > 1){

        //узнаем изготавливаемую деталь
        char option;

         if(!strcmp(argv[1], "A")){
            option = 'A';}
         else if(!strcmp(argv[1], "B")){
            option = 'B';}
         else if(!strcmp(argv[1], "C")){
            option = 'C';}
         else if(!strcmp(argv[1], "M")){
            option = 'M';}

         //получаем ид группы семафоров
         char semId[50];
         strcpy(semId, argv[2]);
         int id_sem = atoi(semId);

         //узнаем кол-во виджетов
         int Num;
         if(argv[3]!=NULL){
             Num = atoi(argv[3]);
         }

        switch(option){
            case 'A':
                 make_A(id_sem);
                return 0;
            case 'B':
                make_B(id_sem);
                return 0;
            case 'C':
                make_C(id_sem);
                return 0;
            case 'M':
                make_M(id_sem, Num);
                return 0;
        }
    }
    //запуск без параметров
    else
    {
        printf("Lab 2. Variant 5.\n\n");

        //ввод количества виджетов с консоли
        int N;
        printf("Enter number of widgets: ");
        //считывание данных с консоли
        scanf("%d", &N);
        printf("\n");

        while(N<=0){
            printf("Wrong number of widgets. \n");
            printf("Enter number of widgets: ");
            //считывание данных с консоли
            scanf("%d", &N);
            printf("\n");
        }

        //формирование группы семафоров
        sem_ini();
        //узнаем ид группы семафоров
        char sem_id_char[50];
        sprintf(sem_id_char, " %d", sem_id);//--

        //формирование команды для запуска процессов
        size_t len = strlen(argv[0]);
        size_t len1 = len;
        len1 +=50;
        len *= 4;
        len += 200;
        char commandA[len];
        //получаем название исполняемого файла
        char *szStr = argv[0];
        strcpy(commandA, szStr);
        char *szStr2 = " A";
        strcat(commandA, szStr2);
        strcat(commandA, sem_id_char);
        strcat(commandA, " & ");

        char commandB[len1];
        strcpy(commandB, szStr);
        char *szStr3 = " B";
        strcat(commandB, szStr3);
        strcat(commandB, sem_id_char);
        strcat(commandA, commandB);
        strcat(commandA, " & ");

        char commandC[len1];
        strcpy(commandC, szStr);
        char *szStr4 = " C";
        strcat(commandC, szStr4);
        strcat(commandC, sem_id_char);
        strcat(commandA, commandC);
        strcat(commandA, " & ");

        char num_char[50];
        sprintf(num_char, " %d", N);//--

        char commandM[len1];
        strcpy(commandM, szStr);
        char *szStr5 = " M";
        strcat(commandM, szStr5);
        strcat(commandM, sem_id_char);
        strcat(commandM, num_char);
        strcat(commandA, commandM);
        strcat(commandA, "\0 ");

        //пример комманды:
        //./lab2 A 123 & ./lab2 B 123 & ./lab2 C 123 & ./lab2 M 123 5
        //./lab2 - исполняемый файл
        // А - опция указывает на изготовление детали А
        //123 - ид группы семафоров
        //5 - кол-во виджетов

        //запуск 4 процессов
        system(commandA);
    }

    return 0;
}

//инициализация группы семафоров
void sem_ini(void)
{
    key_t key;
    key = ftok("/etc/fstab", getpid());

    //формируем группу из 4 семафоров
    if ((sem_id = semget(key, 4, IPC_CREAT|0666))==-1)
    {
        exit(0);
    }
}

//удаление группы семафоров
void del_semvalue(int sem_id)
{
    if (semctl(sem_id, 0, IPC_RMID, 0) == -1){
        exit(0);}
}

/**
 * @brief set_semvalue - устанавливаем значение семафору
 * @param sem_id - ид группы семафоров
 * @param num - номер семафора в группе
 * @param val - значение
 */
int set_semvalue(int sem_id, int num, int val) {
    union semun sem_union;
    sem_union.val = val;

    if (semctl(sem_id, num, SETVAL, sem_union) == -1){ return(0);}

    return(1);
}

/**
 * @brief sem_lock - блокировка семафора
 * @param sem_id - ид группы семафоров
 * @param num - номер семафора в группе
 * @param sem_op - занчение на которое изменяется семафор
 */
int sem_lock(int sem_id, int sem_num, int sem_op) {
 struct sembuf sem_b;
 sem_b.sem_num = sem_num;
 sem_b.sem_op = sem_op;
 sem_b.sem_flg = SEM_UNDO;
 if (semop(sem_id, &sem_b, 1) == -1) {
  return(0);
 }
 return(1);
}


/**
 * @brief sem_unlock - разблокировка семафора
 * @param sem_id - ид группы семафоров
 * @param num - номер семафора в группе
 * @param sem_op - занчение на которое изменяется семафор
 */
int sem_unlock(int sem_id, int sem_num, int sem_op) {
 struct sembuf sem_b;
 sem_b.sem_num = sem_num;
 sem_b.sem_op = sem_op;
 sem_b.sem_flg = SEM_UNDO;
 if (semop(sem_id, &sem_b, 1) == -1) {
  return(0);
 }
 return(1);
}
