#ifndef __MAIN_H__
#define __MAIN_H__


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

#define STACKLEN 4096

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


#define GET_INDOS 0x34
#define GET_CRIT_ERR 0x5d06


#endif