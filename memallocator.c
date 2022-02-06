#include <stdio.h>
#include "memallocator.h"
#include <math.h>
#define TRUE 1
#define FALSE 0

struct
{
    void* head;
    void* nextBlock;
    int size;
}descriptor;

int* GetHeaderSize(void* descriptor)
{
    return (int*)descriptor;
}

void** GetNextBlock(void* descriptor)
{
    return (void**)((char*)descriptor + sizeof(int));
}

int* GetFooterSize(void* descriptor)
{
    return (int*)((char*)descriptor + abs(*GetHeaderSize(descriptor)) - sizeof(int));
}

int memgetminimumsize()
{
    //Free block descriptor size
    //sizeof(int) + sizeof(void*) + sizeof(int) 
    return 2 * sizeof(int) + sizeof(void*);
}

int memgetblocksize()
{
    //Reserved block descriptor size = 2*sizeof(int)
    //but for the correct freeing of memory, the size of the descriptor of the free block is needed
    return 2 * sizeof(int) + sizeof(void*);
}

int meminit(void* pMemory, int size)
{
    //Not enough memory to initialize the descriptor
    if (!pMemory || (size < memgetminimumsize()))
    {
        return FALSE;
    }
    //Creating a list of free blocks
    else
    {
        *GetHeaderSize(pMemory) = size;
        *GetNextBlock(pMemory) = NULL;
        *GetFooterSize(pMemory) = size;
        descriptor.head = pMemory;
        descriptor.nextBlock = pMemory;
        descriptor.size = size;
        return TRUE;
    }

}

void* memalloc(int size)
{
    if ((size > descriptor.size - memgetblocksize()) || (size < 1))
    {
        return NULL;
    }
    void* prev = NULL;
    void* cur = descriptor.nextBlock;
    void* NextBlock = NULL;
    int actualSize = memgetblocksize() + size;
    while (cur != NULL)
    {
        //If we find a block of the right size, we stop the search
        if (*GetHeaderSize(cur) >= actualSize)
        {
            break;
        }
        prev = cur;
        cur = *GetNextBlock(cur);
    }
    if (!cur)
    {
        return NULL;
    }
    //If the difference is greater than the size of the free block descriptor, then the block is split.
    if (*GetHeaderSize(cur) > actualSize + memgetminimumsize())
    {
        NextBlock = (void*)((char*)cur + actualSize);
        *GetHeaderSize(NextBlock) = *GetHeaderSize(cur) - actualSize;
        *GetNextBlock(NextBlock) = *GetNextBlock(cur);
        *GetFooterSize(NextBlock) = *GetHeaderSize(NextBlock);
        if (!prev)
        {
            descriptor.nextBlock = NextBlock;
        }
        else
        {
            *GetNextBlock(prev) = NextBlock;
        }
        *GetHeaderSize(cur) = actualSize;
    }
    else
    {
        if (!prev)
        {
            descriptor.nextBlock = *GetNextBlock(cur);
        }
        else
        {
            *GetNextBlock(prev) = *GetNextBlock(cur);
        }
    }
    *GetHeaderSize(cur) = -*GetHeaderSize(cur);
    *GetFooterSize(cur) = *GetHeaderSize(cur);
    *GetNextBlock(cur) = NULL;
    return (void*)((char*)cur + sizeof(int) + sizeof(void*));
}

void memfree(void* p)
{
    if (p == NULL)
    {
        return;
    }
    //Current block of memory and neighboring blocks
    void* leftBlock = NULL;
    void* curBlock = NULL;
    void* rightBlock = NULL;
    int mergeRight = FALSE;
    int mergeLeft = FALSE;
    //The block to be freed
    curBlock = (void*)((char*)p - sizeof(void*) - sizeof(int));
    //Right neighbor header 
    rightBlock = (void*)((char*)curBlock + abs(*GetFooterSize(curBlock)));

    //Checking that there is space for a free block
    if ((char*)curBlock - (char*)descriptor.head > memgetblocksize())
    {
        leftBlock = (void*)((char*)curBlock - abs(*((int*)curBlock - 1)));
    }

    //Freeing the current block
    *GetHeaderSize(curBlock) *= -1;
    *GetFooterSize(curBlock) = *GetHeaderSize(curBlock);

    //Merge current block with left block
    if ((leftBlock >= descriptor.head) && (*GetHeaderSize(leftBlock) > 0))
    {
        mergeLeft = TRUE;
        *GetHeaderSize(leftBlock) += *GetHeaderSize(curBlock);
        *GetFooterSize(leftBlock) = *GetHeaderSize(leftBlock);
        curBlock = leftBlock;

    }

    //Merge current block with right block
    if ((((char*)rightBlock < (char*)descriptor.head + descriptor.size)) && ((*GetHeaderSize(rightBlock) > 0)))
    {
        if (mergeLeft == FALSE)
        {
            *GetNextBlock(curBlock) = descriptor.nextBlock;
            descriptor.nextBlock = curBlock;
        }
        void* nextBlockPrev = descriptor.nextBlock;
        void* tmp = descriptor.nextBlock;
        while (tmp != NULL)
        {
            if (*GetNextBlock(tmp) == rightBlock)
            {
                nextBlockPrev = tmp;
                break;
            }
            tmp = *GetNextBlock(tmp);
        }
        if (nextBlockPrev != rightBlock)
        {
            *GetNextBlock(nextBlockPrev) = *GetNextBlock(rightBlock);
        }
        else
        {
            descriptor.nextBlock = *GetNextBlock(rightBlock);
        }
        *GetHeaderSize(curBlock) += *GetHeaderSize(rightBlock);
        *GetFooterSize(curBlock) = *GetHeaderSize(curBlock);
        mergeRight = TRUE;
    }
    else
    {
        mergeRight = FALSE;
    }

    //If both blocks are reserved
    if ((mergeLeft == FALSE) && (mergeRight == FALSE))
    {
        *GetNextBlock(curBlock) = descriptor.nextBlock;
        descriptor.nextBlock = curBlock;
    }
}

void memdone()
{
    descriptor.head = NULL;
    descriptor.nextBlock = NULL;
    descriptor.size = 0;
}