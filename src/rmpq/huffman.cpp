/*****************************************************************************/
/* huffman.cpp                       Copyright (c) Ladislav Zezula 1998-2003 */
/*---------------------------------------------------------------------------*/
/* This module contains Huffmann (de)compression methods                     */
/*                                                                           */
/* Authors : Ladislav Zezula (ladik.zezula.net)                              */
/*           ShadowFlare     (BlakFlare@hotmail.com)                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Who  Comment                                              */
/* --------  ----  ---  -------                                              */
/* xx.xx.xx  1.00  Lad  The first version of dcmp.cpp                        */
/* 03.05.03  1.00  Lad  Added compression methods                            */
/*****************************************************************************/

#include "huffman.h"
#include <memory.h>

//-----------------------------------------------------------------------------
// Methods of the THTreeItem struct

// 1501DB70
THTreeItem * THTreeItem::Call1501DB70(THTreeItem * pLast)
{
    if(pLast == NULL)
        pLast = this + 1;
    return pLast;
}

// Gets previous Huffman tree item (?)
THTreeItem * THTreeItem::GetPrevItem(long value)
{
    if(PTR_INT(prev) < 0)
        return PTR_NOT(prev);

    if(value < 0)
        value = this - next->prev;
    return prev + value;

// OLD VERSION
//  if(PTR_INT(value) < 0)
//      value = PTR_INT((item - item->next->prev));
//  return (THTreeItem *)((char *)prev + value);
}

// 1500F5E0
void THTreeItem::ClearItemLinks()
{
    next = prev = NULL;
}

// 1500BC90
void THTreeItem::RemoveItem()
{
    THTreeItem * pTemp;                // EDX

    if(next != NULL)
    {
        pTemp = prev;
        
        if(PTR_INT(pTemp) <= 0)
            pTemp = PTR_NOT(pTemp);
        else
            pTemp += (this - next->prev);

        pTemp->next = next;
        next->prev  = prev;
        next = prev = NULL;
    }
}

/*
// OLD VERSION : Removes item from the tree (?)
static void RemoveItem(THTreeItem * item)
{
    THTreeItem * next = item->next;     // ESI
    THTreeItem * prev = item->prev;     // EDX

    if(next == NULL)
        return;

    if(PTR_INT(prev) < 0)
        prev = PTR_NOT(prev);
    else
        // ??? usually item == next->prev, so what is it ?
        prev = (THTreeItem *)((unsigned char *)prev + (unsigned long)((unsigned char *)item - (unsigned char *)(next->prev)));

    // Remove HTree item from the chain
    prev->next = next;                  // Sets the 'first' pointer
    next->prev = item->prev;

    // Invalidate pointers
    item->next = NULL;
    item->prev = NULL;
}
*/

//-----------------------------------------------------------------------------
// TOutputStream functions

void TOutputStream::PutBits(unsigned long dwBuff, unsigned int nPutBits)
{
    dwBitBuff |= (dwBuff << nBits);
    nBits     += nPutBits;

    // Flush completed bytes
    while(nBits >= 8)
    {
        if(dwOutSize != 0)
        {
            *pbOutPos++ = (unsigned char)dwBitBuff;
            dwOutSize--;
        }

        dwBitBuff >>= 8;
        nBits      -= 8;
    }
}

//-----------------------------------------------------------------------------
// TInputStream functions

// Gets one bit from input stream
unsigned long TInputStream::GetBit()
{
    unsigned long dwBit = (dwBitBuff & 1);

    dwBitBuff >>= 1;
    if(--nBits == 0)
    {
        dwBitBuff   = *(unsigned long *)pbInBuffer;
        pbInBuffer += sizeof(unsigned long);
        nBits       = 32;
    }
    return dwBit;
}    

// Gets 7 bits from the stream
unsigned long TInputStream::Get7Bits()
{
    if(nBits <= 7)
    {
        dwBitBuff  |= *(unsigned short *)pbInBuffer << nBits;
        pbInBuffer += sizeof(unsigned short);
        nBits      += 16;
    }

    // Get 7 bits from input stream
    return (dwBitBuff & 0x7F);
}

// Gets the whole byte from the input stream.
unsigned long TInputStream::Get8Bits()
{
    unsigned long dwOneByte;

    if(nBits <= 8)
    {
        dwBitBuff  |= *(unsigned short *)pbInBuffer << nBits;
        pbInBuffer += sizeof(unsigned short);
        nBits      += 16;
    }
    
    dwOneByte   = (dwBitBuff & 0xFF);
    dwBitBuff >>= 8;
    nBits      -= 8;
    return dwOneByte;
}

//-----------------------------------------------------------------------------
// Functions for huffmann tree items

