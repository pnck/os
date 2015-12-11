#include "main.h"
#include "sem.h"

void interrupt (*old_int8)(void);
void interrupt new_int8(void);
void interrupt test_int8(void);
void over(void);
char far *indos_ptr = 0;
char far *crit_err_ptr = 0;

int DosBusy(void);
void InitInDos(void);
int g_current;
long timecount;
struct TCB g_tcb[NTCB];
semaphore *g_semaphore;


void InitInDos(void)
{
  union REGS regs;
  struct SREGS segregs;

  regs.h.ah = GET_INDOS;
  intdosx(&regs, &regs, &segregs);
  indos_ptr = MK_FP(segregs.es, regs.x.bx);


  if (_osmajor < 3)
    crit_err_ptr = indos_ptr + 1;
  else if (_osmajor == 3 && _osminor == 0)
    crit_err_ptr = indos_ptr - 1;
  else
  {
    regs.x.ax = GET_CRIT_ERR;
    intdosx(&regs, &regs, &segregs);
    crit_err_ptr = MK_FP(segregs.ds, regs.x.si);
  }
}

int DosBusy(void)
{
  if (indos_ptr && crit_err_ptr)
    return (*indos_ptr || *crit_err_ptr);
  else
    return -1;
}

typedef void (far *funcptr)(void);
int create(char *name, funcptr func, int stlen);


void p1( )
{
  long i, j, k, t;

  for (t = 0; t < 10; t++)//semaphore
  {
    aquire_semaphore(g_semaphore);
    for (i = 0; i < 50; i++)
    {
      putchar('A');
      for (j = 0; j < 1000; j++)
        for (k = 0; k < 100; k++);
    }
    release_semaphore(&g_semaphore);
    puts("p1 line..");
  }

}

void p2( )
{
  long i, j, k, t;

  for (t = 0; t < 10; t++)//semaphore
  {
    aquire_semaphore(g_semaphore);
    for (i = 0; i < 50; i++)
    {
      putchar('b');
      for (j = 0; j < 1000; j++)
        for (k = 0; k < 100; k++);
    }
    release_semaphore(&g_semaphore);
    puts("p2 line..");
  }

}

int Find()//find a thread to dispatch
{
  int i = 0, j = 0;
  //while(g_tcb[i=((i+1)%NTCB)].state!=READY||i==g_current);
  for (; j < NTCB; j++)
  {
    if (g_tcb[j].state == READY)
    {
      if (g_tcb[j].priority > g_tcb[i].priority)//find max priority
      {
        i = j;
      }
      if (g_tcb[j].priority == g_tcb[g_current].priority && j != g_current) //same pri,switch
      {
        i = j;
      }
    }

  }
  return i;
}

void interrupt swtch()            /* 其他原因主动CPU调度  */
{
  int i;

  if (g_tcb[g_current].state != FINISHED
      && g_current != 0) // 当前线程还没结束 
    return;

  i = Find();
  if (i < 0)
    return;

  disable();
  //printf("\ntcb[%d]get cpu period\n",i);
  g_tcb[g_current].ss = _SS; //save stack state
  g_tcb[g_current].sp = _SP;

  if (g_tcb[g_current].state == RUNNING)
    g_tcb[g_current].state = READY;    /* 放入就绪队列中 */

  _SS = g_tcb[i].ss;
  _SP = g_tcb[i].sp;      //load saved stack

  g_tcb[i].state = RUNNING;
  g_current = i;
  enable();
}

void over()
{
  if (g_tcb[g_current].state == RUNNING)
  {
    disable();
    g_tcb[g_current].state = FINISHED;
    printf("thread %s finished\n", g_tcb[g_current].name);
    memset(g_tcb[g_current].name,0,sizeof(g_tcb[g_current].name));
    free(g_tcb[g_current].stack);
    enable();
  }
  swtch();
}

