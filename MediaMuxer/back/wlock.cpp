// WLock.cpp: implementation of the WLock class.
//
//////////////////////////////////////////////////////////////////////
#include "wlock.h"

using namespace OPENVP;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WLock::WLock()
{
    pthread_mutexattr_init( &m_attr );
    pthread_mutexattr_settype( &m_attr,PTHREAD_MUTEX_RECURSIVE );
    int rc = pthread_mutex_init( &m_Lock,&m_attr );
}

WLock::~WLock()
{
    int rc = 0;
    rc = pthread_mutex_destroy( &m_Lock );
    pthread_mutexattr_destroy(&m_attr);
}

void WLock::Lock()
{
    int rc;
    do {
        rc = pthread_mutex_lock( &m_Lock );
    } while ( rc== EINTR );
}

void WLock::UnLock()
{
    int rc;
    do {
        rc = pthread_mutex_unlock( &m_Lock );
    } while ( rc== EINTR );
}

bool WLock::TryLock()
{
    int rc;
    do {
        rc = pthread_mutex_trylock( &m_Lock );
    } while ( rc== EINTR );
    bool bResult = ( rc == 0 );
    return bResult;
}


WAutoLock::WAutoLock(WLock* pLock) :m_pLock(pLock)
{
    if (m_pLock)
        m_pLock->Lock();
}

WAutoLock::~WAutoLock()
{
    if (m_pLock)
        m_pLock->UnLock();
}
