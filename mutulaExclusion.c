#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <limits.h>
#define N 20

sem_t mutex,full,empty,read;
int count = 0;

struct Queue
{
    int front, rear, size;
    unsigned capacity;
    int* array;
};


struct Queue* createQueue(unsigned capacity)
{
    struct Queue* queue = (struct Queue*) malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->array = (int*) malloc(queue->capacity * sizeof(int));
    return queue;
}

int isFull(struct Queue* queue)
{  return (queue->size == queue->capacity);  }

int isEmpty(struct Queue* queue)
{  return (queue->size == 0); }

void enqueue(struct Queue* queue, int item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1)%queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}

int dequeue(struct Queue* queue)
{
    if (isEmpty(queue))
        return 0;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1)%queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

struct Queue* queue;


void *counterFunc(void * n)
{
    int temp = (int *) n;
    while(1)
    {
        sleep(1);
        printf("Counter thread %d: received a message.\n", temp);
        printf("Counter thread %d: waiting to write.\n", temp);
        sem_wait(&mutex);
        count++;
        sem_post(&mutex);
        printf("Counter thread %d:  now adding to counter, counter value=%d.\n", temp, count);
    }
}

void *monitorFunc()
{
    int temp;

    while(1)
    {
        sleep(1);

        printf("Monitor thread: waiting to read counter.\n");
        sem_wait(&mutex);
        temp = count;
        count = 0;
        sem_post(&mutex);
        printf("Monitor thread: reading a count value of %d.\n", temp);

        sem_wait(&full);
        sem_wait(&read);
        enqueue(queue, temp);
        printf("Monitor thread: writing to buffer at position %d.\n", queue->size);
        sem_post(&read);
        sem_post(&empty);

        if(isFull(queue))
        {
            printf("Monitor thread: Buffer full!!\n");
        }
    }
}

void *collectorFunc()
{
    int temp;

    while(1)
    {
        sleep(2);
        sem_wait(&empty);
        sem_wait(&read);
        temp = dequeue(queue);
        printf("Collector thread: reading from buffer at position %d.\n", queue->size);
        sem_post(&read);
        sem_post(&full);

        if(isEmpty(queue))
        {
            printf("Collector thread: nothing is in the buffer!\n");
        }
    }
}
int main()
{
    int i = 0;
    queue = createQueue(N);
    pthread_t counter[N], monitor, collector;
    sem_init(&mutex, 0, 1);
    sem_init(&full, 0, N);
    sem_init(&empty, 0, 0);
    sem_init(&read, 0, 1);

    for(i=0;i<N;i++)
    {
        pthread_create(&counter[i], NULL, counterFunc, (void*)i);
    }

    pthread_create(&monitor, NULL, monitorFunc, NULL);
    pthread_create(&collector, NULL, collectorFunc, NULL);

    for(i=0;i<N;i++)
    {
        pthread_join(counter[i], NULL);
    }

    pthread_join(monitor, NULL);
    pthread_join(collector, NULL);
    return 0;
}
