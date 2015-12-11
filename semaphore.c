#include "sem.h"

extern int current;//global var --> current thread
extern struct TCB g_tcb[NTCB];

void block(struct TCB **pqueue)
{
    struct TCB *tcbqueue = *pqueue; 
    g_tcb[current].state = BLOCKED;//block current thread

    if(tcbqueue==NULL)// no thread is wating ,queue is empty
    {
        tcbqueue = &g_tcb[current];//point to current thread
        tcbqueue->fd = NULL;
        tcbqueue->bk = NULL;
    }
    else
    {
        struct TCB *p;
        for(p=tcbqueue;p->bk!=NULL;p=p->bk);//find last
        p->bk = &g_tcb[current];//link current to queue
        g_tcb[current].fd = p;//link current to queue
        g_tcb[current].bk = NULL;
    }
    enable();
    swtch();
}
void wakeup_first(struct TCB ** pqueue)
{
    struct TCB *tcbqueue = *pqueue;
    if(tcbqueue==NULL) return;//no thread is wating,continue current

    tcbqueue->state = READY;//set first wating thread ready
    tcbqueue=tcbqueue->bk;
    tcbqueue->fd->bk = NULL;//unlink current thread
    tcbqueue->fd = NULL;
    return;
}


semaphore * create_semaphore(int rescount)
{
    semaphore *sem;
    if(rescount < 1) return NULL;

    sem = malloc(sizeof(semaphore));
    if(!sem) return NULL;

    disable();
    sem -> max_value = rescount;
    sem -> value = rescount - 1;
    sem -> wake_queue = NULL;
    sem -> aquired_list = malloc(sizeof(struct aquiredtcblist));
    if(!sem->aquired_list)
    {
        free(sem);
        sem = NULL;
    }
    else // add itself to aquired list
    {
        sem->aquired_list->_tcb = &g_tcb[current];
        sem->aquired_list->bk = NULL;
        sem->aquired_list->fd = NULL;
    }
    enable();
    return sem;
}

BOOL aquire_semaphore(semaphore * sem)//P op
{
    struct aquiredtcblist *p;
    if(!sem) return FALSE;
    //check if current thread has aquired a semaphore
    for(p=sem->aquired_list;p->bk!=NULL;p=p->bk)
    {
        if(p->_tcb == &g_tcb[current])//has aquired,do nothing
        {
            return FALSE;
        }
    }
    if (p->_tcb == &g_tcb[current]) //has aquired,do nothing
    {
        return FALSE;
    }
    disable();
    p->bk = malloc(sizeof(struct aquiredtcblist));
    if(p->bk == NULL)
    {
        enable();
        return FALSE;
    }
    p->bk->fd = p;
    p = p->bk;
    p->_tcb = &g_tcb[current];//aquired list += current thread
    p->bk = NULL;
    sem -> value--;
    if(sem->value < 0)
    {
        block(&(sem->wake_queue));
    }
    enable();
    return TRUE;
}
BOOL release_semaphore(semaphore *sem)//V OP
{
    struct aquiredtcblist *p;
    //empty queue and value == max_value
    if(!sem) return FALSE;

    //find current thread from aquired list
    for(p=sem->aquired_list;p!=NULL;p=p->bk)
    {
        if(p->_tcb == &g_tcb[current])//found
            break;
    }
    if(p == NULL) return FALSE;//not found, has not aquire a semaphore obj
    disable();
    sem->value++;

    if(p->fd) p->fd->bk = p->bk;//unlink p 
    else sem->aquired_list = p->bk;//p->bk become the first
    if(p->bk) p->bk->fd = p->fd;//unlink p

    free(p);//release current thread from aquired list

    if(sem->value <= 0)
    {
        wakeup_first(&(sem->wake_queue));
    }
    enable();
    return TRUE;
}
