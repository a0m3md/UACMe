/*******************************************************************************
*
*  (C) COPYRIGHT AUTHORS, 2017 - 2025
*
*  TITLE:       XORDECRYPT.C
*
*  VERSION:     1.0
*
*  DATE:        16 Nov 2025
*
*  XOR decryption routine.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
* ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
* TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
*******************************************************************************/

#include "shared.h"

/*
* ucmXorDecrypt
*
* Purpose:
*
* In-place decryption of XOR encrypted buffer.
*
*/
VOID ucmXorDecrypt(
    _Inout_ PVOID Buffer,
    _In_ ULONG BufferSize
)
{
    ULONG k, c;
    PUCHAR ptr;

    if ((Buffer == NULL) || (BufferSize == 0))
        return;

    k = 'nacr';
    c = BufferSize;
    ptr = (PUCHAR)Buffer;

    do {
        *ptr ^= k;
        k = _rotl(k, 1);
        ptr++;
        --c;
    } while (c != 0);
}
