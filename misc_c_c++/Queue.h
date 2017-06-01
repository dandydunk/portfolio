#pragma once
#include <Ntddk.h>

typedef struct ___cqueueitem {
	void *Data;
	struct ___cqueueitem *Next;
} CQUEUEITEM, *PCQUEUEITEM;

typedef struct ___CQueue {
	unsigned int Size;
	KSPIN_LOCK SpinLock;
	PCQUEUEITEM First;
	PCQUEUEITEM Last;
	KEVENT eventQueue;
	int refs;
} CQUEUE, *PCQUEUE;

PCQUEUE CQueueCreate();
void CQueuePush(PCQUEUE queue, void *data);
void * CQueuePop(PCQUEUE queue);
void CQueueFlush(PCQUEUE queue);
void CQueueAddRef(PCQUEUE queue);