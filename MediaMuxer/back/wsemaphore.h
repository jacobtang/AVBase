#ifndef __WSEMAPHORE_H 
#define __WSEMAPHORE_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>

namespace OPENVP
{

class WSemaphore
{
public:
    WSemaphore(int nInitialCount, int nMaxCount);
    virtual ~WSemaphore();
public:
    int 	WaitSemaphore(int dwWaitTime);
    BOOL 	ReleaseSemaphore(int lCount);

private:
#if defined _OS_IOS
    volatile int    m_nCount;
    int             m_nMax;
    pthread_mutex_t m_mutex;
    pthread_cond_t  m_cond;
    int             Lock();
    void            UnLock();
#else
    sem_t* 	m_sem;
#endif

};

}
#endif
