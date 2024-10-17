#include <iostream>
#include <cassert>
#include <Windows.h>
#include <winnetwk.h>

#include "utils.h"
#include "mntver.h"

int wmain()
{
    Mount *pMount = NULL;
    WSADATA wsa = {0};
    int iResult = 0;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (iResult != 0)
    {
        std::wcout << L"*** Failed to initiate windows socket\n";
        return -1;
    }

    try
    {
        pMount = new Mount();
    }
    catch (std::exception e)
    {
        std::wcout << L"Error: " << e.what() << std::endl;
        return -1;
    }

    pMount->Connect();
    delete pMount;

    return 0;
}

BOOL
Mount::GetMountInfo(_In_ PWSTR *pMountPoint,
                    _In_ PWSTR *pSharePoint,
                    _In_ PWSTR *pUsername,
                    _In_ PWSTR *pPwd)
{
    BOOL bRet = FALSE;

    assert(pMountPoint);
    assert(pSharePoint);
    assert(pUsername);
    assert(pPwd);

    if (pMountPoint != NULL)
    {
        std::wcout << L"Local name: ";
        *pMountPoint = NULL;
        if (GetInput(pMountPoint, FALSE) == FALSE)
        {
            goto Cleanup;
        }
    }

    if (pSharePoint != NULL)
    {
        std::wcout << L"Remote name: ";
        *pSharePoint = NULL;
        if (GetInput(pSharePoint, FALSE) == FALSE)
        {
            goto Cleanup;
        }
    }

    if (pUsername != NULL)
    {
        std::wcout << L"User name: ";
        *pUsername = NULL;
        if (GetInput(pUsername, FALSE) == FALSE)
        {
            goto Cleanup;
        }
    }

    if (pPwd != NULL)
    {
        std::wcout << L"Password: ";
        *pPwd = NULL;
        if (GetInput(pPwd, TRUE) == FALSE)
        {
            goto Cleanup;
        }
    }

    bRet = TRUE;

Cleanup:
    if (bRet == FALSE)
    {
        CleanupString(pMountPoint);
        CleanupString(pSharePoint);
        CleanupString(pUsername);
        CleanupString(pPwd);
    }

    return bRet;
}

BOOL Mount::Connect()
{
    DWORD dwResult;
    ULONG ulBufferSize = 64;
    ULONG ulNewBufferSize = 64;
    PWSTR pstrProvider = NULL;
    NETRESOURCE netResource = {0};
    NETINFOSTRUCT netInfo = {0};

    netResource.dwScope = RESOURCE_GLOBALNET;
    netResource.dwType = RESOURCETYPE_DISK;
    netResource.dwDisplayType = RESOURCEDISPLAYTYPE_SHARE;
    netResource.dwUsage = RESOURCEUSAGE_CONNECTABLE;
    netResource.lpLocalName = mMountPoint;
    netResource.lpRemoteName = mSharePoint;

    if ((mSharePoint == NULL) || (wcslen(mSharePoint) == 0))
    {
        std::wcout << L"No connection information has been provided\n";
        return FALSE;
    }

    std::wcout << L"Attempting to connect to: " << mSharePoint << L"...";
    dwResult = WNetAddConnection2(&netResource,
                                  mPassword,
                                  mUsername,
                                  CONNECT_UPDATE_PROFILE);
    if (dwResult != NO_ERROR)
    {
        std::wcout << L"\n*** Failed to connect to: " << mSharePoint <<
            L"-->" << mMountPoint << L" (" << dwResult << L")\n";

        return FALSE;
    }

    std::wcout << L"Connected!\n";
    mbConnect = TRUE;

    std::wcout << L"Checking network name provider...\n";
    pstrProvider = (PWSTR) AllocMemory(ulBufferSize);
    if (pstrProvider == NULL)
    {
        std::wcout << L"*** Failed to allocate " << ulBufferSize << L" bytes of memory\n";
        return FALSE;
    }

    dwResult = WNetGetProviderName(WNNC_NET_SMB,
                                   pstrProvider,
                                   &ulNewBufferSize);
    if (dwResult != NO_ERROR)
    {
        if (dwResult != ERROR_MORE_DATA)
        {
            std::wcout << L"*** Failed to get network name provider: " << dwResult << std::endl;
            goto Cleanup;
        }

        if (FALSE == ReallocMemory((PVOID *) &pstrProvider, ulBufferSize, ulNewBufferSize))
        {
            std::wcout << L"*** Failed to reallocate " << ulBufferSize << L"-->" << ulNewBufferSize << std::endl;
            goto Cleanup;
        }

        ulBufferSize = ulNewBufferSize;
        dwResult = WNetGetProviderName(WNNC_NET_SMB,
                                       pstrProvider,
                                       &ulBufferSize);
        if (dwResult != NO_ERROR)
        {
            std::wcout << L"*** Failed to get network name provider: " << dwResult << std::endl;
            goto Cleanup;
        }
    }

    std::wcout << L"Provider: " << pstrProvider << std::endl;

    netInfo.cbStructure = sizeof(netInfo);
    dwResult = WNetGetNetworkInformation(pstrProvider, &netInfo);
    if (dwResult != NO_ERROR)
    {
        std::wcout << L"*** Failed to get network info of " << pstrProvider << std::endl;
        goto Cleanup;
    }

    std::wcout << L"Version: " << netInfo.dwProviderVersion << std::endl;
    std::wcout << L"Status: " << netInfo.dwStatus;
    if (netInfo.dwStatus == NO_ERROR)
    {
        std::wcout << L" (No Error)\n";
    }
    else if (netInfo.dwStatus == ERROR_NO_NETWORK)
    {
        std::wcout << L" (No available network)\n";
    }
    else if (netInfo.dwStatus == ERROR_BUSY)
    {
        std::wcout << L" (Network is not currently able to service the requests)\n";
    }
    else
    {
        std::wcout << L" (Undefined error)\n";
    }
    std::wcout << L"Type: " << netInfo.wNetType << std::endl;

Cleanup:
    FreeMemory((PVOID *) &pstrProvider);

    return TRUE;
}

void Mount::Disconnect()
{
    PCWSTR pName = NULL;
    DWORD dwResult = 0;

    if ((mMountPoint != NULL) && (wcslen(mMountPoint) > 0))
    {
        pName = mMountPoint;
    }
    else if ((mSharePoint != NULL) && (wcslen(mSharePoint) > 0))
    {
        pName = mSharePoint;
    }
    else
    {
        std::wcout << L"No connection has to be disconnected\n";
    }

    std::wcout << L"Disconnecting " << mSharePoint << L"...";
    dwResult = WNetCancelConnection2(pName,
                                     CONNECT_UPDATE_PROFILE,
                                     TRUE);
    if (dwResult != NO_ERROR)
    {
        std::wcout << L"\n*** Failed to disconnet from " << mSharePoint << std::endl;
    }
    else
    {
        std::wcout << L"Disconnected\n";
    }
}
