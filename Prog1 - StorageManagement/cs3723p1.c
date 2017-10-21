/**********************************************************************
cs3737p1.c by Kyle Widmann
Purpose:
    This file provides storage management functions to implement garbage collection
    capabilities and to memory management functions for the Storage Manager
**********************************************************************/


#include <string.h>
#include <stdio.h>

/**********************************************************************
cs3723p1.h
Purpose:
Provides type definitions for structures used by the associated storage manager
**********************************************************************/
#include "cs3723p1.h"

/*******************************Function Declarations********************************/
FreeNode *getFreeNodeFromHeap(StorageManager *pMgr,short shTotalSize);
InUseNode * initInUseNode(StorageManager *pMgr, FreeNode *pFreeNode, short shTotalSize, short shNodeType, char sbData[]);
void shrinkFreeNode(StorageManager *pMgr, FreeNode *pFreeNode, short shNewNodeSize);
void setNodeData(StorageManager *pMgr, InUseNode *pInUseNode, short shNodeType, char sbData[]);
void printNodeMeta(const char *szTitle, StorageManager *pMgr, InUseNode *pNode);
void collectCandidateNode(StorageManager *pMgr, InUseNode *pCandidateNode);

/******************** mmInit **********************************
void mmInit(StorageManager *pMgr)
Purpose:
    Initializes all of the heap memory allocated by the Storage Manager
Parameters:
    I/O     StorageManager *pMgr    Provides metadata about the user data and
                                    information for storage management.
Notes:
    Sets all heap memory to null and sets the free head pointer to the
    beginning of the heap as one large free node
**************************************************************************/
void mmInit(StorageManager *pMgr){

    pMgr->pFreeHead = (FreeNode *) pMgr->pBeginStorage;
    memset(pMgr->pFreeHead, '\0', pMgr->iHeapSize);
    pMgr->pFreeHead->cGC = 'F';
    pMgr->pFreeHead->shNodeSize = pMgr->iHeapSize;
    pMgr->pFreeHead->pFreeNext = NULL;

}

/******************** mmAllocate **********************************
void * mmAllocate(StorageManager *pMgr, short shDataSize, short shNodeType,
    char sbData[], MMResult *pmmResult)
Purpose:
    Satisfies a memory allocation request using the first free node from
    the heap that is large enough
Parameters:
    I/O     StorageManager *pMgr    Provides metadata about the user data and
                                    information for storage management.
    I       short shDataSize        Size of bits required for users data
    I       short shNodeType        Corresponds to the index in the Storage
                                    Managers array of node types and their metadata
    I       char sbData[]           Binary data that needs to be copied into the
                                    new in use node
    O       MMResult *pmmResult     Result structure for returning errors
Notes:
    NODE_OVERHEAD_SIZE -Overhead space for node type, size and cGc flag.  It
                        is defined in the header file.
Return value:
    char *pUserData pointer to the beginning of users's data in the node
**************************************************************************/
void * mmAllocate(StorageManager *pMgr, short shDataSize, short shNodeType, char sbData[]
        , MMResult *pmmResult){

    short shTotalSize;      //Total size of the node including overhead
    FreeNode *pFreeNode;    //Reference the free node to be used to allocate an in use node

    shTotalSize = NODE_OVERHEAD_SZ + shDataSize;
    pFreeNode = getFreeNodeFromHeap(pMgr, shTotalSize);

    //If no free node found, set error message and return
    if(pFreeNode == NULL){
        pmmResult->rc = RC_NOT_AVAIL;
        strcpy(pmmResult->szErrorMessage, "Not enough memory to store user data");
        return NULL;
    }

    InUseNode *pInUseNode = initInUseNode(pMgr, pFreeNode, shTotalSize, shNodeType, sbData);

    //get address of user data to return
    char *pUserData = ((char *)pInUseNode) + NODE_OVERHEAD_SZ;

    return pUserData;

}
/******************** mmMark **********************************
void mmMark(StorageManager *pMgr, MMResult *pmmResult)
Purpose:
    Loops over the entire heap and marks all nodes as a candidate for
    the garbage collector
Parameters:
    I       StorageManager *pMgr    Provides metadata about the user data and
                                    information for storage management.
    O       MMResult *pmmResult     Result structure for returning errors
Notes:
    Since the overhead structure for a free node and in use node are the same
    it just assumes each node is in use
**************************************************************************/
void mmMark(StorageManager *pMgr, MMResult *pmmResult){

    char *pNode;
    short shNodeSize;
    InUseNode *pInUseNode;

    for (pNode = pMgr->pBeginStorage; pNode < pMgr->pEndStorage; pNode += shNodeSize)
    {
        pInUseNode = (InUseNode *)pNode;
        shNodeSize = pInUseNode->shNodeSize;

        pInUseNode->cGC = 'C';
    }

}

