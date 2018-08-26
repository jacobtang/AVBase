#include "wsemaphore.h"
#include "wtimefunction.h"

using namespace OPENVP;

WSemaphore::WSemaphore(int nInitialCount, int nMaxCount)
{

#if defined _OS_IOS 
    int rc = 0;
    rc = pthread_mutex_init(&m_mutex, NULL);
    rc = pthread_cond_init(&m_cond, NULL);
    m_nMax = nMaxCount;
    m_nCount = nInitialCount;
#else
    m_sem = new sem_t;
    sem_init(m_sem, 0, nInitialCount);
#endif

}

WSemaphore:: ~WSemaphore()
{

#if defined _OS_IOS 
    int rc = 0;
    rc = pthread_cond_destroy(&m_cond);
    rc = pthread_mutex_destroy(&m_mutex);
#else
    if (m_sem)
    {
        sem_destroy(m_sem);
        delete m_sem;
        m_sem = NULL;
    }
#endif

}

int WSemaphore::WaitSemaphore(int dwWaitTime)
{
    int dwRet = 0;

#if defined _OS_IOS
    int rc = 0;
    struct timespec abstime;

    if (dwWaitTime != INFINITE)
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        abstime.tv_sec = tv.tv_sec + dwWaitTime / 1000;
        abstime.tv_nsec = tv.tv_usec * 1000 + (dwWaitTime % 1000) * 1000000;
        if (abstime.tv_nsec >= 1000000000)
        {
            abstime.tv_sec += abstime.tv_nsec / 1000000000;
            abstime.tv_nsec %= 1000000000;
        }
    }
    rc = Lock();
    if (rc != 0)
        return WAIT_TIMEOUT;
    while (m_nCount < 1)
    {

        if (dwWaitTime == INFINITE)
        {

            if ((rc = pthread_cond_wait(&m_cond, &m_mutex)) != 0)
            {
                UnLock();
                return WAIT_TIMEOUT;
            }
        }
        else
        {
            do { rc = pthread_cond_timedwait(&m_cond, &m_mutex, &abstime); } while (rc == EINTR);
            if (rc != 0)
            {
                if (rc == ETIMEDOUT)
                {
                    break;
                }
                UnLock();
                return WAIT_TIMEOUT;
            }
        }
    }
    if (rc == 0) --m_nCount;
    UnLock();
    dwRet = rc == 0 ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
#else
    int nRet = 0;
    if (dwWaitTime == INFINITE)
    {
        while (((nRet = sem_wait(m_sem)) != 0) && (errno == EINTR));
    }
    else if (dwWaitTime == 0)
    {
        while (((nRet = sem_trywait(m_sem)) != 0) && (errno == EINTR));
    }
    else
    {
        struct timeval tt;
        gettimeofday(&tt, NULL);

        timespec ts;
        ts.tv_sec = tt.tv_sec + (dwWaitTime / 1000);
        ts.tv_nsec = tt.tv_usec * 1000 + (dwWaitTime % 1000) * 1000 * 1000;//这里可能造成纳秒>1000 000 000

        ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
        ts.tv_nsec %= (1000 * 1000 * 1000);

        while (((nRet = sem_timedwait(m_sem, &ts)) != 0) && (errno == EINTR));
    }
    dwRet = nRet == 0 ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
#endif
    return dwRet;
}

bool WSemaphore::ReleaseSemaphore(int lCount)
{
    bool bRet = true;

#if defined _OS_IOS 
    int rc = 0;
    while (lCount--)
    {

        rc = Lock();
        if (rc != 0)
            break;
        if (m_nCount >= m_nMax)
        {
            UnLock();
            break;
        }
        ++m_nCount;
        rc = pthread_cond_signal(&m_cond);
        UnLock();
    }
#else
    if (lCount > 0)
    {
        while (lCount--) sem_post(m_sem);
    }
#endif
    return bRet;
}

#if defined _OS_IOS 
int WSemaphore::Lock()
{
    int rc;
    do
    {
        rc = pthread_mutex_lock(&m_mutex);
    } while (rc == EINTR);
    return rc;
}

VOID WSemaphore::UnLock()
{
    int rc;
    do
    {
        rc = pthread_mutex_unlock(&m_mutex);
    } while (rc == EINTR);
}

#endif