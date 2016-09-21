#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cs3723p1.h"

AllocNode * getNode(void *pUserData);

void * smrcAllocate(StorageManager *pMgr
    , short shDataSize, short shNodeType, char sbData[], SMResult *psmResult){

	//printf("HELLO");

	int count;

	//finds total size
	short totalSize = shDataSize + 3*sizeof(short);
	
	//allocates total space
	AllocNode *pAlloc = smAlloc(pMgr, totalSize);
	
	//returns null if space cannot be allocated
	if(pAlloc == NULL){
		return NULL;
	}

	//initializes AllocNode
	pAlloc->shAllocSize = totalSize;
	//printf("\nDEBUG - ALLOC SIZE %d  %d\n", totalSize, pAlloc->shAllocSize);
	pAlloc->shRefCount = 1;
	pAlloc->shNodeType = shNodeType;
	
	for(count = 0; count < shDataSize; count++){
		pAlloc->sbData[count] = sbData[count];
	}

	//NEEDS WORK HERE
	//void *pUserData = ((char)pAlloc) + 3*sizeof(short);
	return pAlloc;
	//return ((char)pAlloc) + 3*sizeof(short);

}
void smrcRemoveRef(StorageManager *pMgr
    , void *pUserData, SMResult *psmResult){

	//printf("HELLO1");

	int iAt;                        // control variable representing subscript in metaAttrM
	MetaAttr *pAttr;                // slightly simplifies referencing item in metaAttrM
	void **ppNode;                  // pointer into user data if this attribute is a pointer

	//gets the actual node's start point, not the userdata point
	//AllocNode *pAlloc = (AllocNode *)(((char *)pUserData) - 3 * sizeof(short));
	AllocNode *pAlloc = getNode(pUserData);

	//decrements reference count of allocnode
	pAlloc->shRefCount--;

	//if reference count is zero, must go through all user data
	//if user data is a pointer, recursively decrement reference count
	if(pAlloc->shRefCount == 0){
		for(iAt = pMgr->nodeTypeM[pAlloc->shNodeType].shBeginMetaAttr; pMgr->metaAttrM[iAt].shNodeType == pAlloc->shNodeType; iAt++)
		{
			pAttr = &(pMgr->metaAttrM[iAt]);
			if(pAttr->cDataType == 'P'){
				ppNode = (void **) &(pAlloc->sbData[pAttr->shOffset]);
				/*if(ppNode == NULL){
					return;
				}*/
				smrcRemoveRef(pMgr, *ppNode, psmResult);
			}
		}
		smFree(pMgr, pUserData, psmResult);
	}
}
void smrcAssoc(StorageManager *pMgr
    , void *pUserDataFrom, char szAttrName[], void *pUserDataTo, SMResult *psmResult){

	//printf("HELLO2");
	if(strcmp(szAttrName, "P")){
		psmResult->rc = 801;
	}
	if(pUserDataFrom != NULL){
		smrcRemoveRef(pMgr, pUserDataFrom, psmResult);
	}
	if(pUserDataTo != NULL){
		smrcAddRef(pMgr, pUserDataTo, psmResult);
	}
	pUserDataFrom = pUserDataTo;

}
void smrcAddRef(StorageManager *pMgr
    , void *pUserDataTo, SMResult *psmResult){
	//printf("HELLO3");

	AllocNode *node = getNode(pUserDataTo);
	node->shRefCount++;
}

void printNode(StorageManager *pMgr, void *pUserData){
		AllocNode *node = getNode(pUserData);
		printf("\tAlloc Address\tSize\tNode Type\tRef Cnt\tData Address\n");
		printf("\t%x\t\t%d\t%d\t\t%d\t%x\n",node,node->shAllocSize,
			node->shNodeType,node->shRefCount,pUserData);
}

AllocNode * getNode(void *pUserData){
	AllocNode *node = (AllocNode *)(((char *)pUserData) - 3 * sizeof(short));
	return node;
}