// Inserts item into the tree (?)
static void InsertItem(THTreeItem ** itemPtr, THTreeItem * item, unsigned long where, THTreeItem * item2)
{
    THTreeItem * next = item->next;     // EDI - next to the first item
    THTreeItem * prev = item->prev;     // ESI - prev to the first item
    long next2;                         // Pointer to the next item
    THTreeItem * prev2;                 // Pointer to previous item
    
    // The same code like in RemoveItem(item);
    if(next != 0)                       // If the first item already has next one
    {
        if(PTR_INT(prev) < 0)
            prev = PTR_NOT(prev);
        else
            prev += (item - next->prev);

        // 150083C1
        // Remove the item from the tree
        prev->next = next;
        next->prev = prev;

        // Invalidate 'prev' and 'next' pointer
        item->next = 0;
        item->prev = 0;
    }

    if(item2 == NULL)                   // EDX - If the second item is not entered,
        item2 = PTR_PTR(&itemPtr[1]);   // take the first tree item

    switch(where)
    {
        case SWITCH_ITEMS :             // Switch the two items
            item->next  = item2->next;  // item2->next (Pointer to pointer to first)
            item->prev  = item2->next->prev;
            item2->next->prev = item;
            item2->next = item;         // Set the first item
            return;
        
        case INSERT_ITEM:               // Insert as the last item
            item->next = item2;         // Set next item (or pointer to pointer to first item)
            item->prev = item2->prev;   // Set prev item (or last item in the tree)

            next2 = PTR_INT(itemPtr[0]);// Usually NULL
            prev2 = item2->prev;        // Prev item to the second (or last tree item)
            
            if(PTR_INT(prev2) < 0)
            {
                prev2 = PTR_NOT(prev);

                prev2->next = item;
                item2->prev = item;     // Next after last item
                return;
            }

            if(next2 < 0)
                next2 = item2 - item2->next->prev;
//              next2 = (THTreeItem *)(unsigned long)((unsigned char *)item2 - (unsigned char *)(item2->next->prev));

//          prev2 = (THTreeItem *)((char *)prev2 + (unsigned long)next2);// ???
            prev2 += next2;
            prev2->next = item;
            item2->prev = item;         // Set the next/last item
            return;

        default:
            return;
    }
}

//-----------------------------------------------------------------------------
// THuffmannTree class functions

