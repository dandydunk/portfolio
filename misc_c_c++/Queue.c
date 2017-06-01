#pragma warning(disable : 4100)
#pragma warning(disable : 4101)
#include "Queue.h"

PCQUEUE CQueueCreate() {
	PCQUEUE queue = (PCQUEUE)ExAllocatePoolWithTag(NonPagedPool, sizeof(CQUEUE), 'pcq');
	if (!queue) {
		return NULL;
	}

	RtlZeroMemory(queue, sizeof(CQUEUE));

	//queue->SpinLock = (PKSPIN_LOCK)ExAllocatePoolWithTag(NonPagedPool, sizeof(KSPIN_LOCK), 'ksl');
	queue->Size = 0;
	queue->refs = 1;
	KeInitializeSpinLock(&queue->SpinLock);
	KeInitializeEvent(&queue->eventQueue, NotificationEvent, FALSE);

	return queue;
}

void CQueueAddRef(PCQUEUE queue) {
	KLOCK_QUEUE_HANDLE lockHandle;
	KeAcquireInStackQueuedSpinLock(&queue->SpinLock, &lockHandle);
	++(queue->refs);
	KeReleaseInStackQueuedSpinLock(&lockHandle);
}

void CQueuePush(PCQUEUE queue, void *data) {
	KLOCK_QUEUE_HANDLE lockHandle;

	PCQUEUEITEM qItem = (PCQUEUEITEM)ExAllocatePoolWithTag(NonPagedPool, sizeof(CQUEUEITEM), 'cqi');
	if (!qItem) {
		return;
	}
	RtlZeroMemory(qItem, sizeof(CQUEUEITEM));
	qItem->Data = data;

	KeAcquireInStackQueuedSpinLock(&queue->SpinLock, &lockHandle);

	if (!queue->First) {
		queue->First = qItem;
		queue->Last = qItem;
	}

	else {
		queue->Last->Next = qItem;
		queue->Last = qItem;
	}

	++(queue->Size);

	KeReleaseInStackQueuedSpinLock(&lockHandle);
}

void* CQueuePop(PCQUEUE queue) {
	void *qItemData = NULL;
	
	KLOCK_QUEUE_HANDLE lockHandle;
	PCQUEUEITEM poppedItem = NULL;
	

	KeAcquireInStackQueuedSpinLock(&queue->SpinLock, &lockHandle);
	if (queue && queue->Size) {
		poppedItem = queue->First;
		if (!poppedItem) {
			KeReleaseInStackQueuedSpinLock(&lockHandle);
			return NULL;
		}
		qItemData = poppedItem->Data;

		queue->First = queue->First->Next;
		
		--(queue->Size);
	}

	if (poppedItem) {
		ExFreePoolWithTag(poppedItem, 'cqi');
	}
	KeReleaseInStackQueuedSpinLock(&lockHandle);

	return qItemData;
}

void CQueueFlush(PCQUEUE queue) {
	KLOCK_QUEUE_HANDLE lockHandle;
	int del = 0;
	KeAcquireInStackQueuedSpinLock(&queue->SpinLock, &lockHandle);
	if (!(--queue->refs)) {
		del = 1;
	}
	KeReleaseInStackQueuedSpinLock(&lockHandle);

	if (del) {
		while (CQueuePop(queue)) { continue;  }
		ExFreePoolWithTag(queue, 'pcq');
	}
}