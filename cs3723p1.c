#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cs3723p1.h"

void * smrcAllocate(StorageManager *pMgr
    , short shDataSize, short shNodeType, char sbData[], SMResult *psmResult){

	short totalSize = shDataSize + 3*sizeof(short);
	smAlloc(pMgr, totalSize);
}
void smrcRemoveRef(StorageManager *pMgr
    , void *pUserData, SMResult *psmResult){

}
void smrcAssoc(StorageManager *pMgr
    , void *pUserDataFrom, char szAttrName[], void *pUserDataTo, SMResult *psmResult){

}
void smrcAddRef(StorageManager *pMgr
    , void *pUserDataTo, SMResult *psmResult){

}

void printNode(StorageManager *pMgr, void *pUserData){
	
}