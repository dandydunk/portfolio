#ifndef MHASH_H
#define MHASH_H

#include "MList.h"

#define MHASHTABLESIZE 10000
#define POOLTAGMHASHTABLE 'mht'

typedef struct __hashmapelement {
	UINT64 key;
	void *data;
} MHASHMAPELEMENT, *PMHASHMAPELEMENT;

typedef struct __mhashmapcell {
	PMLIST list;
} MHASHMAPCELL;

typedef struct __mhash {
	int size;
	MHASHMAPCELL hashCells[MHASHTABLESIZE];
	KSPIN_LOCK SpinLock;
} MHASHMAP, *PMHASHMAP;

typedef struct __e {
	int lastIndex;
	int listLastIndex;
	PMHASHMAP pHashMap;
	KLOCK_QUEUE_HANDLE lockHandle;
} MHASHMAPITERATOR, *PMHASHMAPITERATOR;

unsigned int MHashMapCreateHash(UINT64 key);
PMHASHMAP MHashMapCreate();
void* MHashMapGetByKey(PMHASHMAP pHashMap, UINT64 key);
void MHashMapRemoveByKey(PMHASHMAP pHashMap, UINT64 key);
void MHashMapInsert(PMHASHMAP pHashMap, UINT64 key, void *data);
PMHASHMAPITERATOR MHashMapGetIterator(PMHASHMAP pHashMap);
void MHashMapFreeIterator(PMHASHMAPITERATOR i);
PMHASHMAPELEMENT MHashMapIteratorNext(PMHASHMAPITERATOR iterator);
void MHashMapFlush(PMHASHMAP pHashMap);
#endif