/******************** mmFollow **********************************
void mmFollow(StorageManager *pMgr, void *pUserData, MMResult *pmmResult)
Purpose:
    Recursively follows the supplied user data node and all pointers to nodes
    to mark as in use
Parameters:
    I       StorageManager *pMgr    Provides metadata about the user data and
                                    information for storage management.
    I       void *pUserData         Pointer to the beginning of user data
                                    to follow
    O       MMResult *pmmResult     Result structure for returning errors
Notes:
    NODE_OVERHEAD_SIZE -Overhead space for node type, size and cGc flag.  It
                        is defined in the header file.
**************************************************************************/
void mmFollow(StorageManager *pMgr, void *pUserData, MMResult *pmmResult){

    //if null return
    if(pUserData == NULL){
        return;
    }

    char *pNode = ((char *) pUserData) - NODE_OVERHEAD_SZ;  //address of node start including meta attributes
    InUseNode *pInUseNode = (InUseNode *) pNode;            //Used to reference meta attributes
    MetaAttr *pAttr = NULL;                                 //To simplify looping through attributes
    short shNodeType = pInUseNode->shNodeType;              //To simplify referencing nodes type

    //if already visited return
    if(pInUseNode->cGC == 'U'){
        return;
    }

    //set our flag to in use to avoid infinite loop
    pInUseNode->cGC = 'U';

    //loop over attributes and follow pointers
    for (int iCurrAttr = pMgr->nodeTypeM[shNodeType].shBeginMetaAttr;
         pMgr->metaAttrM[iCurrAttr].shNodeType == shNodeType;
         iCurrAttr++)
    {
        pAttr = &pMgr->metaAttrM[iCurrAttr];

        //if pointer, follow it
        if(pAttr->cDataType == 'P'){
            void *pMetaAttrPointerToUserData = *((void **)&(pInUseNode->sbData[pAttr->shOffset]));
            mmFollow(pMgr, pMetaAttrPointerToUserData, pmmResult);
        }
    }

}

/******************** mmCollect **********************************
void mmCollect(StorageManager *pMgr, MMResult *pmmResult)
Purpose:
    Loops over each node in the heap and turns all candidate nodes found
    into free nodes and adds them to the free list
Parameters:
    I/O     StorageManager *pMgr    Provides metadata about the user data and
                                    information for storage management.
    O       MMResult *pmmResult     Result structure for returning error
Notes:
**************************************************************************/
void mmCollect(StorageManager *pMgr, MMResult *pmmResult){

    char *pNode;                //Address of the current node
    short shNodeSize;           //Node size of current node
    InUseNode *pCandidateNode;  //Used to cast pNode and check cGC flag

    //ignore old free list
    pMgr->pFreeHead = NULL;

    //find all candidate nodes and collect
    for (pNode = pMgr->pBeginStorage; pNode < pMgr->pEndStorage; pNode += shNodeSize)
    {
        pCandidateNode = (InUseNode *)pNode;
        shNodeSize = pCandidateNode->shNodeSize;

        if(pCandidateNode->cGC == 'C'){
            collectCandidateNode(pMgr, pCandidateNode);
        }
    }
}

