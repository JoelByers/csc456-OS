/*
    Joel Byers
    csc 456 
    Assignment 2
    Due 10/17/21

    Description:  Creates a cheeseburger using producers and consumers. Creates
                  three milk consumers, two cheese consumers and one burger consumer.
                  Uses bounded buffer to store and consume items.  The semaphore.h library
                  is used to synchronize buffer access.
*/
#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <atomic>
#include <cstring>
#include <semaphore.h>

using namespace std;

#define dbg(s) cerr << s << endl;   // Debug Output

#define MILK_THREADS 3
#define CHEESE_THREADS 2
#define BURGER_THREADS 1
#define MILK_BUFFER_SIZE 9
#define CHEESE_BUFFER_SIZE 4
#define TOTAL_THREADS MILK_THREADS + CHEESE_THREADS + BURGER_THREADS

int bufferMilk[MILK_BUFFER_SIZE] = {0};
int bufferCheese[CHEESE_BUFFER_SIZE] = {0};

// Milk Producer Variables
sem_t bufferMilkMutex;
sem_t bufferMilkFull;
sem_t bufferMilkEmpty;
int bufferMilkIn = 0;
int bufferMilkOut = 0;

// Cheese Producer Variables
sem_t bufferCheeseMutex;
sem_t bufferCheeseFull;
sem_t bufferCheeseEmpty;
int bufferCheeseIn = 0;
int bufferCheeseOut = 0;

// Burger Producer Variable
pthread_mutex_t burgerOut;

// Producer Thread Argument
struct ProducerArg
{
    int id;
    int quantity;
};

void *milkProducer(void *param)
{
    ProducerArg args = *(ProducerArg*)param;
    int id = args.id;
    int quantity = args.quantity;

    for(int i = 0; i < (quantity * 2); i++) //Produce 3 milk per burger
    {
        sem_wait(&bufferMilkEmpty);
        sem_wait(&bufferMilkMutex);
        bufferMilk[bufferMilkIn] = id;                          // add milk to buffer
        bufferMilkIn = (bufferMilkIn + 1) % MILK_BUFFER_SIZE;   // move back of buffer
        sem_post(&bufferMilkMutex);
        sem_post(&bufferMilkFull);
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
        string cheeseId = "";

        // Consume 3 Milk
        for(int i = 0; i < 3; i++)
        {
            sem_wait(&bufferMilkFull);
            sem_wait(&bufferMilkMutex);
            cheeseId += to_string(bufferMilk[bufferMilkOut]);       // add one milk to cheese
            bufferMilkOut = (bufferMilkOut + 1) % MILK_BUFFER_SIZE; // move front of buffer
            sem_post(&bufferMilkMutex);
            sem_post(&bufferMilkEmpty);
        }

        // Produce 1 Cheese
        sem_wait(&bufferCheeseEmpty);
        sem_wait(&bufferCheeseMutex);

        cheeseId += to_string(id);                                  // add cheese id to cheese
        bufferCheese[bufferCheeseIn] = stoi(cheeseId);              // convert to int
        bufferCheeseIn = (bufferCheeseIn + 1) % CHEESE_BUFFER_SIZE; // move back of buffer

        sem_post(&bufferCheeseMutex);
        sem_post(&bufferCheeseFull);
       
    }

    pthread_exit(0);
}

void *burgerProducer(void *param)
{
    int numBurgers = *(int*)param;

    for(int i = 0; i < numBurgers; i++)
    {
        string burgerId = "";

        for(int j = 0; j < 2; j++)
        {
            sem_wait(&bufferCheeseFull);
            sem_wait(&bufferCheeseMutex);

            burgerId += to_string(bufferCheese[bufferCheeseOut]);           // add cheese to burger
            bufferCheeseOut = (bufferCheeseOut + 1) % CHEESE_BUFFER_SIZE;   // change front of buffer

            sem_post(&bufferCheeseMutex);
            sem_post(&bufferCheeseEmpty);   
        }

        pthread_mutex_lock(&burgerOut);
        cout << burgerId << endl;                                           // output burger
        pthread_mutex_unlock(&burgerOut);
    }

    pthread_exit(0);
}

int main()
{
    // Initialize semophores
    sem_init(&bufferMilkEmpty, 0, MILK_BUFFER_SIZE);
    sem_init(&bufferMilkFull, 0, 0);
    sem_init(&bufferMilkMutex, 0, 1);

    sem_init(&bufferCheeseEmpty, 0, CHEESE_BUFFER_SIZE);
    sem_init(&bufferCheeseFull, 0, 0);
    sem_init(&bufferCheeseMutex, 0, 1);

    pthread_mutex_init(&burgerOut, NULL);

    pthread_t threadIDs[20] = {};
    int threadIDsIndex = 0;
    pthread_attr_t attr;
    ProducerArg* arg;
    int numBurgers = 0;

    // Get number of burgers to produce
    cout << "Enter number of burgers -> ";
    cin >> numBurgers;
    cout << endl;

    // Create milk threads
    for(int i = 0; i < MILK_THREADS; i++)
    {
        arg = (ProducerArg*)malloc(sizeof(*arg));
        arg->id = i + 1;
        arg->quantity = numBurgers;

        pthread_attr_init(&attr);
        pthread_create(&threadIDs[threadIDsIndex], &attr, milkProducer, arg);
        threadIDsIndex++;
    }                                                         

    // Create cheese threads
    for(int i = 0; i < CHEESE_THREADS; i++)
    {
        arg = (ProducerArg*)malloc(sizeof(*arg));
        arg->id = i + MILK_THREADS + 1;
        arg->quantity = numBurgers;

        pthread_attr_init(&attr);
        pthread_create(&threadIDs[threadIDsIndex], &attr, cheeseProducer, arg);
        threadIDsIndex++;
    }

    // Create burger thread
    int* numPtr = (int*)malloc(sizeof(int*));
    *numPtr = numBurgers;

    pthread_attr_init(&attr);
    pthread_create(&threadIDs[threadIDsIndex], &attr, burgerProducer, numPtr);
    threadIDsIndex++;

    // Join threads
    for(int i = 0; i < threadIDsIndex; i++)
    {
        pthread_join(threadIDs[i], NULL);
    }

    return 0;
}