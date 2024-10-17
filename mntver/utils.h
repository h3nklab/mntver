#pragma once

#define BUFFER_BLOCK_SIZE   128

#if 0
template<typename T>
class AutoClean
{
    AutoClean(const T &method)
    {
        mMethod = method;
    }

    ~AutoClean()
    {
        mMethod();
    }

private:
    const T &mMethod;
};
#endif

extern "C"
{
    void CleanupString(_Inout_ PWSTR *pString);

    PVOID AllocMemory(_Inout_ ULONG ulSize);

    BOOL ReallocMemory(_In_ PVOID *pCurrent,
                       _In_ ULONG ulOldSize,
                       _In_ ULONG ulNewSize);

    void FreeMemory(_Inout_ PVOID *pMem);

    BOOL GetInput(_Inout_ PWSTR *pInput,
                  _In_ BOOL bHidden);
}