// Builds Huffman tree. Called with the first 8 bits loaded from input stream
void THuffmannTree::BuildTree(unsigned int nCmpType)
{
    unsigned long   maxByte;            // [ESP+10] - The greatest character found in table
    THTreeItem   ** itemPtr;            // [ESP+14] - Pointer to Huffman tree item pointer array
    unsigned char * byteArray;          // [ESP+1C] - Pointer to unsigned char in Table1502A630
    THTreeItem    * child1;
    unsigned long   i;                  // egcs in linux doesn't like multiple for loops without an explicit i

    // Loop while pointer has a negative value
    while(PTR_INT(pLast) > 0)           // ESI - Last entry
    {
        THTreeItem * temp;              // EAX

        if(pLast->next != NULL)         // ESI->next
            pLast->RemoveItem();
                                        // EDI = &offs3054
        pItem3058   = PTR_PTR(&pItem3054);// [EDI+4]
        pLast->prev = pItem3058;  // EAX

        temp = PTR_PTR(&pItem3054)->GetPrevItem(PTR_INT(&pItem3050));

        temp->next = pLast;
        pItem3054  = pLast;
    }

    // Clear all pointers in HTree item array
    memset (items306C, 0, sizeof(items306C));

    maxByte = 0;                        // Greatest character found init to zero.
    itemPtr = (THTreeItem **)&items306C; // Pointer to current entry in HTree item pointer array

    // Ensure we have low 8 bits only
    nCmpType &= 0xFF;
    byteArray  = Table1502A630 + nCmpType * 258; // EDI also

    for(i = 0; i < 0x100; i++, itemPtr++)
    {
        THTreeItem * item   = pItem3058;    // Item to be created
        THTreeItem * pItem3 = pItem3058;
        unsigned char         oneByte = byteArray[i];

        // Skip all the bytes which are zero.
        if(byteArray[i] == 0)
            continue;

        // If not valid pointer, take the first available item in the array
        if(PTR_INT(item) <= 0)    
            item = &items0008[nItems++];

        // Insert this item as the top of the tree
        InsertItem(&pItem305C, item, SWITCH_ITEMS, NULL);

        item->parent    = NULL;                 // Invalidate child and parent
        item->child     = NULL;
        *itemPtr        = item;                 // Store pointer into pointer array

        item->dcmpByte  = i;                    // Store counter
        item->byteValue = oneByte;              // Store byte value
        if(oneByte >= maxByte)
        {
            maxByte = oneByte;
            continue;
        }

        // Find the first item which has byte value greater than current one byte
        if(PTR_INT((pItem3 = pLast)) > 0)        // EDI - Pointer to the last item
        {
            // 15006AF7
            if(pItem3 != NULL)
            {
                do  // 15006AFB
                {
                    if(pItem3->byteValue >= oneByte)
                        goto _15006B09;
                    pItem3 = pItem3->prev;
                }
                while(PTR_INT(pItem3) > 0);
            }
        }
        pItem3 = NULL;

        // 15006B09
        _15006B09:
        if(item->next != NULL)
            item->RemoveItem();

        // 15006B15
        if(pItem3 == NULL)
            pItem3 = PTR_PTR(&pFirst);

        // 15006B1F
        item->next = pItem3->next;
        item->prev = pItem3->next->prev;
        pItem3->next->prev = item;
        pItem3->next = item;
    }

    // 15006B4A
    for(; i < 0x102; i++)
    {
        THTreeItem ** itemPtr = &items306C[i];  // EDI

        // 15006B59
        THTreeItem * item = pItem3058;          // ESI
        if(PTR_INT(item) <= 0)
            item = &items0008[nItems++];

        InsertItem(&pItem305C, item, INSERT_ITEM, NULL);

        // 15006B89
        item->dcmpByte   = i;
        item->byteValue  = 1;
        item->parent     = NULL;
        item->child      = NULL;
        *itemPtr++ = item;
    }

    // 15006BAA
    if(PTR_INT((child1 = pLast)) > 0)               // EDI - last item (first child to item
    {
        THTreeItem * child2;                        // EBP
        THTreeItem * item;                          // ESI

        // 15006BB8
        while(PTR_INT((child2 = child1->prev)) > 0)
        {
            if(PTR_INT((item = pItem3058)) <= 0)
                item = &items0008[nItems++];

            // 15006BE3
            InsertItem(&pItem305C, item, SWITCH_ITEMS, NULL);

            // 15006BF3
            item->parent = NULL;
            item->child  = NULL;

            //EDX = child2->byteValue + child1->byteValue;
            //EAX = child1->byteValue;
            //ECX = maxByte;                        // The greatest character (0xFF usually)

            item->byteValue = child1->byteValue + child2->byteValue; // 0x02
            item->child     = child1;                                // Prev item in the 
            child1->parent  = item;
            child2->parent  = item;

            // EAX = item->byteValue;
            if(item->byteValue >= maxByte)
                maxByte = item->byteValue;
            else
            {
                THTreeItem * pItem2 = child2->prev;   // EDI

                if(PTR_INT(pItem2) > 0)
                {
                    // 15006C2D
                    do
                    {
                        if(pItem2->byteValue >= item->byteValue)
                            goto _15006C3B;
                        pItem2 = pItem2->prev;
                    }
                    while(PTR_INT(pItem2) > 0);
                }
                pItem2 = NULL;

                _15006C3B:
                if(item->next != 0)
                {
                    THTreeItem * temp4 = item->GetPrevItem(-1);
                                                                    
                    temp4->next      = item->next;                 // The first item changed
                    item->next->prev = item->prev;                 // First->prev changed to negative value
                    item->next = NULL;
                    item->prev = NULL;
                }

                // 15006C62
                if(pItem2 == NULL)
                    pItem2 = PTR_PTR(&pFirst);

                item->next = pItem2->next;                           // Set item with 0x100 byte value
                item->prev = pItem2->next->prev;                     // Set item with 0x17 byte value
                pItem2->next->prev = item;                           // Changed prev of item with
                pItem2->next = item;
            }
            // 15006C7B
            if(PTR_INT((child1 = child2->prev)) <= 0)
                break;
        }
    }
    // 15006C88
    offs0004 = 1;
}
/*
// Modifies Huffman tree. Adds new item and changes
void THuffmannTree::ModifyTree(unsigned long dwIndex)
{
    THTreeItem * pItem1 = pItem3058;                              // ESI
    THTreeItem * pSaveLast = (PTR_INT(pLast) <= 0) ? NULL : pLast;  // EBX
    THTreeItem * temp;                                              // EAX 

    // Prepare the first item to insert to the tree
    if(PTR_INT(pItem1) <= 0)
        pItem1 = &items0008[nItems++];

    // If item has any next item, remove it from the chain
    if(pItem1->next != NULL)
    {
        THTreeItem * temp = pItem1->GetPrevItem(-1);                  // EAX

        temp->next = pItem1->next;
        pItem1->next->prev = pItem1->prev;
        pItem1->next = NULL;
        pItem1->prev = NULL;
    }

    pItem1->next = PTR_PTR(&pFirst);
    pItem1->prev = pLast;
    temp = pItem1->next->GetPrevItem(PTR_INT(pItem305C));

    // 150068E9
    temp->next = pItem1;
    pLast  = pItem1;

    pItem1->parent = NULL;
    pItem1->child  = NULL;

    // 150068F6
    pItem1->dcmpByte  = pSaveLast->dcmpByte;   // Copy item index
    pItem1->byteValue = pSaveLast->byteValue;  // Copy item byte value
    pItem1->parent    = pSaveLast;             // Set parent to last item
    items306C[pSaveLast->dcmpByte] = pItem1;  // Insert item into item pointer array

    // Prepare the second item to insert into the tree
    if(PTR_INT((pItem1 = pItem3058)) <= 0)
        pItem1 = &items0008[nItems++];

    // 1500692E
    if(pItem1->next != NULL)
    {
        temp = pItem1->GetPrevItem(-1);   // EAX

        temp->next = pItem1->next;
        pItem1->next->prev = pItem1->prev;
        pItem1->next = NULL;
        pItem1->prev = NULL;
    }
    // 1500694C
    pItem1->next = PTR_PTR(&pFirst);
    pItem1->prev = pLast;
    temp = pItem1->next->GetPrevItem(PTR_INT(pItem305C));

    // 15006968
    temp->next = pItem1;
    pLast      = pItem1;

    // 1500696E
    pItem1->child     = NULL;
    pItem1->dcmpByte  = dwIndex;
    pItem1->byteValue = 0;
    pItem1->parent    = pSaveLast;
    pSaveLast->child   = pItem1;
    items306C[dwIndex] = pItem1;

    do
    {
        THTreeItem  * pItem2 = pItem1;
        THTreeItem  * pItem3;
        unsigned long byteValue;

        // 15006993
        byteValue = ++pItem1->byteValue;

        // Pass through all previous which have its value greater than byteValue
        while(PTR_INT((pItem3 = pItem2->prev)) > 0)  // EBX
        {
            if(pItem3->byteValue >= byteValue)
                goto _150069AE;

            pItem2 = pItem2->prev;
        }
        // 150069AC
        pItem3 = NULL;

        _150069AE:
        if(pItem2 == pItem1)
            continue;

        // 150069B2
        // Switch pItem2 with item
        InsertItem(&pItem305C, pItem2, SWITCH_ITEMS, pItem1);
        InsertItem(&pItem305C, pItem1, SWITCH_ITEMS, pItem3);

        // 150069D0
        // Switch parents of pItem1 and pItem2
        temp = pItem2->parent->child;
        if(pItem1 == pItem1->parent->child)
            pItem1->parent->child = pItem2;

        if(pItem2 == temp)
            pItem2->parent->child = pItem1;

        // 150069ED
        // Switch parents of pItem1 and pItem3
        temp = pItem1->parent;
        pItem1 ->parent = pItem2->parent;
        pItem2->parent = temp;
        offs0004++;
    }
    while(PTR_INT((pItem1 = pItem1->parent)) > 0);
}
*/

