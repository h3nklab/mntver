#pragma once

class Mount
{
public:
    Mount()
    {
        if (FALSE == GetMountInfo(&mMountPoint,
                                  &mSharePoint,
                                  &mUsername,
                                  &mPassword))
        {
            throw std::exception("Failed to get share point");
        }
    }

    ~Mount()
    {
        if (mbConnect == TRUE)
        {
            Disconnect();
        }

        FreeMemory((PVOID *) &mMountPoint);
        FreeMemory((PVOID *) &mSharePoint);
        FreeMemory((PVOID *) &mUsername);
        FreeMemory((PVOID *) &mPassword);
    }

    PCWSTR GetMountPoint()
    {
        return mMountPoint;
    }

    PCWSTR GetSharePoint()
    {
        return mSharePoint;
    }

    PCWSTR GetUsername()
    {
        return mUsername;
    }

    PCWSTR GetPassword()
    {
        return mPassword;
    }

    BOOL Connect();

    void Disconnect();

private:
    BOOL mbConnect = FALSE;

    PWSTR mMountPoint;
    PWSTR mSharePoint;
    PWSTR mUsername;
    PWSTR mPassword;

    BOOL GetMountInfo(_In_ PWSTR *pMountPoint,
                      _In_ PWSTR *pSharePoint,
                      _In_ PWSTR *pUsername,
                      _In_ PWSTR *pPwd);
};
