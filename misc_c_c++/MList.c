#include "MList.h"

PMLIST MListCreate() {
	PMLIST pMList = (PMLIST)ExAllocatePoolWithTag(NonPagedPool, sizeof(MLIST), POOL_TAG_CREATE_MLIST);
	if (!pMList) {
		return NULL;
	}

	pMList->firstNode = NULL;
	pMList->lastNode = NULL;
	pMList->count = 0;

	//pMList->SpinLock = (PKSPIN_LOCK)ExAllocatePoolWithTag(NonPagedPool, sizeof(KSPIN_LOCK), 'ksl');

	KeInitializeSpinLock(&pMList->SpinLock);

	return pMList;
}



void MListAdd(PMLIST pMList, void *data) {
	PMLISTCONTAINER listContainer = (PMLISTCONTAINER)ExAllocatePoolWithTag(NonPagedPool, sizeof(MLISTCONTAINER), POOL_TAG_CREATE_MLIST);
	if (!listContainer) {
		return;
	}
	listContainer->next = NULL;
	listContainer->data = data;

	KLOCK_QUEUE_HANDLE lockHandle;

	KeAcquireInStackQueuedSpinLock(&pMList->SpinLock, &lockHandle);
	if (!pMList->firstNode) {
		pMList->firstNode = listContainer;
		pMList->lastNode = pMList->firstNode;
	}
	else {
		pMList->lastNode->next = listContainer;
		pMList->lastNode = listContainer;
	}

	++(pMList->count);

	KeReleaseInStackQueuedSpinLock(&lockHandle);
}

void MListFree(PMLIST pMList) {
	KLOCK_QUEUE_HANDLE lockHandle;

	KeAcquireInStackQueuedSpinLock(&pMList->SpinLock, &lockHandle);
	if (pMList->count) {
		PMLISTCONTAINER l = pMList->firstNode;
		PMLISTCONTAINER temp = NULL;
		while (l) {
			temp = l->next;
			ExFreePoolWithTag(l, POOL_TAG_CREATE_MLIST);
			l = temp;
		}
	}
	pMList->firstNode = NULL;
	pMList->lastNode = NULL;
	pMList->count = 0;
	KeReleaseInStackQueuedSpinLock(&lockHandle);
	ExFreePoolWithTag(pMList, POOL_TAG_CREATE_MLIST);
}

void MListRemove(PMLIST pMList, void *toRemove) {
	KLOCK_QUEUE_HANDLE lockHandle;

	KeAcquireInStackQueuedSpinLock(&pMList->SpinLock, &lockHandle);
	if (pMList->count) {
		PMLISTCONTAINER l = pMList->firstNode;
		PMLISTCONTAINER previousL = NULL;
		while (l) {
			if (l->data == toRemove) {
				if (l == pMList->firstNode) {
					pMList->firstNode = l->next;
				}
				else {
					previousL->next = l->next;
				}
				ExFreePoolWithTag(l, POOL_TAG_CREATE_MLIST);
				--(pMList->count);
				break;
			}
			previousL = l;
			l = l->next;
		}
	}
	KeReleaseInStackQueuedSpinLock(&lockHandle);
}