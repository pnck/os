#ifndef __MAIN_H__
#define __MAIN_H__

#include <dos.h>
#include <stdlib.h>

typedef int BOOL;

#define TRUE 1;
#define FALSE 0;

#define NTCB 3

#define FINISHED 0
#define RUNNING 1
#define READY   2
#define BLOCKED 3

#define TIMEINT 0x08

#define TIMESLIP 5

#define STACKLEN 1024

struct TCB
{
	struct TCB *fd;//frontend
	struct TCB *bk;//backend
	unsigned char *stack;
	unsigned  int ss;
	unsigned  int sp;
	int state;
	int priority;
	char name[10];
};

void interrupt (*old_int8)(void);
void interrupt new_int8(void);
void interrupt test_int8(void);
void over(void);

#define GET_INDOS 0x34
#define GET_CRIT_ERR 0x5d06

char far *indos_ptr=0;
char far *crit_err_ptr=0;

int DosBusy(void);
void InitInDos(void);

#endif