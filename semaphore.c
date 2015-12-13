#include <stdlib.h>
#include "sem.h"

extern int g_current;//global var --> g_current thread
extern struct TCB g_tcb[NTCB];


void block(struct TCB **pqueue)
{
  struct TCB *tcbqueue = *pqueue,*p;
  asm cli;
  g_tcb[g_current].state = BLOCKED;//block g_current thread
  //putchar('B');
  //putchar(g_current+'0');

  for (p = tcbqueue; p->bk != NULL; p = p->bk); //find last
  p->bk = &g_tcb[g_current];//link g_current to queue
  g_tcb[g_current].fd = p;//link g_current to queue
  g_tcb[g_current].bk = NULL;

  asm sti;
  //swtch();
  asm int 0x80;

}
void wakeup_first(struct TCB ** pqueue)
{
  struct TCB *tcbqueue = *pqueue,*p;
  if (tcbqueue == NULL) return; //no thread is wating,continue g_current
  asm cli;
  for (p = tcbqueue; p->state != BLOCKED && p!=NULL; p = p->bk);//find first BLOCKED
  if(p) p->state = READY;//set first blocking thread ready
  //putchar('W');
  //putchar(p->name[1]);
  asm sti;
  return;
}


semaphore * create_semaphore(int rescount)
{
  semaphore *sem;
  if (rescount < 1) return NULL;

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
  struct TCB *p = sem->wake_queue;
  if (!sem) goto FAILED;

  asm cli;//clear int flag
  //printf("%s aquire semaphore\n", g_tcb[g_current].name);


  //check if g_current thread has aquired a semaphore
  if(p==NULL)//no thread in wakeQ, means no thread aquired
  {
    sem->wake_queue = &g_tcb[g_current];
    //link myself, means i aquired.
    sem->value--;
    asm sti;
    return TRUE;
  }

  for(; p->bk != NULL; p = p->bk)//find myself
  {
    if (p == &g_tcb[g_current]) //has aquired,do nothing
    {
      goto FAILED;
    }
  }
  if (p == &g_tcb[g_current]) //has aquired,do nothing
  {
    goto FAILED;
  }//not aquired, aquiring

  //no more malloc
  sem -> value--;

  if (sem->value < 0)
  {
    block(&(sem->wake_queue));
  }
  else
  {
    for(p=sem->wake_queue;p->bk != NULL; p = p->bk);//find last
    p->bk = &g_tcb[g_current];
    g_tcb[g_current].fd = p;//link myself
  }
  asm sti;
  return TRUE;
FAILED:
  asm sti;
  return FALSE;
}
BOOL release_semaphore(semaphore *sem)//V OP
{
  struct TCB *p;
  //empty queue and value == max_value

  if (!sem) return FALSE; //the ptr points to NULL

  //find g_current thread from wakeQ
  for (p = sem->wake_queue; p != NULL; p = p->bk)
  {
    if (p == &g_tcb[g_current]) //found
      break;
  }
  if (p == NULL) return FALSE; //not found, has not aquire a semaphore obj
  asm cli;
  sem->value++;

  if (sem->value <= 0)
  {
    wakeup_first(&(sem->wake_queue));
  }

  if (p->fd) p->fd->bk = p->bk; //unlink p
  else sem->wake_queue = p->bk;//p->bk become the first
  if (p->bk) p->bk->fd = p->fd; //unlink p

  asm sti;
  return TRUE;
}
BOOL delete_semaphore(semaphore **p)
{
  semaphore *sem = *p;
  if (!p) return FALSE; //not a ptr addr
  if (!sem) return FALSE; //the ptr points to NULL
  if (sem->value == sem->max_value)
  {
    free(sem);
    *p = NULL;//clear semaphore ptr
  }
  return TRUE;
}