#include <iostream>
#include <cassert>
#include <Windows.h>

#include "utils.h"
#include "mntver.h"

#define STRING_BUFFER_MAX_SIZE  1024

PVOID AllocMemory(
    _Inout_ ULONG ulSize)
{
    PVOID pRet = NULL;

    if (ulSize > 0)
    {
        pRet = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ulSize);
    }

    return pRet;
}

BOOL ReallocMemory(_In_ PVOID *pCurrent,
                   _In_ ULONG ulOldSize,
                   _In_ ULONG ulNewSize)
{
    PVOID pRet = NULL;

    if (ulNewSize <= ulOldSize)
    {
        return FALSE;
    }

    if (pCurrent == NULL)
    {
        return FALSE;
    }

    pRet = AllocMemory(ulNewSize);
    if (pRet == NULL)
    {
        return FALSE;
    }

    CopyMemory(pRet, pCurrent, ulOldSize);
    FreeMemory(pCurrent);
    *pCurrent = pRet;

    return TRUE;
}

void FreeMemory(
    _Inout_ PVOID *pMem)
{
    assert(pMem);

    if (*pMem != NULL)
    {
        HeapFree(GetProcessHeap(), 0, *pMem);
        *pMem = NULL;
    }
}

void CleanupString(
    _Inout_ PWSTR *pString)
{
    assert(pString);

    if ((pString != NULL) && (*pString != NULL))
    {
        FreeMemory((PVOID *) pString);
    }
}

BOOL GetInput(
    _Inout_ PWSTR *pInput,
    _In_ BOOL bHidden)
{
    wint_t cInput = 0;
    WCHAR sInput[STRING_BUFFER_MAX_SIZE] = {0};
    DWORD dwCount = 0;

    assert(pInput);

    *pInput = NULL;
    while (dwCount < ((sizeof(sInput) / sizeof(WCHAR)) - 1))
    {
        cInput = _getwch();
        if (cInput == 0x3)
        {
            return FALSE;
        }

        if ((cInput == L'\r') || (cInput == L'\n') || (cInput == L'\t'))
        {
            putwchar(L'\n');
            break;
        }

        if (cInput == L'\b')
        {
            if (dwCount == 0)
            {
                continue;
            }


            dwCount--;
            sInput[dwCount] = L'\0';
            putwchar(L'\b');

            continue;
        }

        if (bHidden == TRUE)
        {
            putwchar(L'*');
        }
        else
        {
            putwchar(cInput);
        }
        sInput[dwCount] = cInput;
        dwCount++;
    }

    if (dwCount > 0)
    {
        sInput[dwCount] = L'\0';
        *pInput = (PWSTR) AllocMemory((dwCount + 1) * sizeof(WCHAR));
        if (*pInput == NULL)
        {
            return FALSE;
        }
        wcscpy_s(*pInput, dwCount + 1, sInput);
    }

    return TRUE;
}