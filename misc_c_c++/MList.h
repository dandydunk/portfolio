#ifndef MLIST_C
#define MLIST_C

#include <Ntddk.h>

#define POOL_TAG_CREATE_MLIST 'cml'


typedef struct _mlistcontainer {
	struct _mlistcontainer *next;
	void *data;
} MLISTCONTAINER, *PMLISTCONTAINER;

typedef struct _mlist {
	PMLISTCONTAINER firstNode;
	PMLISTCONTAINER lastNode;
	size_t count;
	KSPIN_LOCK SpinLock;
} MLIST, *PMLIST;


PMLIST MListCreate();

void MListAdd(PMLIST pMList, void *data);

void MListFree(PMLIST pMList);

void MListRemove(PMLIST pMList, void* toRemove);


#endif