THTreeItem * THuffmannTree::Call1500E740(unsigned int nValue)
{
    THTreeItem * pItem1 = pItem3058;    // EDX
    THTreeItem * pItem2;                // EAX
    THTreeItem * pNext;
    THTreeItem * pPrev;
    THTreeItem ** ppItem;

    if(PTR_INT(pItem1) <= 0 || (pItem2 = pItem1) == NULL)
    {
        if((pItem2 = &items0008[nItems++]) != NULL)
            pItem1 = pItem2;
        else
            pItem1 = pFirst;
    }
    else
        pItem1 = pItem2;

    pNext = pItem1->next;
    if(pNext != NULL)
    {
        pPrev = pItem1->prev;
        if(PTR_INT(pPrev) <= 0)
            pPrev = PTR_NOT(pPrev);
        else
            pPrev += (pItem1 - pItem1->next->prev);

        pPrev->next = pNext;
        pNext->prev = pPrev;
        pItem1->next = NULL;
        pItem1->prev = NULL;
    }

    ppItem = &pFirst;       // esi
    if(nValue > 1)
    {
        // ecx = pFirst->next;
        pItem1->next = *ppItem;
        pItem1->prev = (*ppItem)->prev;

        (*ppItem)->prev = pItem2;
        *ppItem = pItem1;

        pItem2->parent = NULL;
        pItem2->child  = NULL;
    }
    else
    {
        pItem1->next = (THTreeItem *)ppItem;
        pItem1->prev = ppItem[1];
        // edi = pItem305C;
        pPrev = ppItem[1];      // ecx
        if(pPrev <= 0)
        {
            pPrev = PTR_NOT(pPrev);
            pPrev->next = pItem1;
            pPrev->prev = pItem2;

            pItem2->parent = NULL;
            pItem2->child  = NULL;
        }
        else
        {
            if(PTR_INT(pItem305C) < 0)
                pPrev += (THTreeItem *)ppItem - (*ppItem)->prev;
            else
                pPrev += PTR_INT(pItem305C);

            pPrev->next    = pItem1;
            ppItem[1]      = pItem2;
            pItem2->parent = NULL;
            pItem2->child  = NULL;
        }
    }
    return pItem2;
}

void THuffmannTree::Call1500E820(THTreeItem * pItem)
{
    THTreeItem * pItem1;                // edi
    THTreeItem * pItem2 = NULL;         // eax
    THTreeItem * pItem3;                // edx
    THTreeItem * pPrev;                 // ebx

    for(; pItem != NULL; pItem = pItem->parent)
    {
        pItem->byteValue++;
        
        for(pItem1 = pItem; ; pItem1 = pPrev)
        {
            pPrev = pItem1->prev;
            if(PTR_INT(pPrev) <= 0)
            {
                pPrev = NULL;
                break;
            }

            if(pPrev->byteValue >= pItem->byteValue)
                break;
        }

        if(pItem1 == pItem)
            continue;

        if(pItem1->next != NULL)
        {
            pItem2 = pItem1->GetPrevItem(-1);
            pItem2->next = pItem1->next;
            pItem1->next->prev = pItem1->prev;
            pItem1->next = NULL;
            pItem1->prev = NULL;
        }

        pItem2 = pItem->next;
        pItem1->next = pItem2;
        pItem1->prev = pItem2->prev;
        pItem2->prev = pItem1;
        pItem->next = pItem1;
        if((pItem2 = pItem1) != NULL)
        {
            pItem2 = pItem->GetPrevItem(-1);
            pItem2->next = pItem->next;
            pItem->next->prev = pItem->prev;
            pItem->next = NULL;
            pItem->prev = NULL;
        }

        if(pPrev == NULL)
            pPrev = PTR_PTR(&pFirst);

        pItem2       = pPrev->next;
        pItem->next  = pItem2;
        pItem->prev  = pItem2->prev;
        pItem2->prev = pItem;
        pPrev->next  = pItem;

        pItem3 = pItem1->parent->child;
        pItem2 = pItem->parent;
        if(pItem2->child == pItem)
            pItem2->child = pItem1;
        if(pItem3 == pItem1)
            pItem1->parent->child = pItem;

        pItem2 = pItem->parent;
        pItem->parent  = pItem1->parent;
        pItem1->parent = pItem2;
        offs0004++;
    }
}