void InitTcb()
{
  unsigned int *tmp = 0;

  //for thread 1
  g_tcb[1].priority = 2;//run p1 first
  g_tcb[1].state = READY;
  strcpy(g_tcb[1].name, "p1");

  g_tcb[1].stack = (unsigned char *)malloc(STACKLEN);
  memset(g_tcb[1].stack, 0xff, STACKLEN);

  tmp = (unsigned int *)(g_tcb[1].stack + STACKLEN - 2);

  *tmp = FP_SEG(over);
  *(tmp - 1) = FP_OFF(over);
  *(tmp - 2) = 0x200;
  *(tmp - 3) = FP_SEG(p1);
  *(tmp - 4) = FP_OFF(p1);

  *(tmp - 9) = _ES;
  *(tmp - 10) = _DS;
  g_tcb[1].ss = FP_SEG(tmp - 13);
  g_tcb[1].sp = FP_OFF(tmp - 13);


  //for thread 2
  g_tcb[2].priority = 2;
  g_tcb[2].state = READY;
  strcpy(g_tcb[2].name, "p2");

  g_tcb[2].stack = (unsigned char *)malloc(STACKLEN);
  memset(g_tcb[2].stack, 0xff, STACKLEN);

  tmp = (unsigned int *)(g_tcb[2].stack + STACKLEN - 2);

  *tmp = FP_SEG(over);
  *(tmp - 1) = FP_OFF(over);
  *(tmp - 2) = 0x0200;
  *(tmp - 3) = FP_SEG(p2);
  *(tmp - 4) = FP_OFF(p2);

  *(tmp - 9) = _ES;
  *(tmp - 10) = _DS;
  g_tcb[2].ss = FP_SEG(tmp - 13);
  g_tcb[2].sp = FP_OFF(tmp - 13);
}

void interrupt new_int8(void)// interrupt switch period
{
  int i;

  (*old_int8)();
  timecount++;
  //puts("\ntime period!");
  if (timecount != TIMESLIP)
    return;
  else
  {
    if (DosBusy())
      return;
    else
    {
      disable();
//      asm CLI

      g_tcb[g_current].ss = _SS;
      g_tcb[g_current].sp = _SP;

      if (g_tcb[g_current].state == RUNNING)
        g_tcb[g_current].state = READY;

      i = Find();
      //printf("\ntcb[%d]get cpu period\n",i);
      /*
            if(i==g_current)
              return;
      */

      _SS = g_tcb[i].ss;
      _SP = g_tcb[i].sp;
      g_tcb[i].state = RUNNING;

      timecount = 0;
      g_current = i;
      enable();
//      asm STI

    }
  }
}


void tcb_state()
{
  int i;

  for (i = 0; i < NTCB; i++)
  {
    switch (g_tcb[i].state)
    {
    case FINISHED:
      printf("\nthe thread %d is finished!\n", i);
      break;
    case RUNNING:
      printf("the thread %d is running!\n", i);
      break;
    case READY:
      printf("the thread %d is ready!\n", i);
      break;
    case BLOCKED:
      printf("the thread %d is blocked!\n", i);
      break;
    default:
      printf("unknown state of thread %!\n", i);
    }
  }
}

int all_finished()
{
  int i;

  if (g_tcb[1].state != FINISHED || g_tcb[2].state != FINISHED)
    return -1;
  else
    return 0;
}

void releaseTcb()
{
  int i = 0;

  for (i = 1; i < NTCB; i++)
  {
    if (g_tcb[i].stack)
      free(g_tcb[i].stack);
  }
}

void main()
{
  g_semaphore = create_semaphore(1);//create a semaphore obj with 1 res

  timecount = 0;

  InitInDos();
  InitTcb();

  old_int8 = getvect(TIMEINT);

  strcpy(g_tcb[0].name, "main");
  g_tcb[0].state = RUNNING;
  g_tcb[0].priority = 0;
  g_current = 0;

  tcb_state();

  disable();
  setvect(TIMEINT, new_int8);
  enable();

  while (all_finished())
  {
    // printf("system running!\n");
  }

  puts("all child thread finished");
  g_tcb[0].name[0] = '\0';
  g_tcb[0].state = FINISHED;
  setvect(TIMEINT, old_int8);

  tcb_state();

  printf("\n Multi_task system terminated.\n");
}