/******************** collectCandidateNode **********************************
void collectCandidateNode(StorageManager *pMgr, InUseNode *pCandidateNode)
Purpose:
    Takes the candidate node and turns it into a free node, then adds it to the free list
Parameters:
    I/O     StorageManager *pMgr        Provides metadata about the user data and
                                        information for storage management.
    I/O     InUseNode *pCandidateNode   Node to be freed and added to free list
    O       MMResult *pmmResult         Result structure for returning error
Notes:
    If the previous free node is adjacent to the candidate node they
    are combined
**************************************************************************/
void collectCandidateNode(StorageManager *pMgr, InUseNode *pCandidateNode){

    FreeNode *pNewFreeNode;                         //Pointer to new free node
    char *pPreviousNodeEnd = NULL;                  //Pointer to end of previous free node
    short shNodeSize = pCandidateNode->shNodeSize;  //size of candidate node

    //if free list is not empty, get end of previous free node
    if(pMgr->pFreeHead != NULL){
        pPreviousNodeEnd = ((char *)pMgr->pFreeHead) + pMgr->pFreeHead->shNodeSize;
    }

    memset(pCandidateNode, '\0', shNodeSize);

    //if previous node is adjacent to candidate node
    if(pPreviousNodeEnd != NULL && pPreviousNodeEnd == (char *)pCandidateNode){
        printf("\tCombining %08lX with %08lX\n", ULAddr(pMgr->pFreeHead), ULAddr(pCandidateNode));
        pMgr->pFreeHead->shNodeSize += shNodeSize;
    }
    //if previous node is not adjacent to candidate node
    else
    {
        printf("\tCollecting %08lX\n", ULAddr(pCandidateNode));
        pNewFreeNode = (FreeNode *)pCandidateNode;
        pNewFreeNode->pFreeNext = pMgr->pFreeHead;
        pNewFreeNode->cGC = 'F';
        pNewFreeNode->shNodeSize = shNodeSize;
        pMgr->pFreeHead = pNewFreeNode;
    }
}

/******************** mmAssoc **********************************
void mmAssoc(StorageManager *pMgr, void *pUserDataFrom, char szAttrName[]
        , void *pUserDataTo, MMResult *pmmResult)
Purpose:
    Set an in use node attribute of type pointer to reference another in use node
Parameters:
    I       StorageManager *pMgr        Provides metadata about the user data and
                                        information for storage management.
    I/O     void *pUserDataFrom         User data whose pointer we are updating
    I       char szAttrName[]           User data attribute we are updating
    I       void *pUserDataTo           User data we the updated pointer should reference
    O       MMResult *pmmResult         Result structure for returning error
Notes:
    NODE_OVERHEAD_SIZE -Overhead space for node type, size and cGc flag.  It
                        is defined in the header file.
**************************************************************************/
void mmAssoc(StorageManager *pMgr, void *pUserDataFrom, char szAttrName[]
        , void *pUserDataTo, MMResult *pmmResult){

    char sbzErrorMessage[60];           //Buffer to store error messages
    MetaAttr *pAttr = NULL;             //Pointer to simplify reference meta attribute
    InUseNode *pInUseFrom;              //Pointer to reference node we are altering
    short shNodeType;                   //Node type index to simplify reference when looping

    pInUseFrom = (InUseNode *)((char *)pUserDataFrom - NODE_OVERHEAD_SZ);
    shNodeType = pInUseFrom->shNodeType;

    for (int iCurrAttr = pMgr->nodeTypeM[shNodeType].shBeginMetaAttr;
         pMgr->metaAttrM[iCurrAttr].shNodeType == shNodeType;
         iCurrAttr++)
    {
        if(strcmp(pMgr->metaAttrM[iCurrAttr].szAttrName, szAttrName) == 0){
            pAttr = &pMgr->metaAttrM[iCurrAttr];
            break;
        }
    }

    //If attr not found set error and return
    if(pAttr == NULL){
        pmmResult->rc = RC_ASSOC_ATTR_NOT_FOUND;
        sprintf(sbzErrorMessage, "Attribute %s was not found.", szAttrName);
        strcpy(pmmResult->szErrorMessage, sbzErrorMessage);
        return;
    }

    //If attr is not a pointer set error and return
    if(pAttr->cDataType != 'P'){
        pmmResult->rc = RC_ASSOC_ATTR_NOT_PTR;
        sprintf(sbzErrorMessage, "Attribute %s was not a pointer.", szAttrName);
        strcpy(pmmResult->szErrorMessage, sbzErrorMessage);
        return;
    }

    //update the pointer to new node
    void **ppNode = (void **)&(pInUseFrom->sbData[pAttr->shOffset]);
    *ppNode = pUserDataTo;

}

