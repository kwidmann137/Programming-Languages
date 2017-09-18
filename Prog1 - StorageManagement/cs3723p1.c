#include "cs3723p1.h"
#include <string.h>
#include <stdio.h>

FreeNode *getFreeNodeFromHeap(StorageManager *pMgr,short shTotalSize);
InUseNode * initializeInUseNode(StorageManager *pMgr, FreeNode *pFreeNode, short shTotalSize, short shNodeType, char sbData[]);
void addToFreeList(StorageManager *pMgr, FreeNode *pFreeNode, short shNodeSize);
void setNodeData(StorageManager *pMgr, InUseNode *pInUseNode, short shNodeType, char sbData[]);

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
    memset(pMgr->pFreeHead, '0', pMgr->iHeapSize);
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

//    /***PRINT OUT INCOMING DATA **********/
//    printf("\n");
//    for (int iCurrAttr = pMgr->nodeTypeM[shNodeType].shBeginMetaAttr; pMgr->metaAttrM[iCurrAttr].shNodeType == shNodeType; iCurrAttr++)
//    {
//        printf("\t%-15s\t%-5c\t%-5d", pMgr->metaAttrM[iCurrAttr].szAttrName, pMgr->metaAttrM[iCurrAttr].cDataType, pMgr->metaAttrM[iCurrAttr].shOffset);
//        switch(pMgr->metaAttrM[iCurrAttr].cDataType){
//            case 'S':
//                printf("\t%-15s\n", &(sbData[pMgr->metaAttrM[iCurrAttr].shOffset]));
//                break;
//            case 'I':  // int
//                printf("\t%-15d\n", *((int *)&(sbData[pMgr->metaAttrM[iCurrAttr].shOffset])));
//                break;
//            case 'P':
//                printf("\t%-15p\n", *((void**)&(sbData[pMgr->metaAttrM[iCurrAttr].shOffset])));
//                break;
//            case 'D':
//                printf("\t%-15f\n", *((double *)&(sbData[pMgr->metaAttrM[iCurrAttr].shOffset])));
//                break;
//            default:
//                printf("\t%-15s\n", "Unknown data type");
//
//        }
//    }
//    printf("\n");
//    /*********END PRINT*******************/

    if(pFreeNode == NULL){
        pmmResult->rc = RC_NOT_AVAIL;
        strcpy(pmmResult->szErrorMessage, "Not enough memory to store user data");
        return NULL;
    }

    InUseNode *pInUseNode = initializeInUseNode(pMgr, pFreeNode, shTotalSize, shNodeType, sbData);

