#include "main.h"
//semaphore declaration
#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__
/*
struct aquiredtcblist
{
	struct TCB *_tcb;
	struct aquiredtcblist* fd;
	struct aquiredtcblist* bk;	
};
*/
typedef struct _tagsemaphore
{
		int max_value;
    int value;
    struct TCB *wake_queue;
    //struct aquiredtcblist *aquired_list;//threads that aquired a semaphore obj 
}semaphore;

semaphore * create_semaphore(int rescount);
BOOL aquire_semaphore(semaphore * sem);//P op
BOOL release_semaphore(semaphore **sem);//V OP

#endif