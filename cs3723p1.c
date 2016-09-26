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
/******************** smrcAssoc **************************************
    void smrcAssoc(StorageManager *pMgr, void *pUserDataFrom, char szAttrName[], void *pUserDataTo, SMResult *psmResult)
Purpose:
    Sets a pointer in the specified user data node to a new referenced user data node.
    If the original pointer given is not pointing to a pointer, returns the code 
    RC_ASSOC_NOT_PTR. If it is a pointer and currently pointing to something, it first
    calls smrcRemoveRef to decrement the reference count. If the new pointer is non-null,
    it also will increase the reference count to the node by calling smrcAddRef.
Parameters:
    I StorageManager *pMgr  Provides metadata about the user data and
                            information for storage management.
	I void *pUserDataFrom	A pointer to the userdata of an AllocNode that
							is to be changed.
	I char szAttrName		The name of the given attribute to be changed.
	I void *pUserDataTo		A pointer to the userdata of an AllocNode that
							the from var will be changed to.
	I SMResult *psmResult   Result structure containing a return code and
                            an error message.  For normal execution, 
                            the rc should be set to zero.
Notes:
	- Returns the return code RC_ASSOC_NOT_PTR if the from pointer is not a pointer.
	- calls smrcRemoveRef if the from pointer is non-null.
	- calls smrcAddRef if the to pointer is non-null.
**************************************************************************/
void smrcAssoc(StorageManager *pMgr, void *pUserDataFrom, char szAttrName[], void *pUserDataTo, SMResult *psmResult){

	AllocNode *pNodeFrom = getNode(pUserDataFrom);	// gets the pointer to the
													// AllocNode of this user data
	int iAt;                        // control variable representing subscript in metaAttrM
	MetaAttr *pAttr;                // slightly simplifies referencing item in metaAttrM
	void **ppNode;                  // pointer into user data if this attribute is a pointer

	/* loops through each of the user data's attributes.  The subscripts start with
		shBeginMetaAttr from nodeTypeM and end when the corresponding metaAttr's node type is
		different. */
	for(iAt = pMgr->nodeTypeM[pNodeFrom->shNodeType].shBeginMetaAttr; pMgr->metaAttrM[iAt].shNodeType == pNodeFrom->shNodeType; iAt++)
	{
		pAttr = &(pMgr->metaAttrM[iAt]);	// slightly simplify the reference in 
											// the metaAttrM array

		/* checks to see if found the attribute with matching name */
		if(strcmp(szAttrName,pAttr->szAttrName) == 0){
			/* checks to see if the matched attribute is a pointer.
			if not, changes the return code to RC_ASSOC_NOT_PTR */
			if(pAttr->cDataType != 'P'){
				psmResult->rc = RC_ASSOC_NOT_PTR;
				return;
			}
			ppNode = (void **) &(pNodeFrom->sbData[pAttr->shOffset]);	//grabs the address
																		//of what the attribute
																		//is pointing to
			/* if the address is non-null, removes the reference to the 
			user data node */
			if(*ppNode != NULL)
			{
				smrcRemoveRef(pMgr,*ppNode,psmResult);
			}
			*ppNode = pUserDataTo;	//changes the pointer to point to the to pointer
			/* if the address is non-null, increments the reference counter
			to the user data node */
			if(*ppNode != NULL){
				smrcAddRef(pMgr,*ppNode,psmResult);
			}
		}
	}
}
/******************** smrcAddRef **************************************
    void smrcAddRef(StorageManager *pMgr, void *pUserDataTo, SMResult *psmResult)
Purpose:
    Increases the reference count of the given user data node.
Parameters:
    I StorageManager *pMgr  Provides metadata about the user data and
                            information for storage management.
	I void *pUserDataTo		A pointer to the userdata of an AllocNode
							to be incremented.
	I SMResult *psmResult   Result structure containing a return code and
                            an error message.  For normal execution, 
                            the rc should be set to zero.
Notes:
	- Calls getNode to grab the address of the AllocNode from
	the userdata.
**************************************************************************/
void smrcAddRef(StorageManager *pMgr, void *pUserDataTo, SMResult *psmResult){

	AllocNode *node = getNode(pUserDataTo);	// gets the pointer to the
											// AllocNode of this user data
	node->shRefCount++;		//increases the reference count
}
/******************** printNode **************************************
    void printNode(StorageManager *pMgr, void *pUserData)
Purpose:
    Prints the specified node and returns a list of pointers that it references.
Parameters:
    I StorageManager *pMgr  Provides metadata about the user data and
                            information for storage management.
	I void *pUserData		A pointer to the userdata of an AllocNode
							to be printed.
Notes:
	- Calls getNode to grab the address of the AllocNode from
	the userdata.
**************************************************************************/
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

		/* loops through each of the user data's attributes.  The subscripts start with
		shBeginMetaAttr from nodeTypeM and end when the corresponding metaAttr's node type is
		different. */
		for(iAt = pMgr->nodeTypeM[pAlloc->shNodeType].shBeginMetaAttr; pMgr->metaAttrM[iAt].shNodeType == pAlloc->shNodeType; iAt++)
		{
			pAttr = &(pMgr->metaAttrM[iAt]);	// slightly simplify the reference in 
												// the metaAttrM array
			printf("\t\t%-10s", pAttr->szAttrName);	//prints the attribute name of this attribute
			/* prints out the specified data based on what type of data the attribute is */
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
/******************** getNode **************************************
    AllocNode * getNode(void *pUserData)
Purpose:
    Returns a pointer to the AllocNode of the given userdata.
Parameters:
	I void *pUserData		A pointer to the userdata of an AllocNode
							to be returned.
Returns:
    Pointer to the AllocNode.
**************************************************************************/
AllocNode * getNode(void *pUserData){
	AllocNode *node = (AllocNode *)(((char *)pUserData) - 3 * sizeof(short));	//gets the address
																				//of AllocNode
	return node;
}