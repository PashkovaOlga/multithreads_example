/**
    Lab №1.
    Variant 12.
    OC: Windows

    Задание:
    Процесс 1 порождает поток 2, они присоединяют к себе общую память объемом (N*sizeof(int)).
    Процесс 1 пишет в нее k1 чисел сразу, процесс 2 переписывает k2 чисел из памяти в файл.
    Процесс 1 может производить запись, только если в памяти достаточно места,
    а поток 2 переписывать, только если имеется не меньше, чем k2 чисел.
    После каждой записи (чтения) процессы засыпают на t секунд.
    После каждой записи процесс 1 увеличивает значение записываемых чисел на 1.
    Используя события, обеспечить синхронизацию работы процессов.

**/

#include <windows.h>
#include <stdio.h>
#include <malloc.h>



int N=0, k1=0, k2=0, t=0, t2 = 0;

//количество не считанных чисел
int notRead_counter = 0;

//указатель на считываемую область памяти
int *ptr2_sh = NULL;

FILE *file = NULL;
HANDLE hEvent1, hEvent2;
HANDLE hThr;
unsigned long uThrID;

void Thread( void* pParams )
{
    int j;
    while (1)
    {
       //ожидание остановки потока 1
        WaitForSingleObject( hEvent2, INFINITE );
        //если имеется не меньше, чем k2 не считанных ранее чисел
        if(notRead_counter >= k2)
        {
            printf("Wtite to file ...\n");
            //запись в файл
            for(j=0; j < k2; j++){
                fprintf(file, "%d ", *ptr2_sh);
                printf("%d ", *ptr2_sh++);}//вывод в консоль
            printf("\n");
            fprintf(file, "\n");

            //уменьшение количества несчитанных чисел
            notRead_counter -= k2;

            printf("Sleep %d secs\n\n", t2);
            //засыпает на время t
            Sleep (t);
        }


        // устанавливает событие в сигнальное состояние
        SetEvent( hEvent1 );

    }
}

int main( void )
{
    //очищение консоли
    system("cls");

    printf("Lab 1. Variant 12.\n\n");
    //введите данные через пробел
    printf("Enter N k1 k2 t separated with a space: ");
    //считывание данных с консоли
    scanf("%d %d %d %d", &N, &k1, &k2, &t);
    t2 = t;
    t *= 1000; //millisecs


    //проверка: k1 и k2 не могут быть больше N
    while((k1 > N) || (k2 > N))
    {
        printf("Error of the input data: \n");
        if(k1 > N){
            //размера памяти не хватит чтобы записать к1 чисел сразу
            printf("(k1 > N)\n");}
        if(k2 > N){
            //в памяти не будет находиться к2 чисел
            printf("(k2 > N)\n");}

         printf("Enter new N k1 k2 t value: ");
         scanf("%d %d %d %d", &N, &k1, &k2, &t);
    }
    printf("\n");
    //создаем файл для записи чисел
    if ((file = fopen("output_lab1.txt","w")) == NULL){
        printf("Error can't create output file\n");
        return 1;}

    //выделение памяти
    //указатель на общую память потоков 1 и 2
    int *share = (int*) malloc(N * sizeof(int));

    /**
    http://support.tenasys.com/intimehelp_6/createevent.html

    HANDLE CreateEvent(
    LPSECURITY_ATTRIBUTES lpEventAttributes, в документации указано NULL
    BOOLEAN ManualReset, ручной или автосброс события, false - создаем события с автосбросом
    BOOLEAN bInitialState, состояние события  TRUE - сигнальное, FALSE - не сигнальное
    LPCTSTR lpName, объект события создаем без имени
    );**/
    hEvent1=CreateEvent(NULL, FALSE, TRUE, NULL);
    hEvent2=CreateEvent(NULL, FALSE, FALSE, NULL);

    /**
    HANDLE CreateThread(
    LPSECURITY_ATTRIBUTES lpThreadAttributes, // дескриптор защиты
    Если lpThreadAttributes является значением ПУСТО (NULL), дескриптор не может быть унаследован.

    SIZE_T dwStackSize,                       // начальный размер стека
    Если это значение нулевое, новый поток использует по умолчанию размер стека исполняемой программы

    LPTHREAD_START_ROUTINE lpStartAddress,    // функция потока
    LPVOID lpParameter,                       // параметр потока
    Указатель на переменную, которая передается в поток.

    DWORD dwCreationFlags,                    // опции создания
    Если это значение нулевое, поток запускается немедленно после создания

    LPDWORD lpThreadId                        // идентификатор потока
    );
    **/
    hThr=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Thread,NULL,0,&uThrID);

    //поток 1: запись чисел в память
    int i;
    //значение записываемых чисел
    int val = 0;
    //свободное место в памяти
    int freeSpace_counter = N;


    //указатель на область памяти для записи чисел
    int *ptr1_sh = share;
    ptr2_sh = share;

    while(1){
        //ожидание остановки потока 2
        WaitForSingleObject( hEvent1, INFINITE );

        //проверка свободного места в памяти для записи
        if (freeSpace_counter >= k1)
        {
            printf("Wtite to memory ...\n");
            for(i=0; i < k1; i++){
                *ptr1_sh = val;
                 printf("%d ", *ptr1_sh++);}//вывод в консоль
            val++;
            printf("\n");

            //уменьшение значения свободного места
            freeSpace_counter -= k1;
            //увеличение счетчика не считанных чисел
            notRead_counter += k1;

            printf("Sleep %d secs\n\n", t2);
            //засыпает на время t
            Sleep (t);
        }
        //если все считано
        else if (notRead_counter < k2)
        {
            //освобождение памяти
            free (share);

            //закрытие файла
            fclose(file);

            printf("The END.\n");
            //устанавливает событие в сигнальное состояние
            return 0;
        }

        SetEvent( hEvent2 );}

}





