#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cs3723p1.h"

AllocNode * getNode(void *pUserData);
int checkPoint(void *point); 

void * smrcAllocate(StorageManager *pMgr
    , short shDataSize, short shNodeType, char sbData[], SMResult *psmResult){

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
	pAlloc->shRefCount = 1;
	pAlloc->shNodeType = shNodeType;
	for(count = 0; count < shDataSize; count++){
		pAlloc->sbData[count] = sbData[count];
	}

	void *pUserData = ((char *)pAlloc) + 3*sizeof(short);
	return pUserData;
}
void smrcRemoveRef(StorageManager *pMgr
    , void *pUserData, SMResult *psmResult){

	int iAt;                        // control variable representing subscript in metaAttrM
	MetaAttr *pAttr;                // slightly simplifies referencing item in metaAttrM
	void **ppNode;                  // pointer into user data if this attribute is a pointer

	//gets the actual node's start point, not the userdata point
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
					if(*ppNode != NULL)
				{
					smrcRemoveRef(pMgr,*ppNode,psmResult);
				}
			}
		}
		smFree(pMgr, pUserData, psmResult);
	}
}
void smrcAssoc(StorageManager *pMgr
    , void *pUserDataFrom, char szAttrName[], void *pUserDataTo, SMResult *psmResult){

	//printf("---FROM------------------------------------------------\n");
	//printNode(pMgr,pUserDataFrom);
	//printf("---TO--------------------------------------------------\n");
	//printNode(pMgr,pUserDataTo);

	//printf("szAttrName: %s\n", szAttrName);
	AllocNode *pNodeFrom = getNode(pUserDataFrom);
	int iAt;                        // control variable representing subscript in metaAttrM
	MetaAttr *pAttr;                // slightly simplifies referencing item in metaAttrM
	void **ppNode;                  // pointer into user data if this attribute is a pointer


	for(iAt = pMgr->nodeTypeM[pNodeFrom->shNodeType].shBeginMetaAttr; pMgr->metaAttrM[iAt].shNodeType == pNodeFrom->shNodeType; iAt++)
	{
		pAttr = &(pMgr->metaAttrM[iAt]);
		if(strcmp(szAttrName,pAttr->szAttrName) == 0){
			//printf("MATCH FOUND: %s %c\n",pAttr->szAttrName,pAttr->cDataType);
			if(pAttr->cDataType != 'P'){
				psmResult->rc = 801;
				return;
			}
			ppNode = (void **) &(pNodeFrom->sbData[pAttr->shOffset]);
			if(*ppNode != NULL)
			{
				smrcRemoveRef(pMgr,*ppNode,psmResult);
			}
			*ppNode = pUserDataTo;
			if(*ppNode != NULL){
				smrcAddRef(pMgr,*ppNode,psmResult);
			}
		}
	}



}
void smrcAddRef(StorageManager *pMgr
    , void *pUserDataTo, SMResult *psmResult){

	AllocNode *node = getNode(pUserDataTo);
	node->shRefCount++;
}

void printNode(StorageManager *pMgr, void *pUserData){
		AllocNode *pAlloc = getNode(pUserData);
		int iAt;                        // control variable representing subscript in metaAttrM
		MetaAttr *pAttr;                // slightly simplifies referencing item in metaAttrM
		char *pszMemAtOffset;           // pointer into user data if this attribute is a string
	    int *piMemAtOffset;             // pointer into user data if this attribute is an int
	    void **ppNode;                  // pointer into user data if this attribute is a pointer
	    double *pdMemAtOffset;          // pointer into user data if this attribute is a double

		printf("\tAlloc Address\tSize\tNode Type\tRef Cnt\tData Address\n");
		printf("\t%p\t\t%d\t\t%d\t\t\t%d\t\t%p\n",pAlloc,pAlloc->shAllocSize,
			pAlloc->shNodeType,pAlloc->shRefCount,pUserData);
		printf("\t\tAttr Name\tType\tValue\n");

		for(iAt = pMgr->nodeTypeM[pAlloc->shNodeType].shBeginMetaAttr; pMgr->metaAttrM[iAt].shNodeType == pAlloc->shNodeType; iAt++)
		{
			pAttr = &(pMgr->metaAttrM[iAt]);
			printf("\t\t%-10s", pAttr->szAttrName);
			switch(pAttr->cDataType)
			{
				case 'P':
					ppNode = (void **) &(pAlloc->sbData[pAttr->shOffset]);
					printf("\tP\t\t%p\n",*ppNode);
					break;
				case 'S':
					pszMemAtOffset = (char *) &(pAlloc->sbData[pAttr->shOffset]);
					printf("\tS\t\t%s\n",pszMemAtOffset);
					break;
				case 'I':
					piMemAtOffset = (int *) &(pAlloc->sbData[pAttr->shOffset]);
					printf("\tI\t\t%d\n",*piMemAtOffset);
					break;
				case 'D':
					pdMemAtOffset = (double *) &(pAlloc->sbData[pAttr->shOffset]);
					printf("\tD\t\t%f\n",*pdMemAtOffset);
					break;
			}
		}
}

AllocNode * getNode(void *pUserData){
	AllocNode *node = (AllocNode *)(((char *)pUserData) - 3 * sizeof(short));
	return node;
}