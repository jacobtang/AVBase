// WLock.h: interface for the WLock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_WLOCK_H_)
#define _WLOCK_H_

#include <pthread.h>
#include <errno.h>


namespace OPENVP
{

class WLock
{
public:
    WLock();
    virtual ~WLock();
public:
    void UnLock();
    void Lock();
    bool TryLock();
private:
    pthread_mutexattr_t         m_attr;
    mutable pthread_mutex_t		m_Lock;

};

class WAutoLock
{
public:
    WAutoLock(WLock* pLock);
    virtual ~WAutoLock();

private:
    WLock*  m_pLock;
};
}
#endif // !defined(_WLOCK_H_)
