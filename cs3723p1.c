/**********************************************************************
cs3723p1.c by Robert Brisson
Purpose:	
    This program contains the necessary storage management functions
    given a heap of data. 
Notes:
    Can set return code to RC_ASSOC_ATTR_NOT_PTR
**********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cs3723p1.h"

AllocNode * getNode(void *pUserData);

/******************** smrcAllocate **************************************
    void * smrcAllocate(StorageManager *pMgr, short shDataSize, short shNodeType, char sbData[], SMResult *psmResult)
Purpose:
    Allocates a node with the given parameters. Initializes the size,
    reference count, node type, and userdata of this node.
Parameters:
    I StorageManager *pMgr  Provides metadata about the user data and
                            information for storage management.
	I short shDataSize		The size of the userdata to be allocated.
	I short shNodeType		The unique ID of this node. This ID is used
							to group every item of userdata to this node.
	I char sbData			The userdata to be placed inside of this node.
	I SMResult *psmResult   Result structure containing a return code and
                            an error message.  For normal execution, 
                            the rc should be set to zero.
Returns:
    Pointer to the userdata.
Notes:
	- Calls smAlloc to actually allocate the memory.
    - Returns null if the space cannot be allocated.
**************************************************************************/
void * smrcAllocate(StorageManager *pMgr, short shDataSize, short shNodeType, char sbData[], SMResult *psmResult){

	int icount;											// counter to iterate over 
														// the userdata
	short totalSize = shDataSize + 3*sizeof(short);		// calculates the total size
														// of this node
	AllocNode *pAlloc = smAlloc(pMgr, totalSize);		// allocates total space
	
	/* returns null if space cannot be allocated */
	if(pAlloc == NULL){
		return NULL;
	}

	pAlloc->shAllocSize = totalSize;	// initializes total size of node
	pAlloc->shRefCount = 1;				// initializes reference count of node
	pAlloc->shNodeType = shNodeType;	// initializes node type of node
	
	/* iterates over the user data given and puts it into the node */
	for(count = 0; count < shDataSize; count++){
		pAlloc->sbData[count] = sbData[count];
	}

	void *pUserData = ((char *)pAlloc) + 3*sizeof(short);	//pointer to start of userdata
	
	return pUserData;	//returns pointer to userdata NOT allocnode
}
/******************** smrcRemoveRef **************************************
    void smrcRemoveRef(StorageManager *pMgr,void *pUserData, SMResult *psmResult)
Purpose:
    Removes a reference to the specified user data, and decrements the
    reference count. If after decrementing the reference count has reached
    zero, the node must iterate through it's userdata and check for any other
    referenced user nodes and go through those recursively. Afterwards, it must
    free this node.
Parameters:
    I StorageManager *pMgr  Provides metadata about the user data and
                            information for storage management.
	I void *pUserData		A pointer to the userdata of an AllocNode.
	I SMResult *psmResult   Result structure containing a return code and
                            an error message.  For normal execution, 
                            the rc should be set to zero.
Notes:
	- Calls smFree to free the memory.
**************************************************************************/
void smrcRemoveRef(StorageManager *pMgr, void *pUserData, SMResult *psmResult){

	int iAt;			// control variable representing subscript in metaAttrM
	MetaAttr *pAttr;	// slightly simplifies referencing item in metaAttrM
	void **ppNode;		// pointer into user data if this attribute is a pointer

	AllocNode *pAlloc = getNode(pUserData);		// gets the pointer to the
												// AllocNode of this user data
	pAlloc->shRefCount--;	// decrements reference count of allocnode

	/* checks if reference count is zero */
	if(pAlloc->shRefCount == 0){

		/* loops through each of the user data's attributes.  The subscripts start with
		shBeginMetaAttr from nodeTypeM and end when the corresponding metaAttr's node type is
		different. */
		for(iAt = pMgr->nodeTypeM[pAlloc->shNodeType].shBeginMetaAttr; pMgr->metaAttrM[iAt].shNodeType == pAlloc->shNodeType; iAt++)
		{
			pAttr = &(pMgr->metaAttrM[iAt]);	// slightly simplify the reference in 
												// the metaAttrM array

			/* checks if the given attribute is a pointer */
			if(pAttr->cDataType == 'P'){
				ppNode = (void **) &(pAlloc->sbData[pAttr->shOffset]);	// gets the actual
																		// address of this 
																		// pointer
				/* checks if this pointer is null 
				   if pointer is not null, recurses on this pointer*/
					if(*ppNode != NULL)
				{
					smrcRemoveRef(pMgr,*ppNode,psmResult);
				}
			}
		}
		/* calls smFree to free the node AFTER it has finished checking every attribute */
		smFree(pMgr, pUserData, psmResult);
	}
}
void smrcAssoc(StorageManager *pMgr
    , void *pUserDataFrom, char szAttrName[], void *pUserDataTo, SMResult *psmResult){


	//printf("szAttrName: %s\n", szAttrName);
	AllocNode *pNodeFrom = getNode(pUserDataFrom);
	int iAt;                        // control variable representing subscript in metaAttrM
	MetaAttr *pAttr;                // slightly simplifies referencing item in metaAttrM
	void **ppNode;                  // pointer into user data if this attribute is a pointer


	for(iAt = pMgr->nodeTypeM[pNodeFrom->shNodeType].shBeginMetaAttr; pMgr->metaAttrM[iAt].shNodeType == pNodeFrom->shNodeType; iAt++)
	{
		pAttr = &(pMgr->metaAttrM[iAt]);
		if(strcmp(szAttrName,pAttr->szAttrName) == 0){
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