// 1500E920
unsigned int THuffmannTree::DoCompression(TOutputStream * os, unsigned char * pbInBuffer, int nInLength, int nCmpType)
{
    THTreeItem  * pItem1;
    THTreeItem  * pItem2;
    THTreeItem  * pItem3;
    THTreeItem  * pTemp;
    unsigned long dwBitBuff;
    unsigned int  nBits;
    unsigned int  nBit;

    BuildTree(nCmpType);
    bIsCmp0 = (nCmpType == 0);

    // Store the compression type into output buffer
    os->dwBitBuff |= (nCmpType << os->nBits);
    os->nBits     += 8;

    // Flush completed bytes
    while(os->nBits >= 8)
    {
        if(os->dwOutSize != 0)
        {
            *os->pbOutPos++ = (unsigned char)os->dwBitBuff;
            os->dwOutSize--;
        }

        os->dwBitBuff >>= 8;
        os->nBits      -= 8;
    }

    for(; nInLength != 0; nInLength--)
    {
        unsigned char bOneByte = *pbInBuffer++;

        if((pItem1 = items306C[bOneByte]) == NULL)
        {
            pItem2    = items306C[0x101];  // ecx
            pItem3    = pItem2->parent;    // eax
            dwBitBuff = 0;
            nBits     = 0;

            for(; pItem3 != NULL; pItem3 = pItem3->parent)
            {
                nBit      = (pItem3->child != pItem2) ? 1 : 0;
                dwBitBuff = (dwBitBuff << 1) | nBit;
                nBits++;
                pItem2  = pItem3;
            }
            os->PutBits(dwBitBuff, nBits);

            // Store the loaded byte into output stream
            os->dwBitBuff |= (bOneByte << os->nBits);
            os->nBits     += 8;

            // Flush the whole byte(s)
            while(os->nBits >= 8)
            {
                if(os->dwOutSize != 0)
                {
                    *os->pbOutPos++ = (unsigned char)os->dwBitBuff;
                    os->dwOutSize--;
                }
                os->dwBitBuff >>= 8;
                os->nBits -= 8;
            }

            pItem1 = (PTR_INT(pLast) <= 0) ? NULL : pLast;
            pItem2 = Call1500E740(1);
            pItem2->dcmpByte  = pItem1->dcmpByte;
            pItem2->byteValue = pItem1->byteValue;
            pItem2->parent    = pItem1;
            items306C[pItem2->dcmpByte] = pItem2;

            pItem2 = Call1500E740(1);
            pItem2->dcmpByte  = bOneByte;
            pItem2->byteValue = 0;
            pItem2->parent    = pItem1;
            items306C[pItem2->dcmpByte] = pItem2;
            pItem1->child = pItem2;

            Call1500E820(pItem2);

            if(bIsCmp0 != 0)
            {
                Call1500E820(items306C[bOneByte]);
                continue;
            }

            for(pItem1 = items306C[bOneByte]; pItem1 != NULL; pItem1 = pItem1->parent)
            {
                pItem1->byteValue++;
                pItem2 = pItem1;

                for(;;)
                {
                    pItem3 = pItem2->prev;
                    if(PTR_INT(pItem3) <= 0)
                    {
                        pItem3 = NULL;
                        break;
                    }
                    if(pItem3->byteValue >= pItem1->byteValue)
                        break;
                    pItem2 = pItem3;
                }

                if(pItem2 != pItem1)
                {
                    InsertItem(&pItem305C, pItem2, SWITCH_ITEMS, pItem1);
                    InsertItem(&pItem305C, pItem1, SWITCH_ITEMS, pItem3);

                    pItem3 = pItem2->parent->child;
                    if(pItem1->parent->child == pItem1)
                        pItem1->parent->child = pItem2;

                    if(pItem3 == pItem2)
                        pItem2->parent->child = pItem1;

                    pTemp = pItem1->parent;
                    pItem1->parent = pItem2->parent;
                    pItem2->parent = pTemp;
                    offs0004++;
                }
            }
        }
// 1500EB62
        else
        {
            dwBitBuff = 0;
            nBits = 0;
            for(pItem2 = pItem1->parent; pItem2 != NULL; pItem2 = pItem2->parent)
            {
                nBit      = (pItem2->child != pItem1) ? 1 : 0;
                dwBitBuff = (dwBitBuff << 1) | nBit;
                nBits++;
                pItem1    = pItem2;
            }
            os->PutBits(dwBitBuff, nBits);
        }

// 1500EB98
        if(bIsCmp0 != 0)
            Call1500E820(items306C[bOneByte]);  // 1500EB9D
// 1500EBAF
    } // for(; nInLength != 0; nInLength--)

// 1500EBB8
    pItem1 = items306C[0x100];
    dwBitBuff = 0;
    nBits = 0;
    for(pItem2 = pItem1->parent; pItem2 != NULL; pItem2 = pItem2->parent)
    {
        nBit      = (pItem2->child != pItem1) ? 1 : 0;
        dwBitBuff = (dwBitBuff << 1) | nBit;
        nBits++;
        pItem1    = pItem2;
    }

// 1500EBE6
    os->PutBits(dwBitBuff, nBits);

// 1500EBEF
    // Flush the remaining bits
    while(os->nBits != 0)
    {
        if(os->dwOutSize != 0)
        {
            *os->pbOutPos++ = (unsigned char)os->dwBitBuff;
            os->dwOutSize--;
        }
        os->dwBitBuff >>= 8;
        os->nBits -= ((os->nBits > 8) ? 8 : os->nBits);
    }

    return (os->pbOutPos - os->pbOutBuffer);
}

