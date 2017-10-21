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
    I       StorageManager *pMgr    Provides metadata about the user data and
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

    short shTotalSize = NODE_OVERHEAD_SZ + shDataSize;
    FreeNode *pFreeNode = getFreeNodeFromHeap(pMgr, shTotalSize);

    if(pFreeNode == NULL){
        pmmResult->rc = RC_NOT_AVAIL;
        strcpy(pmmResult->szErrorMessage, "Not enough memory to store user data");
        return NULL;
    }

    InUseNode *pInUseNode = initInUseNode(pMgr, pFreeNode, shTotalSize, shNodeType, sbData);

    char *pUserData = ((char *)pInUseNode) + NODE_OVERHEAD_SZ;

    return pUserData;

}
/******************** mmMark **********************************
void mmMark(StorageManager *pMgr, MMResult *pmmResult)
Purpose:
    Loops over the entire heap and marks all nodes as a candidate for
    the garbage collector
Parameters:
    I/O     StorageManager *pMgr    Provides metadata about the user data and
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

/******************** Name of Function **********************************
function prototype
Purpose:
    Explain what the function does including a brief overview of what it
    returns.
Parameters:
    I/O     StorageManager *pMgr    Provides metadata about the user data and
                                    information for storage management.
    I       void *pUserData         Pointer to the beginning of user data
                                    to follow
    O       MMResult *pmmResult     Result structure for returning errors
Notes:
    NODE_OVERHEAD_SIZE -Overhead space for node type, size and cGc flag.  It
                        is defined in the header file.
**************************************************************************/
void mmFollow(StorageManager *pMgr, void *pUserData, MMResult *pmmResult){

    if(pUserData == NULL){
        return;
    }

    char *pNode = ((char *) pUserData) - NODE_OVERHEAD_SZ;
    InUseNode *pInUseNode = (InUseNode *) pNode;
    MetaAttr *pAttr = NULL;

    if(pInUseNode->cGC == 'U'){
        return;
    }

    //set our flag to in use to avoid infinite loop
    pInUseNode->cGC = 'U';

    short shNodeType = pInUseNode->shNodeType;

    for (int iCurrAttr = pMgr->nodeTypeM[shNodeType].shBeginMetaAttr;
         pMgr->metaAttrM[iCurrAttr].shNodeType == shNodeType;
         iCurrAttr++)
    {
        pAttr = &pMgr->metaAttrM[iCurrAttr];

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

    char *pNode;
    short shNodeSize;
    InUseNode *pCandidateNode;

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

/******************** Name of Function **********************************
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

    FreeNode *pNewFreeNode;
    char *pPreviousNodeEnd = NULL;
    short shNodeSize = pCandidateNode->shNodeSize;

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
    I/O     StorageManager *pMgr        Provides metadata about the user data and
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
                pPreviousFree->pFreeNext = pFree->pFreeNext;
            }else{
                pMgr->pFreeHead = pFree->pFreeNext;
            }

            return pFree;
        }
        pPreviousFree = pFree;
        pFree = pFree->pFreeNext;
    }

    return NULL;
}

/******************** initInUseNode **********************************
InUseNode * initInUseNode(StorageManager *pMgr, FreeNode *pFreeNode,
                          short shTotalSize, short shNodeType, char *sbData)
Purpose:
    Initializes an in use node from a free node and sets the nodes data
    and attributes
Parameters:
    List each parameter on a separate line including data type name and
    description.  Each item should begin with whether the parameter is passed
    in, out or both:
    I   Passed in.  Value isn’t modified by subroutine.
    O   Passed out. Value is returned through this parameter.
    I/O Modified. Original value is used, but this subroutine modifies it.
Notes:
    Include any special assumptions.  If global variables are referenced,
    reference them.  Explain critical parts of the algorithm.
Return value:
    List the values returned by the function.  Do not list returned
    parameters.  Remove if void.
**************************************************************************/
InUseNode * initInUseNode(StorageManager *pMgr, FreeNode *pFreeNode,
                          short shTotalSize, short shNodeType, char *sbData){

    short shMinNodeSize = sizeof(FreeNode);
    short shSizeOfNodeRemaining = pFreeNode->shNodeSize - shTotalSize;

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

/******************** Name of Function **********************************
function prototype
Purpose:
    Explain what the function does including a brief overview of what it
    returns.
Parameters:
    List each parameter on a separate line including data type name and
    description.  Each item should begin with whether the parameter is passed
    in, out or both:
    I   Passed in.  Value isn’t modified by subroutine.
    O   Passed out. Value is returned through this parameter.
    I/O Modified. Original value is used, but this subroutine modifies it.
Notes:
    Include any special assumptions.  If global variables are referenced,
    reference them.  Explain critical parts of the algorithm.
Return value:
    List the values returned by the function.  Do not list returned
    parameters.  Remove if void.
**************************************************************************/
void shrinkFreeNode(StorageManager *pMgr, FreeNode *pFreeNode, short shNewNodeSize){

    if(pFreeNode == NULL){
        return;
    }

    FreeNode *pPreviousFreeNode = NULL, *pCurrFreeNode;
    short shBitsToAdvance = pFreeNode->shNodeSize - shNewNodeSize;
    FreeNode *pNewFreeNode = (FreeNode *)(((char *)pFreeNode) + shBitsToAdvance);

    for(pCurrFreeNode = pMgr->pFreeHead; pCurrFreeNode != NULL; pCurrFreeNode = pCurrFreeNode->pFreeNext){
        if(pCurrFreeNode == pFreeNode){
            break;
        }
        pPreviousFreeNode = pCurrFreeNode;
    }

    pNewFreeNode->pFreeNext = pFreeNode->pFreeNext;
    pNewFreeNode->shNodeSize = shNewNodeSize;
    pNewFreeNode->cGC = 'F';

    if(pPreviousFreeNode != NULL){
        pPreviousFreeNode->pFreeNext = pNewFreeNode;
    }else{
        pMgr->pFreeHead = pNewFreeNode;
    }



}

/******************** Name of Function **********************************
function prototype
Purpose:
    Explain what the function does including a brief overview of what it
    returns.
Parameters:
    List each parameter on a separate line including data type name and
    description.  Each item should begin with whether the parameter is passed
    in, out or both:
    I   Passed in.  Value isn’t modified by subroutine.
    O   Passed out. Value is returned through this parameter.
    I/O Modified. Original value is used, but this subroutine modifies it.
Notes:
    Include any special assumptions.  If global variables are referenced,
    reference them.  Explain critical parts of the algorithm.
Return value:
    List the values returned by the function.  Do not list returned
    parameters.  Remove if void.
**************************************************************************/
void setNodeData(StorageManager *pMgr, InUseNode *pInUseNode, short shNodeType, char sbData[]){

    MetaAttr* pAttr;
    double *pDouble;
    void **ppPointer;
    int *pInt;

    memset(pInUseNode->sbData, '\0', pMgr->nodeTypeM[shNodeType].shNodeTotalSize);

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

/******************** Name of Function **********************************
function prototype
Purpose:
    Explain what the function does including a brief overview of what it
    returns.
Parameters:
    List each parameter on a separate line including data type name and
    description.  Each item should begin with whether the parameter is passed
    in, out or both:
    I   Passed in.  Value isn’t modified by subroutine.
    O   Passed out. Value is returned through this parameter.
    I/O Modified. Original value is used, but this subroutine modifies it.
Notes:
    Include any special assumptions.  If global variables are referenced,
    reference them.  Explain critical parts of the algorithm.
Return value:
    List the values returned by the function.  Do not list returned
    parameters.  Remove if void.
**************************************************************************/
void printNodeMeta(const char *szTitle, StorageManager *pMgr, InUseNode *pNode){

    short shNodeType = findNodeType(pMgr, pMgr->nodeTypeM[pNode->shNodeType].szNodeTypeNm);

    printf("\n\t%s\n", szTitle);
    printf("\tcGC:%-5c\tAddr: %p\n", pNode->cGC, pNode);
    for (int iCurrAttr = pMgr->nodeTypeM[shNodeType].shBeginMetaAttr; pMgr->metaAttrM[iCurrAttr].shNodeType == shNodeType; iCurrAttr++)
    {
        printf("\t%-15s\t%-5c\t%-5d", pMgr->metaAttrM[iCurrAttr].szAttrName, pMgr->metaAttrM[iCurrAttr].cDataType, pMgr->metaAttrM[iCurrAttr].shOffset);
        switch(pMgr->metaAttrM[iCurrAttr].cDataType){
            case 'S':
                printf("\t%-15s\n", &(pNode->sbData[pMgr->metaAttrM[iCurrAttr].shOffset]));
                break;
            case 'I':  // int
                printf("\t%-15d\n", *((int *)&(pNode->sbData[pMgr->metaAttrM[iCurrAttr].shOffset])));
                break;
            case 'P':
                printf("\t%-15p\n", *((void**)&(pNode->sbData[pMgr->metaAttrM[iCurrAttr].shOffset])));
                break;
            case 'D':
                printf("\t%-15f\n", *((double *)&(pNode->sbData[pMgr->metaAttrM[iCurrAttr].shOffset])));
                break;
            default:
                printf("\t%-15s\n", "Unknown data type");

        }
    }
    printf("\n");
}