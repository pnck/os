typedef struct _tagsemaphore
{
    int value;
    struct TCB *wake_queue;
}semaphore;

void sem_p(semaphore *sem)
{
    struct TCB **pqueue;
    disable();
    sem -> value--;
    if(sem->value < 0)
    {
        pqueue = &(sem->wake_queue);
        block(pqueue);
    }
    enable();
}

void sem_v(semaphore *sem)
{
    struct TCB **pqueue;
    disable();
    pqueue = &(sem->wake_queue);
    sem->value++;
    if(sem->value <= 0)
    {
        wakeup_first(pqueue);
    }
    enable();
}