// Decompression using Huffman tree (1500E450)
unsigned int THuffmannTree::DoDecompression(unsigned char * pbOutBuffer, unsigned int dwOutLength, TInputStream * is)
{
    TQDecompress  * qd;
    THTreeItem    * pItem1;
    THTreeItem    * pItem2;
    unsigned char * pbOutPos = pbOutBuffer;
    unsigned long nBitCount;
    unsigned int nDcmpByte = 0;
    unsigned int n8Bits;                // 8 bits loaded from input stream
    unsigned int n7Bits;                // 7 bits loaded from input stream
    bool bHasQdEntry;
    
    // Test the output length. Must not be NULL.
    if(dwOutLength == 0)
        return 0;

    // Get the compression type from the input stream
    n8Bits = is->Get8Bits();

    // Build the Huffman tree
    BuildTree(n8Bits);     
    bIsCmp0 = (n8Bits == 0) ? 1 : 0;

    for(;;)
    {
        n7Bits = is->Get7Bits();            // Get 7 bits from input stream

        // Try to use quick decompression. Check TQDecompress array for corresponding item.
        // If found, ise the result byte instead.
        qd = &qd3474[n7Bits];

        // If there is a quick-pass possible (ebx)
        bHasQdEntry = (qd->offs00 >= offs0004) ? true : false;

        // If we can use quick decompress, use it.
        if(bHasQdEntry)
        {
            if(qd->nBits > 7)
            {
                is->dwBitBuff >>= 7;
                is->nBits -= 7;
                pItem1 = qd->pItem;
                goto _1500E549;
            }
            is->dwBitBuff >>= qd->nBits;
            is->nBits -= qd->nBits;
            nDcmpByte = qd->dcmpByte;
        }
        else
        {
            pItem1 = pFirst->next->prev;
            if(PTR_INT(pItem1) <= 0)
                pItem1 = NULL;
_1500E549:            
            nBitCount = 0;
            pItem2 = NULL;

            do
            {
                pItem1 = pItem1->child;     // Move down by one level
                if(is->GetBit())            // If current bit is set, move to previous
                    pItem1 = pItem1->prev;

                if(++nBitCount == 7)        // If we are at 7th bit, save current HTree item.
                    pItem2 = pItem1;
            }
            while(pItem1->child != NULL);   // Walk until tree has no deeper level

            if(bHasQdEntry == false)
            {
                if(nBitCount > 7)
                {
                    qd->offs00 = offs0004;
                    qd->nBits  = nBitCount;
                    qd->pItem  = pItem2;
                }
                else
                {
                    unsigned long nIndex = n7Bits & (0xFFFFFFFF >> (32 - nBitCount));
                    unsigned long nAdd   = (1 << nBitCount);
                    
                    for(qd = &qd3474[nIndex]; nIndex <= 0x7F; nIndex += nAdd, qd += nAdd)
                    {
                        qd->offs00   = offs0004;
                        qd->nBits    = nBitCount;
                        qd->dcmpByte = pItem1->dcmpByte;
                    }
                }
            }
            nDcmpByte = pItem1->dcmpByte;
        }

        if(nDcmpByte == 0x101)          // Huffman tree needs to be modified
        {
            n8Bits = is->Get8Bits();
            pItem1 = (pLast <= 0) ? NULL : pLast;

            pItem2 = Call1500E740(1);
            pItem2->parent    = pItem1;
            pItem2->dcmpByte  = pItem1->dcmpByte;
            pItem2->byteValue = pItem1->byteValue;
            items306C[pItem2->dcmpByte] = pItem2;

            pItem2 = Call1500E740(1);
            pItem2->parent    = pItem1;
            pItem2->dcmpByte  = n8Bits;
            pItem2->byteValue = 0;
            items306C[pItem2->dcmpByte] = pItem2;

            pItem1->child = pItem2;
            Call1500E820(pItem2);
            if(bIsCmp0 == 0)
                Call1500E820(items306C[n8Bits]);

            nDcmpByte = n8Bits;
        }

        if(nDcmpByte == 0x100)
            break;

        *pbOutPos++ = (unsigned char)nDcmpByte;
        if(--dwOutLength == 0)
            break;

        if(bIsCmp0)
            Call1500E820(items306C[nDcmpByte]);
    }

    return (pbOutPos - pbOutBuffer);
}

