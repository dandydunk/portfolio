#include "mhash.h"
#include <Ntstrsafe.h>

PMHASHMAP MHashMapCreate() {
	int loop = 0;

	PMHASHMAP pHashMap = (PMHASHMAP)ExAllocatePoolWithTag(NonPagedPool, sizeof(MHASHMAP), POOLTAGMHASHTABLE);
	if (!pHashMap) {
		return NULL;
	}

	for (; loop < MHASHTABLESIZE; loop++) {
		pHashMap->hashCells[loop].list = NULL;
	}

	//pHashMap->SpinLock = (PKSPIN_LOCK)ExAllocatePoolWithTag(NonPagedPool, sizeof(KSPIN_LOCK), POOLTAGMHASHTABLE);
	KeInitializeSpinLock(&pHashMap->SpinLock);

	return pHashMap;
}

void MHashMapFlush(PMHASHMAP pHashMap) {
	ExFreePoolWithTag(pHashMap, POOLTAGMHASHTABLE);
}

void* MHashMapGetByKey(PMHASHMAP pHashMap, UINT64 key) {
	PMLISTCONTAINER list;
	KLOCK_QUEUE_HANDLE lockHandle;
	unsigned int keyHash = MHashMapCreateHash(key);

	KeAcquireInStackQueuedSpinLock(&pHashMap->SpinLock, &lockHandle);
	PMLIST pList = pHashMap->hashCells[keyHash].list;
	if (pList) {
		list = pList->firstNode;
		while (list) {
			PMHASHMAPELEMENT e = (PMHASHMAPELEMENT)list->data;
			if (e->key == key) {
				return e->data;
			}
			list = list->next;
		}
	}
	KeReleaseInStackQueuedSpinLock(&lockHandle);

	return NULL;
}

void MHashMapRemoveByKey(PMHASHMAP pHashMap, UINT64 key) {
	unsigned int keyHash = MHashMapCreateHash(key);

	KLOCK_QUEUE_HANDLE lockHandle;
	KeAcquireInStackQueuedSpinLock(&pHashMap->SpinLock, &lockHandle);
	PMLIST pList = pHashMap->hashCells[keyHash].list;
	if (pList) {
		PMHASHMAPELEMENT elementToRemove = NULL;
		PMLISTCONTAINER list = pList->firstNode;
		while (list) {
			elementToRemove = (PMHASHMAPELEMENT)list->data;
			if (elementToRemove->key == key) {
				break;
			}

			elementToRemove = NULL;
			list = list->next;
		}

		if (elementToRemove) {
			MListRemove(pList, elementToRemove);
			if (!pList->count) {
				MListFree(pList);
				pHashMap->hashCells[keyHash].list = NULL;
			}
			ExFreePoolWithTag(elementToRemove, POOLTAGMHASHTABLE);
		}
	}
	
	KeReleaseInStackQueuedSpinLock(&lockHandle);
}

void MHashMapInsert(PMHASHMAP pHashMap, UINT64 key, void *data) {
	unsigned int keyHash = MHashMapCreateHash(key);
	KLOCK_QUEUE_HANDLE lockHandle;
	PMHASHMAPELEMENT e;

	e = (PMHASHMAPELEMENT)ExAllocatePoolWithTag(NonPagedPool, sizeof(MHASHMAPELEMENT), POOLTAGMHASHTABLE);
	if (!e) {
		return;
	}
	e->data = data;
	e->key = key;

	KeAcquireInStackQueuedSpinLock(&pHashMap->SpinLock, &lockHandle);
	if (!pHashMap->hashCells[keyHash].list) {
		pHashMap->hashCells[keyHash].list = MListCreate();
	}
	MListAdd(pHashMap->hashCells[keyHash].list, e);
	KeReleaseInStackQueuedSpinLock(&lockHandle);
}

PMHASHMAPITERATOR MHashMapGetIterator(PMHASHMAP pHashMap) {
	PMHASHMAPITERATOR i = (PMHASHMAPITERATOR)ExAllocatePoolWithTag(NonPagedPool, sizeof(MHASHMAPITERATOR), POOLTAGMHASHTABLE);
	if (!i) {
		return NULL;
	}
	i->lastIndex = 0;
	i->listLastIndex = 0;
	i->pHashMap = pHashMap;

	//KeAcquireInStackQueuedSpinLock(pHashMap->SpinLock, &i->lockHandle);

	return i;
}

void MHashMapFreeIterator(PMHASHMAPITERATOR i) {
	ExFreePoolWithTag(i, POOLTAGMHASHTABLE);
	//KeReleaseInStackQueuedSpinLock(&i->lockHandle);
}

PMHASHMAPELEMENT MHashMapIteratorNext(PMHASHMAPITERATOR iterator) {
	while (iterator->lastIndex < MHASHTABLESIZE) {
		PMLIST list = iterator->pHashMap->hashCells[iterator->lastIndex].list;
		if (list) {
			PMLISTCONTAINER listContainer = list->firstNode;
			int listIndex = 0;
			while (listContainer) {
				if (listIndex++ >= iterator->listLastIndex) {
					iterator->listLastIndex = listIndex + 1;
					PMHASHMAPELEMENT e = (PMHASHMAPELEMENT)listContainer->data;
					return e;
				}

				listContainer = listContainer->next;
			}
			iterator->listLastIndex = 0;
		}
		iterator->lastIndex++;
	}

	return NULL;
}


unsigned int MHashMapCreateHash(UINT64 key) {
	return key % MHASHTABLESIZE;
}