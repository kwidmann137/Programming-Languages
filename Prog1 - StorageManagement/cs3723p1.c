#include "cs3723p1.h"
#include <string.h>
#include <stdio.h>

FreeNode *getFreeNodeFromHeap(StorageManager *pMgr,short shTotalSize);
InUseNode * initializeInUseNode(StorageManager *pMgr, FreeNode *pFreeNode, short shTotalSize, short shNodeType, char sbData[]);
void addToFreeList(StorageManager *pMgr, FreeNode *pFreeNode, short shNodeSize);
void setNodeData(StorageManager *pMgr, InUseNode *pInUseNode, short shNodeType, char sbData[]);
void printNodeMeta(const char *szTitle, StorageManager *pMgr, InUseNode *pNode);

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
void mmInit(StorageManager *pMgr){
    pMgr->pFreeHead = (FreeNode *) pMgr->pBeginStorage;
    memset(pMgr->pFreeHead, '\0', pMgr->iHeapSize);
    pMgr->pFreeHead->cGC = 'F';
    pMgr->pFreeHead->shNodeSize = pMgr->iHeapSize;
    pMgr->pFreeHead->pFreeNext = NULL;
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
void * mmAllocate(StorageManager *pMgr, short shDataSize, short shNodeType, char sbData[]
        , MMResult *pmmResult){

    short shTotalSize = NODE_OVERHEAD_SZ + shDataSize;
    FreeNode *pFreeNode = getFreeNodeFromHeap(pMgr, shTotalSize);

    if(pFreeNode == NULL){
        pmmResult->rc = RC_NOT_AVAIL;
        strcpy(pmmResult->szErrorMessage, "Not enough memory to store user data");
        return NULL;
    }

    InUseNode *pInUseNode = initializeInUseNode(pMgr, pFreeNode, shTotalSize, shNodeType, sbData);

    printNodeMeta("MMALLOC - POST INIT", pMgr, pInUseNode);

    char *pUserData = ((char *)pInUseNode) + NODE_OVERHEAD_SZ;

    return pUserData;

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
void mmMark(StorageManager *pMgr, MMResult *pmmResult){

    char *pNode;
    short shNodeSize;
    InUseNode *pInUseNode;

    printf("Starting Mark Phase\n");

    for (pNode = pMgr->pBeginStorage; pNode < pMgr->pEndStorage; pNode += shNodeSize)
    {
        pInUseNode = (InUseNode *)pNode;
        shNodeSize = pInUseNode->shNodeSize;

        pInUseNode->cGC = 'C';
    }

    printf("Ending Mark Phase\n");
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
void mmFollow(StorageManager *pMgr, void *pUserData, MMResult *pmmResult){

    printf("Starting follow\n");

    if(pUserData == NULL){
        return;
    }

    char *pNode = ((char *) pUserData) - NODE_OVERHEAD_SZ;
    InUseNode *pInUseNode = (InUseNode *) pNode;
    MetaAttr *pAttr = NULL;

    printNodeMeta("FOLLOW NODE INFO", pMgr, pInUseNode);

    if(pInUseNode->cGC == 'U'){
        return;
    }

    pInUseNode->cGC = 'U';

    short shNodeType = findNodeType(pMgr, pMgr->nodeTypeM[pInUseNode->shNodeType].szNodeTypeNm);

    for (int iCurrAttr = pMgr->nodeTypeM[shNodeType].shBeginMetaAttr;
         pMgr->metaAttrM[iCurrAttr].shNodeType == shNodeType;
         iCurrAttr++)
    {

        pAttr = &pMgr->metaAttrM[iCurrAttr];

        if(pAttr->cDataType == 'P'){
            void *pMetaAttrPointerToUserData = *((void **)&(pInUseNode->sbData[pAttr->shOffset]));
            printf("\t%-15p\n", pMetaAttrPointerToUserData);
            mmFollow(pMgr, pMetaAttrPointerToUserData, pmmResult);
        }
    }

    printf("Ending follow\n");

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
void mmCollect(StorageManager *pMgr, MMResult *pmmResult){

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
void mmAssoc(StorageManager *pMgr, void *pUserDataFrom, char szAttrName[]
        , void *pUserDataTo, MMResult *pmmResult){

    MetaAttr *pAttr = NULL;
    InUseNode *pInUseFrom = (InUseNode *)((char *)pUserDataFrom - NODE_OVERHEAD_SZ);

    short shNodeType = findNodeType(pMgr, pMgr->nodeTypeM[pInUseFrom->shNodeType].szNodeTypeNm);

    for (int iCurrAttr = pMgr->nodeTypeM[shNodeType].shBeginMetaAttr;
         pMgr->metaAttrM[iCurrAttr].shNodeType == shNodeType;
         iCurrAttr++)
    {
        if(strcmp(pMgr->metaAttrM[iCurrAttr].szAttrName, szAttrName) == 0){
            pAttr = &pMgr->metaAttrM[iCurrAttr];
        }
    }

    if(pAttr->cDataType != 'P'){
        pmmResult->rc = RC_ASSOC_ATTR_NOT_PTR;

        char sbErrorMessage[60];
        sprintf(sbErrorMessage, "Attribute %s was not a pointer.", szAttrName);
        strcpy(pmmResult->szErrorMessage, sbErrorMessage);
        return;
    }

    if(pAttr == NULL){
        pmmResult->rc = RC_ASSOC_ATTR_NOT_FOUND;

        char sbErrorMessage[60];
        sprintf(sbErrorMessage, "Attribute %s was not found.", szAttrName);
        strcpy(pmmResult->szErrorMessage, sbErrorMessage);
        return;
    }


    void **ppNode = (void **)&(pInUseFrom->sbData[pAttr->shOffset]);
    *ppNode = pUserDataTo;

    printNodeMeta("ASSOC - POST UPDATE", pMgr, pInUseFrom);
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
FreeNode * getFreeNodeFromHeap(StorageManager *pMgr, short shTotalSize){

    FreeNode *pPreviousFree = NULL;
    FreeNode *pFree = pMgr->pFreeHead;

    while(pFree != NULL){
        if(pFree->shNodeSize >= shTotalSize){
            if(pPreviousFree != NULL){
                pPreviousFree->pFreeNext = pFree->pFreeNext;
            }else if(pPreviousFree == NULL && pFree->pFreeNext == NULL){
                pMgr->pFreeHead = NULL;
            }
            return pFree;
        }
        pPreviousFree = pFree;
        pFree = pFree->pFreeNext;
    }

    return NULL;
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
InUseNode * initializeInUseNode(StorageManager *pMgr, FreeNode *pFreeNode,
                                short shTotalSize, short shNodeType, char sbData[]){

    short shMinNodeSize = sizeof(FreeNode);
    short shSizeOfNodeRemaining = pFreeNode->shNodeSize - shTotalSize;

    if(shSizeOfNodeRemaining <= shMinNodeSize){
        //allocate whole node
        shTotalSize = pFreeNode->shNodeSize;
    }else{
        char *pNewFreeNode = ((char *)pFreeNode) + shTotalSize;
        addToFreeList(pMgr, (FreeNode *) pNewFreeNode, shSizeOfNodeRemaining);
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
void addToFreeList(StorageManager *pMgr, FreeNode *pNewFreeNode, short shNodeSize){

    if(pNewFreeNode == NULL){
        return;
    }

    pNewFreeNode->pFreeNext = pMgr->pFreeHead;
    pMgr->pFreeHead = pNewFreeNode;

    pMgr->pFreeHead->cGC = 'F';
    pMgr->pFreeHead->shNodeSize = shNodeSize;
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

    /***PRINT OUT NODE DATA **********/
    printf("\n\t%s\n", szTitle);
    printf("\tcGC:%-5c\n", pNode->cGC);
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
    /*********END PRINT*******************/
}