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

#define INT_TIME 0x08

#define TIMESLIP 5

#define STACKLEN 2048

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

#define INT_SWITCH 0x80
#define CALL_SWITCH() asm int INT_SWITCH
/*
#define ASM_PUSHA\
asm push ax;\
asm push bx;\
asm push cx;\
asm push dx;\
asm push es;\
asm push ds;\
asm push di;\
asm push si;

#define ASM_POPA\
asm pop si;\
asm pop di;\
asm pop ds;\
asm pop es;\
asm pop dx;\
asm pop cx;\
asm pop bx;\
asm pop ax
*/
#endif