/* OLD VERSION
unsigned int THuffmannTree::DoDecompression(unsigned char * pbOutBuffer, unsigned int dwOutLength, TInputStream * is)
{
    THTreeItem  * pItem1;               // Current item if walking HTree
    unsigned long bitCount;             // Bit counter if walking HTree
    unsigned long oneByte;              // 8 bits from bit stream/Pointer to target
    unsigned char * outPtr;             // Current pointer to output buffer
    bool          hasQDEntry;           // true if entry for quick decompression if filled
    THTreeItem  * itemAt7 = NULL;       // HTree item found at 7th bit
    THTreeItem  * temp;                 // For every use
    unsigned long dcmpByte = 0;         // Decompressed byte value
    bool          bFlag = 0;
    
    // Test the output length. Must not be NULL.
    if(dwOutLength == 0)
        return 0;

    // If too few bits in input bit buffer, we have to load next 16 bits
    is->EnsureHasMoreThan8Bits();

    // Get 8 bits from input stream
    oneByte = is->Get8Bits();

    // Build the Huffman tree
    BuildTree(oneByte);     

    bIsCmp0 = (oneByte == 0) ? 1 : 0;
    outPtr  = pbOutBuffer;              // Copy pointer to output data

    for(;;)
    {
        TQDecompress * qd;              // For quick decompress
        unsigned long sevenBits = is->Get7Bits();// 7 bits from input stream

        // Try to use quick decompression. Check TQDecompress array for corresponding item.
        // If found, ise the result byte instead.
        qd = &qd3474[sevenBits];

        // If there is a quick-pass possible
        hasQDEntry = (qd->offs00 == offs0004) ? 1 : 0;

        // Start passing the Huffman tree. Set item to tree root item
        pItem1 = pFirst;

        // If we can use quick decompress, use it.
        bFlag = 1;
        if(hasQDEntry == 1)
        {
            // Check the bit count is greater than 7, move item to 7 levels deeper
            if((bitCount = qd->bitCount) > 7)
            {
                is->dwBitBuff >>= 7;
                is->nBits  -= 7;
                pItem1      = qd->item; // Don't start with root item, but with some deeper-laying
            }
            else
            {
                // If OK, use their byte value
                is->dwBitBuff >>= bitCount;
                is->nBits  -= bitCount;
                dcmpByte    = qd->dcmpByte;
                bFlag       = 0;
            }
        }
        else
        {
            pItem1 = pFirst->next->prev;
            if(PTR_INT(pItem1) <= 0)
                pItem1 = NULL;
        }

        if(bFlag == 1)
        {
            // Walk through Huffman Tree
            bitCount = 0;               // Clear bit counter
            do
            {
                pItem1 = pItem1->child;     
                if(is->GetBit() != 0)   // If current bit is set, move to previous
                    pItem1 = pItem1->prev;  // item in current level

                if(++bitCount == 7)     // If we are at 7th bit, store current HTree item.
                    itemAt7 = pItem1;   // Store Huffman tree item
            }
            while(pItem1->child != NULL); // Walk until tree has no deeper level

            // If quick decompress entry is not filled yet, fill it.
            if(hasQDEntry == 0)
            {
                if(bitCount > 7)        // If we passed more than 7 bits, store bitCount and item
                {
                    qd->offs00   = offs0004;   // Value indicates that entry is resolved
                    qd->bitCount = bitCount;   // Number of bits passed
                    qd->item     = itemAt7;    // Store item at 7th bit
                }
                // If we passed less than 7 bits, fill entry and bit count multipliers
                else                    
                {
                    unsigned long index = sevenBits & (0xFFFFFFFF >> (32 - bitCount)); // Index for quick-decompress entry
                    unsigned long addIndex = (1 << bitCount);                          // Add value for index

                    qd = &qd3474[index];

                    do
                    {
                        qd->offs00   = offs0004;
                        qd->bitCount = bitCount;
                        qd->dcmpByte = pItem1->dcmpByte;

                        index += addIndex;
                        qd    += addIndex;
                    }
                    while(index <= 0x7F);
                }
            }
            dcmpByte = pItem1->dcmpByte;
        }

        if(dcmpByte == 0x101)        // Huffman tree needs to be modified
        {
            // Check if there is enough bits in the buffer
            is->EnsureHasMoreThan8Bits();

            // Get 8 bits from the buffer
            oneByte = is->Get8Bits();

            // Modify Huffman tree
            ModifyTree(oneByte);

            // Get lastly added tree item
            pItem1 = items306C[oneByte];

            if(bIsCmp0 == 0 && pItem1 != NULL)
            {
                // 15006F15
                do
                {
                    THTreeItem * pItem2 = pItem1;
                    THTreeItem * pItem3;
                    unsigned long byteValue;

                    byteValue = ++pItem1->byteValue;

                    while(PTR_INT((pItem3 = pItem2->prev)) > 0)
                    {
                        if(pItem3->byteValue >= byteValue)
                            goto _15006F30;

                        pItem2 = pItem2->prev;
                    }
                    pItem3 = NULL;

                    _15006F30:
                    if(pItem2 == pItem1)
                        continue;

                    InsertItem(&pItem305C, pItem2, SWITCH_ITEMS, pItem1);
                    InsertItem(&pItem305C, pItem1, SWITCH_ITEMS, pItem3);

                    temp = pItem2->parent->child;
                    if(pItem1 == pItem1->parent->child)
                        pItem1->parent->child = pItem2;

                    if(pItem2 == temp)
                        pItem2->parent->child = pItem1;

                    // Switch parents of pItem1 and pItem3
                    temp = pItem1->parent;
                    pItem1->parent = pItem2->parent;
                    pItem2->parent = temp;
                    offs0004++;
                }
                while(PTR_INT((pItem1 = pItem1->parent)) > 0);
            }
            dcmpByte = oneByte;
        }

        if(dcmpByte != 0x100)        // Not at the end of data ?
        {
            *outPtr++ = (unsigned char)dcmpByte;
            if(--dwOutLength > 0)
            {
                if(bIsCmp0 != 0)
                    Call1500E820(items306C[pItem1->byteValue]);
            }                    
            else
                break;
        }
        else
            break;
    }
    return (unsigned long)(outPtr - pbOutBuffer);
}
*/

