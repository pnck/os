#include "sem.h"

extern int g_current;//global var --> g_current thread
extern struct TCB g_tcb[NTCB];

void block(struct TCB **pqueue)
{
  struct TCB *tcbqueue = *pqueue;
  g_tcb[g_current].state = BLOCKED;//block g_current thread

  puts("block");
  if (tcbqueue == NULL) // no thread is wating ,queue is empty
  {
    tcbqueue = &g_tcb[g_current];//point to g_current thread
    tcbqueue->fd = NULL;
    tcbqueue->bk = NULL;
  }
  else
  {
    struct TCB *p;
    for (p = tcbqueue; p->bk != NULL; p = p->bk); //find last
    p->bk = &g_tcb[g_current];//link g_current to queue
    g_tcb[g_current].fd = p;//link g_current to queue
    g_tcb[g_current].bk = NULL;
  }
  enable();
  swtch();
}
void wakeup_first(struct TCB ** pqueue)
{
  struct TCB *tcbqueue = *pqueue;
  puts("wakeup");
  if (tcbqueue == NULL) return; //no thread is wating,continue g_current

  tcbqueue->state = READY;//set first wating thread ready
  tcbqueue = tcbqueue->bk;
  tcbqueue->fd->bk = NULL;//unlink g_current thread
  tcbqueue->fd = NULL;
  return;
}


semaphore * create_semaphore(int rescount)
{
  semaphore *sem;
  if (rescount < 1) return NULL;

  sem = malloc(sizeof(semaphore));
  if (!sem) return NULL;

  disable();
  sem -> max_value = rescount;
  //sem -> value = rescount - 1;
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
  enable();
  return sem;
}

BOOL aquire_semaphore(semaphore * sem)//P op
{
  struct aquiredtcblist *p;
  if (!sem) return FALSE;
  //check if g_current thread has aquired a semaphore
  for (p = sem->aquired_list; p->bk != NULL; p = p->bk)
  {
    if (p->_tcb == &g_tcb[g_current]) //has aquired,do nothing
    {
      return FALSE;
    }
  }
  if (p->_tcb == &g_tcb[g_current]) //has aquired,do nothing
  {
    return FALSE;
  }
  disable();
  p->bk = malloc(sizeof(struct aquiredtcblist));
  if (p->bk == NULL)
  {
    enable();
    return FALSE;
  }
  p->bk->fd = p;
  p = p->bk;
  p->_tcb = &g_tcb[g_current];//aquired list += g_current thread
  p->bk = NULL;
  sem -> value--;
  if (sem->value < 0)
  {
    block(&(sem->wake_queue));
  }
  enable();
  return TRUE;
}
BOOL release_semaphore(semaphore **psem)//V OP
{
  semaphore *sem = *psem;
  struct aquiredtcblist *p;
  //empty queue and value == max_value

  if (!psem) return FALSE; //not a ptr addr
  if (!sem) return FALSE; //the ptr points to NULL

  //find g_current thread from aquired list
  for (p = sem->aquired_list; p != NULL; p = p->bk)
  {
    if (p->_tcb == &g_tcb[g_current]) //found
      break;
  }
  if (p == NULL) return FALSE; //not found, has not aquire a semaphore obj
  disable();
  sem->value++;

  if (p->fd) p->fd->bk = p->bk; //unlink p
  else sem->aquired_list = p->bk;//p->bk become the first
  if (p->bk) p->bk->fd = p->fd; //unlink p

  free(p);//release g_current thread from aquired list

  if (sem->value <= 0)
  {
    wakeup_first(&(sem->wake_queue));
  }
  if (sem->value == sem->max_value)
  {
    free(sem);
    *psem = NULL;//clear semaphore ptr
  }
  enable();
  return TRUE;
}
