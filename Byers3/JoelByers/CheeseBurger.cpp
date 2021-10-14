#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <atomic>

using namespace std;

#define MILK_THREADS 3
#define CHEESE_THREADS 2

int bufferMilk[9] = {0};
int bufferCheese[4] = {0};
int bufferMilkIndex = 0;
int bufferCheeseIndex = 0;

struct ProducerArg
{
    int id;
    int quantity;
};

bool mutex = false;
int smphr1 = 1;

void aquire()
{
    while(mutex)
    ;
    mutex = true;
}

void release()
{
    mutex = false;
}

void wait(int s)
{
    while(s <= 0)
    ;
    s--;
}

void signal(int s)
{
    s++;
}

void *milkProducer(void *param)
{
    ProducerArg args = *(ProducerArg*)param;
    int id = args.id;
    int quantity = args.quantity;

    for(int i = 0; i < quantity; i++)
    {
        aquire();
        bufferMilk[bufferMilkIndex] = id;
        bufferMilkIndex++;
        release();
    }

    pthread_exit(0);
}

void *cheeseProducer(void *param)
{
    ProducerArg args = *(ProducerArg*)param;
    int id = args.id;
    int quantity = args.quantity;

    for(int i = 0; i < quantity; i++)
    {
        wait(smphr1);
        bufferCheese[bufferCheeseIndex] = id;
        bufferCheeseIndex++;
        signal(smphr1);
    }

    pthread_exit(0);
}

int main()
{
    pthread_t threadIDs[MILK_THREADS + CHEESE_THREADS];
    int threadIDsIndex = 0;
    pthread_attr_t attr;

    ProducerArg* arg = (ProducerArg*)malloc(sizeof(*arg));

    for(int i = 1; i < MILK_THREADS + 1; i++)
    {
        arg->id = i;
        arg->quantity = 1;

        pthread_attr_init(&attr);
        pthread_create(&threadIDs[threadIDsIndex], &attr, milkProducer, arg);
        threadIDsIndex++;
        pthread_join(threadIDs[threadIDsIndex], NULL);        // Reason for consistant numbers, create, join, create next
    }

    for(int i = MILK_THREADS; i < CHEESE_THREADS + MILK_THREADS + 1; i++)
    {
        arg->id = i;
        arg->quantity = 1;

        pthread_attr_init(&attr);
        pthread_create(&threadIDs[threadIDsIndex], &attr, cheeseProducer, arg);
        threadIDsIndex++;
        pthread_join(threadIDs[threadIDsIndex], NULL);        
    }

    cout << "MILK " << sizeof(bufferMilk) <<endl;
    for(int i = 0; i < sizeof(bufferMilk); i++)
    {
        cout << bufferMilk[i] << " " << i << endl;
    }

    cout << "CHEESE" << endl;
    for(int i = 0; i < sizeof(bufferCheese); i++)
    {
        cout << bufferCheese[i] << endl;
    }

/*    
    int* a = (int*)malloc(sizeof(*a));
    *a = 3;

    pthread_attr_init(&attr);
    pthread_create(&tid, &attr, milkProducer, a);
    pthread_join(tid, NULL);
*/
    return 0;
}