//    /***PRINT OUT INCOMING DATA **********/
//    printf("\n");
//    for (int iCurrAttr = pMgr->nodeTypeM[shNodeType].shBeginMetaAttr; pMgr->metaAttrM[iCurrAttr].shNodeType == shNodeType; iCurrAttr++)
//    {
//        printf("\t%-15s\t%-5c\t%-5d", pMgr->metaAttrM[iCurrAttr].szAttrName, pMgr->metaAttrM[iCurrAttr].cDataType, pMgr->metaAttrM[iCurrAttr].shOffset);
//        switch(pMgr->metaAttrM[iCurrAttr].cDataType){
//            case 'S':
//                printf("\t%-15s\n", &(pInUseNode->sbData[pMgr->metaAttrM[iCurrAttr].shOffset]));
//                break;
//            case 'I':  // int
//                printf("\t%-15d\n", *((int *)&(sbData[pMgr->metaAttrM[iCurrAttr].shOffset])));
//                break;
//            case 'P':
//                printf("\t%-15p\n", *((void**)&(pInUseNode->sbData[pMgr->metaAttrM[iCurrAttr].shOffset])));
//                break;
//            case 'D':
//                printf("\t%-15f\n", *((double *)&(pInUseNode->sbData[pMgr->metaAttrM[iCurrAttr].shOffset])));
//                break;
//            default:
//                printf("\t%-15s\n", "Unknown data type");
//
//        }
//    }
//    printf("\n");
//    /*********END PRINT*******************/

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

    for (pNode = pMgr->pBeginStorage; pNode < pMgr->pEndStorage; pNode += shNodeSize)
    {
        pInUseNode = (InUseNode *)pNode;
        shNodeSize = pInUseNode->shNodeSize;

        // Change the output based on the cGC type
        pInUseNode->cGC = 'C';
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
void mmFollow(StorageManager *pMgr, void *pUserData, MMResult *pmmResult){

    char * pNode = ((char *) pUserData) - NODE_OVERHEAD_SZ;
    InUseNode * pInUseNode = (InUseNode *) pNode;
    MetaAttr *pAttr = NULL;

    if(pInUseNode->cGC == 'U' || pUserData == NULL){
        return;
    }

    short shNodeSubScript = findNodeType(pMgr, pMgr->nodeTypeM[pInUseNode->shNodeType].szNodeTypeNm);

    for (int iAt = pMgr->nodeTypeM[shNodeSubScript].shBeginMetaAttr; pMgr->metaAttrM[iAt].shNodeType == shNodeSubScript; iAt++)
    {
        if(pMgr->metaAttrM[iAt].cDataType == 'P'){
            short shMetaAttrOffset = pMgr->metaAttrM[iAt].shOffset;
            void *pMetaAttrPointerToNode = (void *)&(pInUseNode->sbData[shMetaAttrOffset]);
            mmFollow(pMgr, pMetaAttrPointerToNode, pmmResult);
        }
    }

    pInUseNode->cGC = 'U';

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

    short shNodeSubScript = findNodeType(pMgr, pMgr->nodeTypeM[pInUseFrom->shNodeType].szNodeTypeNm);

    for (int iAt = pMgr->nodeTypeM[shNodeSubScript].shBeginMetaAttr; pMgr->metaAttrM[iAt].shNodeType == shNodeSubScript; iAt++)
    {
        if(strcmp(pMgr->metaAttrM[iAt].szAttrName, szAttrName) == 0){
            pAttr = &pMgr->metaAttrM[iAt];
        }
    }

    if(pAttr->cDataType != 'P'){
        pmmResult->rc = RC_ASSOC_ATTR_NOT_PTR;

        //ToDo: Fix this, pretty sure it won't persist after function exits
        char sbErrorMessage[60];
        sprintf(sbErrorMessage, "Attribute %s was not a pointer.", szAttrName);
        strcpy(pmmResult->szErrorMessage, sbErrorMessage);
        return;
    }

    if(pAttr == NULL){
        pmmResult->rc = RC_ASSOC_ATTR_NOT_FOUND;

        //ToDo: Fix this, pretty sure it won't persist after function exits
        char sbErrorMessage[60];
        sprintf(sbErrorMessage, "Attribute %s was not found.", szAttrName);
        strcpy(pmmResult->szErrorMessage, sbErrorMessage);
        return;
    }


    void **ppNode = (void **)&(pInUseFrom->sbData[pAttr->shOffset]);
    *ppNode = pUserDataTo;
}

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

InUseNode * initializeInUseNode(StorageManager *pMgr, FreeNode *pFreeNode, short shTotalSize, short shNodeType, char sbData[]){

    short shMinNodeSize = sizeof(FreeNode);
    short shSizeOfNodeRemaining = pFreeNode->shNodeSize - shTotalSize;

    if(shSizeOfNodeRemaining < shMinNodeSize){
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

void addToFreeList(StorageManager *pMgr, FreeNode *pNewFreeNode, short shNodeSize){
    pNewFreeNode->pFreeNext = pMgr->pFreeHead;
    pMgr->pFreeHead = pNewFreeNode;

    pMgr->pFreeHead->cGC = 'F';
    pMgr->pFreeHead->shNodeSize = shNodeSize;
}

void setNodeData(StorageManager *pMgr, InUseNode *pInUseNode, short shNodeType, char sbData[]){

    MetaAttr* pAttr;
    double *pDouble;
    void **ppPointer;
    int *pInt;

    memset(pInUseNode->sbData, '\0', pMgr->nodeTypeM[shNodeType].shNodeTotalSize);

    for (int iCurrAttr = pMgr->nodeTypeM[shNodeType].shBeginMetaAttr; pMgr->metaAttrM[iCurrAttr].shNodeType == shNodeType; iCurrAttr++)
    {
        pAttr = &pMgr->metaAttrM[iCurrAttr];

        switch(pMgr->metaAttrM[iCurrAttr].cDataType){
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