/******************** getFreeNodeFromHeap **********************************
FreeNode * getFreeNodeFromHeap(StorageManager *pMgr, short shTotalSize)
Purpose:
    Gets a free node from the heap whose size is at least as large as the size
    provided as a parameter
Parameters:
    I/O     StorageManager *pMgr        Provides metadata about the user data and
                                        information for storage management.
    I       short shTotalSize           The total size of the node request, including overhead
Notes:
Return value:
    pFree   Pointer to the first free node whose size is >= shTotalSize
    NULL    If no free node with the minimum size was found
**************************************************************************/
FreeNode * getFreeNodeFromHeap(StorageManager *pMgr, short shTotalSize){

    FreeNode *pPreviousFree = NULL;
    FreeNode *pFree = pMgr->pFreeHead;

    while(pFree != NULL){
        if(pFree->shNodeSize >= shTotalSize){
            if(pPreviousFree != NULL){
                //adjust free list to remove this free node
                pPreviousFree->pFreeNext = pFree->pFreeNext;
            }else{
                //was pFreeHead so set pFreeHead to next free node
                pMgr->pFreeHead = pFree->pFreeNext;
            }

            return pFree;
        }
        pPreviousFree = pFree;
        pFree = pFree->pFreeNext;
    }

    //No free node found, return null
    return NULL;
}

/******************** initInUseNode **********************************
InUseNode * initInUseNode(StorageManager *pMgr, FreeNode *pFreeNode,
                          short shTotalSize, short shNodeType, char *sbData)
Purpose:
    Initializes an in use node from a free node and sets the nodes data
    and attributes
Parameters:
    I/O     StorageManager *pMgr        Provides metadata about the user data and
                                        information for storage management.
    I       FreeNode *pFreeNode         Address of the freenode to use init as an
                                        in use node
    I       short shTotalSize           Total size of the in use node, including overhead
    I       short shNodeType            Corresponds to index in the Storage Manager
                                        node type array
    I       char sbData[]               Binary data to be set in the new in use node
Notes:
Return value:
    pInUseNode  Address of the new in use node
**************************************************************************/
InUseNode * initInUseNode(StorageManager *pMgr, FreeNode *pFreeNode,
                          short shTotalSize, short shNodeType, char sbData[]){

    short shMinNodeSize;            //Minimum bits required for a free node
    short shSizeOfNodeRemaining;    //How many bits are left in the free node
                                    //after subtracting the size of the new in use node

    shMinNodeSize = sizeof(FreeNode);
    shSizeOfNodeRemaining = pFreeNode->shNodeSize - shTotalSize;

    if(shSizeOfNodeRemaining <= shMinNodeSize){
        //allocate whole node
        shTotalSize = pFreeNode->shNodeSize;
    }else{
        //adjust free node size in linked list
        shrinkFreeNode(pMgr, pFreeNode, shSizeOfNodeRemaining);
    }

    InUseNode *pInUseNode = (InUseNode *) pFreeNode;
    pInUseNode->cGC = 'U';
    pInUseNode->shNodeType = shNodeType;
    pInUseNode->shNodeSize = shTotalSize;
    setNodeData(pMgr, pInUseNode, shNodeType, sbData);

    return pInUseNode;
}

