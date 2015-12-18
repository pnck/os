#include <stdlib.h>
#include "sem.h"

extern int g_current;//global var --> g_current thread
extern struct TCB g_tcb[NTCB];


void block(struct semaphore_waiter **pqueue)
{
  struct semaphore_waiter *p;// = *pqueue;
  struct semaphore_waiter waiter;
  asm cli;

  waiter.thread = &g_tcb[g_current];
  waiter.fd = waiter.bk = NULL;
  //putchar('B');
  //putchar(g_current+'0');
  if (*pqueue == NULL) *pqueue = &waiter;
  else
  {
    for (p = *pqueue; p->bk != NULL; p = p->bk); //find last
    p->bk = &waiter;//link g_current to queue
    waiter.fd = p;//link g_current to queue
  }
  g_tcb[g_current].state = BLOCKED;//block g_current thread
  asm sti;
  //swtch();
  CALL_SWITCH();

}
void wakeup_first(struct semaphore_waiter ** pqueue)
{
  struct semaphore_waiter *p = *pqueue;// = *pqueue;

  if (p == NULL) return; //no thread is wating,continue g_current
  asm cli;
  p->thread->state = READY;//set first blocking thread ready
  if (p->bk) p->bk->fd = NULL;
  *pqueue = p->bk;
  //putchar('W');
  //putchar(p->thread->name[1]);
  asm sti;
  return;
}


semaphore * create_semaphore(int rescount)
{
  semaphore *sem;
  sem = malloc(sizeof(semaphore));
  if (!sem) return NULL;


  sem -> max_value = rescount;
  sem -> value = rescount;
  sem -> wake_queue = NULL;
  //sem -> aquired_list = malloc(sizeof(struct aquiredtcblist));
  /*
  if(!sem->aquired_list)
  {
      free(sem);
      sem = NULL;
  }
  else // add itself to aquired list
  {
      sem->aquired_list->_tcb = &g_tcb[g_current];
      sem->aquired_list->bk = NULL;
      sem->aquired_list->fd = NULL;
  }
  */

  //printf("init sem value:%d\n", sem->value);
  return sem;
}

BOOL aquire_semaphore(semaphore * sem)//P op
{
  int i;
  if (!sem) goto FAILED;

  asm cli;//clear int flag
  //printf("%s aquire semaphore\n", g_tcb[g_current].name);

  sem -> value--;

  if (sem->value < 0)
  {
    block(&(sem->wake_queue));
  }
  asm sti;
  return TRUE;
  FAILED:
  asm sti;
  return FALSE;
}
BOOL release_semaphore(semaphore *sem)//V OP
{

  if (!sem) return FALSE; //the ptr points to NULL

  asm cli;
  sem->value++;

  if (sem->value <= 0)
  {
    wakeup_first(&(sem->wake_queue));
  }
  asm sti;
  return TRUE;
}
BOOL delete_semaphore(semaphore **p)
{
  semaphore *sem = *p;
  struct semaphore_waiter *wq;
  if (!p) return FALSE; //not a ptr addr
  if (!sem) return FALSE; //the ptr points to NULL

  for(wq = sem->wake_queue;wq!=NULL;wq=wq->bk)
  {
    wq->thread->state = READY;
  }
  free(sem);
  *p = NULL;//clear semaphore ptr

  return TRUE;
}