// Table for (de)compression. Every compression type has 258 entries
unsigned char THuffmannTree::Table1502A630[] = 
{
    // Data for compression type 0x00
    0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x00, 0x00,
    
    // Data for compression type 0x01
    0x54, 0x16, 0x16, 0x0D, 0x0C, 0x08, 0x06, 0x05, 0x06, 0x05, 0x06, 0x03, 0x04, 0x04, 0x03, 0x05, 
    0x0E, 0x0B, 0x14, 0x13, 0x13, 0x09, 0x0B, 0x06, 0x05, 0x04, 0x03, 0x02, 0x03, 0x02, 0x02, 0x02, 
    0x0D, 0x07, 0x09, 0x06, 0x06, 0x04, 0x03, 0x02, 0x04, 0x03, 0x03, 0x03, 0x03, 0x03, 0x02, 0x02, 
    0x09, 0x06, 0x04, 0x04, 0x04, 0x04, 0x03, 0x02, 0x03, 0x02, 0x02, 0x02, 0x02, 0x03, 0x02, 0x04, 
    0x08, 0x03, 0x04, 0x07, 0x09, 0x05, 0x03, 0x03, 0x03, 0x03, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 
    0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x02, 0x01, 0x02, 0x02, 
    0x06, 0x0A, 0x08, 0x08, 0x06, 0x07, 0x04, 0x03, 0x04, 0x04, 0x02, 0x02, 0x04, 0x02, 0x03, 0x03, 
    0x04, 0x03, 0x07, 0x07, 0x09, 0x06, 0x04, 0x03, 0x03, 0x02, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 
    0x0A, 0x02, 0x02, 0x03, 0x02, 0x02, 0x01, 0x01, 0x02, 0x02, 0x02, 0x06, 0x03, 0x05, 0x02, 0x03, 
    0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x01, 0x01, 0x01, 
    0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x04, 0x04, 0x04, 0x07, 0x09, 0x08, 0x0C, 0x02, 
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x03, 
    0x04, 0x01, 0x02, 0x04, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 
    0x04, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
    0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
    0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x02, 0x02, 0x02, 0x06, 0x4B, 
    0x00, 0x00,
    
    // Data for compression type 0x02
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x27, 0x00, 0x00, 0x23, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x06, 0x0E, 0x10, 0x04, 
    0x06, 0x08, 0x05, 0x04, 0x04, 0x03, 0x03, 0x02, 0x02, 0x03, 0x03, 0x01, 0x01, 0x02, 0x01, 0x01, 
    0x01, 0x04, 0x02, 0x04, 0x02, 0x02, 0x02, 0x01, 0x01, 0x04, 0x01, 0x01, 0x02, 0x03, 0x03, 0x02, 
    0x03, 0x01, 0x03, 0x06, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x02, 0x01, 0x01, 
    0x01, 0x29, 0x07, 0x16, 0x12, 0x40, 0x0A, 0x0A, 0x11, 0x25, 0x01, 0x03, 0x17, 0x10, 0x26, 0x2A, 
    0x10, 0x01, 0x23, 0x23, 0x2F, 0x10, 0x06, 0x07, 0x02, 0x09, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00,
    
    // Data for compression type 0x03
    0xFF, 0x0B, 0x07, 0x05, 0x0B, 0x02, 0x02, 0x02, 0x06, 0x02, 0x02, 0x01, 0x04, 0x02, 0x01, 0x03, 
    0x09, 0x01, 0x01, 0x01, 0x03, 0x04, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 
    0x05, 0x01, 0x01, 0x01, 0x0D, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
    0x02, 0x01, 0x01, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, 
    0x0A, 0x04, 0x02, 0x01, 0x06, 0x03, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x03, 0x01, 0x01, 0x01, 
    0x05, 0x02, 0x03, 0x04, 0x03, 0x03, 0x03, 0x02, 0x01, 0x01, 0x01, 0x02, 0x01, 0x02, 0x03, 0x03, 
    0x01, 0x03, 0x01, 0x01, 0x02, 0x05, 0x01, 0x01, 0x04, 0x03, 0x05, 0x01, 0x03, 0x01, 0x03, 0x03, 
    0x02, 0x01, 0x04, 0x03, 0x0A, 0x06, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
    0x02, 0x02, 0x01, 0x0A, 0x02, 0x05, 0x01, 0x01, 0x02, 0x07, 0x02, 0x17, 0x01, 0x05, 0x01, 0x01, 
    0x0E, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
    0x06, 0x02, 0x01, 0x04, 0x05, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x07, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, 
    0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x11, 
    0x00, 0x00,
    
    // Data for compression type 0x04
    0xFF, 0xFB, 0x98, 0x9A, 0x84, 0x85, 0x63, 0x64, 0x3E, 0x3E, 0x22, 0x22, 0x13, 0x13, 0x18, 0x17, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00,
    
    // Data for compression type 0x05
    0xFF, 0xF1, 0x9D, 0x9E, 0x9A, 0x9B, 0x9A, 0x97, 0x93, 0x93, 0x8C, 0x8E, 0x86, 0x88, 0x80, 0x82, 
    0x7C, 0x7C, 0x72, 0x73, 0x69, 0x6B, 0x5F, 0x60, 0x55, 0x56, 0x4A, 0x4B, 0x40, 0x41, 0x37, 0x37, 
    0x2F, 0x2F, 0x27, 0x27, 0x21, 0x21, 0x1B, 0x1C, 0x17, 0x17, 0x13, 0x13, 0x10, 0x10, 0x0D, 0x0D, 
    0x0B, 0x0B, 0x09, 0x09, 0x08, 0x08, 0x07, 0x07, 0x06, 0x05, 0x05, 0x04, 0x04, 0x04, 0x19, 0x18, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00,
    
    // Data for compression type 0x06
    0xC3, 0xCB, 0xF5, 0x41, 0xFF, 0x7B, 0xF7, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xBF, 0xCC, 0xF2, 0x40, 0xFD, 0x7C, 0xF7, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x7A, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00,
    
    // Data for compression type 0x07
    0xC3, 0xD9, 0xEF, 0x3D, 0xF9, 0x7C, 0xE9, 0x1E, 0xFD, 0xAB, 0xF1, 0x2C, 0xFC, 0x5B, 0xFE, 0x17, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xBD, 0xD9, 0xEC, 0x3D, 0xF5, 0x7D, 0xE8, 0x1D, 0xFB, 0xAE, 0xF0, 0x2C, 0xFB, 0x5C, 0xFF, 0x18, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x70, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00,
    
    // Data for compression type 0x08
    0xBA, 0xC5, 0xDA, 0x33, 0xE3, 0x6D, 0xD8, 0x18, 0xE5, 0x94, 0xDA, 0x23, 0xDF, 0x4A, 0xD1, 0x10, 
    0xEE, 0xAF, 0xE4, 0x2C, 0xEA, 0x5A, 0xDE, 0x15, 0xF4, 0x87, 0xE9, 0x21, 0xF6, 0x43, 0xFC, 0x12, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xB0, 0xC7, 0xD8, 0x33, 0xE3, 0x6B, 0xD6, 0x18, 0xE7, 0x95, 0xD8, 0x23, 0xDB, 0x49, 0xD0, 0x11, 
    0xE9, 0xB2, 0xE2, 0x2B, 0xE8, 0x5C, 0xDD, 0x15, 0xF1, 0x87, 0xE7, 0x20, 0xF7, 0x44, 0xFF, 0x13, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x5F, 0x9E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00
};