/******************** shrinkFreeNode **********************************
void shrinkFreeNode(StorageManager *pMgr, FreeNode *pFreeNode, short shNewNodeSize
Purpose:
    Takes an existing free node and reduces its size to the new node size by advancing
    the start address of the free node
Parameters:
    I/O     StorageManager *pMgr        Provides metadata about the user data and
                                        information for storage management.
    I       FreeNode *pFreeNode         Address of the free node to shrink
    I       short shNewNodeSize         New size of the free node
Notes:
**************************************************************************/
void shrinkFreeNode(StorageManager *pMgr, FreeNode *pFreeNode, short shNewNodeSize){

    if(pFreeNode == NULL){
        return;
    }

    FreeNode *pPreviousFreeNode = NULL;     //To simplify tracking if the prior node needs to be updated
    FreeNode *pCurrFreeNode;                //Used to loop over free nodes and find previous node
    FreeNode *pNewFreeNode;                 //New address of the free node after shrinking
    short shBitsToAdvance;                  //How far to advance the free node start address

    shBitsToAdvance = pFreeNode->shNodeSize - shNewNodeSize;
    pNewFreeNode = (FreeNode *)((char *)pFreeNode + shBitsToAdvance);

    for(pCurrFreeNode = pMgr->pFreeHead; pCurrFreeNode != NULL; pCurrFreeNode = pCurrFreeNode->pFreeNext){
        if(pCurrFreeNode == pFreeNode){
            break;
        }
        pPreviousFreeNode = pCurrFreeNode;
    }

    pNewFreeNode->pFreeNext = pFreeNode->pFreeNext;
    pNewFreeNode->shNodeSize = shNewNodeSize;
    pNewFreeNode->cGC = 'F';

    //if not the first free node, update previous free node next ptr
    if(pPreviousFreeNode != NULL)
    {
        pPreviousFreeNode->pFreeNext = pNewFreeNode;
    }
    //otherwise update the pFreeHead
    else
    {
        pMgr->pFreeHead = pNewFreeNode;
    }



}

/******************** setNodeData **********************************
void setNodeData(StorageManager *pMgr, InUseNode *pInUseNode, short shNodeType, char sbData[])
Purpose:
    Sets the meta and user data for an in use node
Parameters:
    I       StorageManager *pMgr        Provides metadata about the user data and
                                        information for storage management.
    I/O     FreeNode *pFreeNode         Address of the free node to shrink
    I       short shNodeType            Corresponds to index in the Storage Manager
                                        node type array
    I       char sbData[]               Binary data to be set in the new in use node
Notes:
    Include any special assumptions.  If global variables are referenced,
    reference them.  Explain critical parts of the algorithm.
Return value:
    List the values returned by the function.  Do not list returned
    parameters.  Remove if void.
**************************************************************************/
void setNodeData(StorageManager *pMgr, InUseNode *pInUseNode, short shNodeType, char sbData[]){

    MetaAttr* pAttr;        //Simplify referencing the current meta attribute
    double *pDouble;        //Used if setting a double in the node data
    void **ppPointer;       //Used if setting a pointer in the node meta
    int *pInt;              //Used if setting an integer in the node meta

    //reset sbData portion of in use node
    memset(pInUseNode->sbData, '\0', pMgr->nodeTypeM[shNodeType].shNodeTotalSize);

    //loop over this node types meta attributes
    for (int iCurrAttr = pMgr->nodeTypeM[shNodeType].shBeginMetaAttr; pMgr->metaAttrM[iCurrAttr].shNodeType == shNodeType; iCurrAttr++)
    {

        pAttr = &pMgr->metaAttrM[iCurrAttr];

        switch(pAttr->cDataType){
            case 'S':
                strcpy(&pInUseNode->sbData[pAttr->shOffset], &(sbData[pAttr->shOffset]));
                break;
            case 'I':  // int
                pInt = (int *)&pInUseNode->sbData[pAttr->shOffset];
                *pInt = *((int *)&sbData[pAttr->shOffset]);
                break;
            case 'P':
                ppPointer = (void **)&pInUseNode->sbData[pAttr->shOffset];
                *ppPointer =*((void **)&(sbData[pAttr->shOffset]));
                break;
            case 'D':
                pDouble = (double*)&pInUseNode->sbData[pAttr->shOffset];
                *pDouble = *((double *)&(sbData[pAttr->shOffset]));
                break;
            default:
                break;

        }
    }
}