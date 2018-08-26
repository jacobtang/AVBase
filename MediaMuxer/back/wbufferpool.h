// 
//
//////////////////////////////////////////////////////////////////////

#if !defined(_WBUFFERPOOL_H_)
#define _WBUFFERPOOL_H_

#include "wlock.h"
#include "wsemaphore.h"
#include "wtimefunction.h"
#include <list>

#define INFINITE 0xFFFFFFFF
#define WAIT_TIMEOUT 0x00000102L
#define WAIT_OBJECT_0 0x00000000L

namespace OPENVP
{

class WFlexBuffer
{
public:
    WFlexBuffer(int unSize = 0) :
        m_unSize(0),
        m_unDataLen(0),
        m_pbBuffer(NULL)
    {
        m_unSize = unSize;
        if (unSize > 0)
            m_pbBuffer = new unsigned char[unSize];
    };
    virtual ~WFlexBuffer()
    {
        if (m_pbBuffer)
        {
            delete[] m_pbBuffer;
            m_pbBuffer = NULL;
        }
    };

public:
    virtual unsigned char*		GetPtr() { return m_pbBuffer; };
    virtual unsigned char*		GetPtr(int unSize) { CheckSize(unSize); return m_pbBuffer; };
    virtual bool		WriteData(unsigned char* pbData, int unDataLen)
    {
        if (unDataLen > m_unSize)
        {
            if (!CheckSize(unDataLen))
                return false;
        }
        memcpy(m_pbBuffer, pbData, unDataLen);
        m_unDataLen = unDataLen;
        return true;
    };

    virtual void        SetDataLen(int unDataLen) { m_unDataLen = unDataLen; };
    virtual int		GetDataLen() { return m_unDataLen; };

    virtual int        GetSize() { return m_unSize; };
protected:
    virtual bool		CheckSize(int unSize)
    {
        bool bRet = true;
        if (unSize > m_unSize)
        {

            delete[] m_pbBuffer;
            m_unSize = unSize;
            if (unSize > 0)
            {
                m_pbBuffer = new unsigned char[unSize];
                if (!m_pbBuffer)
                {
                    bRet = false;
                    m_unSize = 0;
                }
            }
        }
        return bRet;
    };
    int		m_unSize;
    int		m_unDataLen;
    unsigned char*		m_pbBuffer;
};

template<class T>
class WPoolTemplate
{
    typedef std::list< T* > BufferList;
public:

    enum { BUFFER_CLEAR_DIR_BEGIN = 0, BUFFER_CLEAR_DIR_END };

    WPoolTemplate(int unCount, int unSize = 0)
        :m_bStop(false),
        m_unBufferCount(0),
        m_unBufferSize(0),
        m_semBusy(0, unCount),
        m_semFree(unCount, unCount)
    {
        m_unBufferCount = unCount;
        m_unBufferSize = unSize;

        if (unCount > 0)
        {
            for (int u = 0; u < unCount; u++)
            {
                T *pBuffer = new T(unSize);
                if (pBuffer)
                {
                    m_lsTotal.push_back(pBuffer);
                    m_lsFree.push_back(pBuffer);
                }
            }
        }
    };

    ~WPoolTemplate()
    {
        SetStop();

        m_csBusy.Lock();
        m_lsBusy.clear();
        m_csBusy.UnLock();

        m_csFree.Lock();
        m_lsFree.clear();
        m_csFree.UnLock();

        while (m_lsTotal.size() > 0)
        {
            T *pBuffer = m_lsTotal.front();
            if (pBuffer)
                delete pBuffer;
            m_lsTotal.pop_front();
        }
    };

    int GetBufferCount() { return m_unBufferCount; };
    int GetBufferSize() { return m_unBufferSize; };

    int GetBufferFreeCount()
    {
        m_csFree.Lock();
        int unCount = m_lsFree.size();
        m_csFree.UnLock();

        return unCount;
    };
    int GetBufferBusyCount()
    {
        m_csBusy.Lock();
        int unCount = m_lsBusy.size();
        m_csBusy.UnLock();

        return unCount;
    };

    int GetBufferStatus()
    {
        if (m_unBufferCount < 1)
            return 0;

        int unCount = GetBufferBusyCount();

        return unCount * 100 / m_unBufferCount;
    };


    T * GetFreeBuffer(int dwWaitMiniSecond = INFINITE)
    {
        WaitSemphoreOrStop(m_semFree, dwWaitMiniSecond);

        T *pBuffer = NULL;
        m_csFree.Lock();
        pBuffer = m_lsFree.front();
        m_lsFree.pop_front();
        m_csFree.UnLock();

        return pBuffer;
    };

    T * PeekBusyBuffer(int dwWaitMiniSecond = INFINITE)
    {

        WaitSemphoreOrStop(m_semBusy, dwWaitMiniSecond);
	

        T *pBuffer = NULL;
        m_csBusy.Lock();
        pBuffer = m_lsBusy.front();
        m_csBusy.UnLock();
        return pBuffer;
    };

    T * GetBusyBuffer(int dwWaitMiniSecond = INFINITE)
    {

        WaitSemphoreOrStop(m_semBusy, dwWaitMiniSecond);			
        T *pBuffer = NULL;
        m_csBusy.Lock();
        pBuffer = m_lsBusy.front();
        m_lsBusy.pop_front();
        m_csBusy.UnLock();

        return pBuffer;
    };

    void AddFreeBuffer(T *pBuffer)
    {
        if (!pBuffer)
            return;

        m_csFree.Lock();
        m_lsFree.push_back(pBuffer);
        m_csFree.UnLock();

        m_semFree.ReleaseSemaphore(1L);
    };

    void AddBusyBuffer(T *pBuffer)
    {
        if (!pBuffer)
            return;

        m_csBusy.Lock();
        m_lsBusy.push_back(pBuffer);
        m_csBusy.UnLock();

        m_semBusy.ReleaseSemaphore(1L);
    };

    bool InsertBusyBuffer(T *pBuffer)
    {
        int 	unTemp;
        T*	pTempBuffer;
        int   	unSeqNum;
        bool	bRet = true;

#ifdef _STLP_LIST
        std::priv::_List_iterator<T*, std::_Nonconst_traits<T*> > it;
#else
#if defined _OS_IOS
        typename BufferList::iterator it;
#else
        std::_List_iterator<T*> it;
#endif
#endif			


        if (!pBuffer)
            return false;

        unSeqNum = pBuffer->GetSeqNum();

        bool  bInvalid = true;

        m_csBusy.Lock();

        for (it = m_lsBusy.begin(); it != m_lsBusy.end(); it++)
        {

            pTempBuffer = *it;
            if (pTempBuffer)
            {

                unTemp = pTempBuffer->GetSeqNum();
                if (unTemp == unSeqNum)
                {
                    bInvalid = false;
                    break;
                }

                if ((unSeqNum < unTemp && unTemp - unSeqNum < m_unBufferCount * 2) ||
                    (unSeqNum > unTemp && unSeqNum - unTemp > m_unBufferCount * 2))
                    break;
            }
        }

        if (bInvalid)
        {
            if (it != m_lsBusy.end())
                m_lsBusy.insert(it, pBuffer);
            else
                m_lsBusy.push_back(pBuffer);
            m_semBusy.ReleaseSemaphore(1L);

            bRet = true;
        }
        else
        {
            m_lsFree.push_back(pBuffer);
            m_semFree.ReleaseSemaphore(1L);

            bRet = false;
        }
        m_csBusy.UnLock();

        return bRet;
    };

    void SetStop()
    {
        m_bStop = true;
    };
    bool  IsStoped()
    {
        return m_bStop;
    };
    int WaitStop(int dwMilliSeconds)
    {
	
        if (m_bStop)
            return 0;
        const int kStandardTime = 10;
        int dwPassedTime = 0;
        bool bWaitInfinite = dwMilliSeconds == INFINITE;
        int dwStartTime = GetTickCount();
        while (bWaitInfinite || dwPassedTime <= dwMilliSeconds)
        {
            int dwAvailTime = dwMilliSeconds - dwPassedTime;
            int dwWaitTime = dwAvailTime < kStandardTime ? dwAvailTime : kStandardTime;
            Sleep(dwWaitTime);
            if (m_bStop)
            {
                return 0;
            }
            else if (!bWaitInfinite)
            {
                dwPassedTime = GetTickCount() - dwStartTime;
            }
        }
        if (m_bStop)
            return 0;
        return WAIT_TIMEOUT;
    };

    void ResetStop()
    {
        m_bStop = false;
    };

    void FreeLock() { m_csFree.Lock(); };
    void FreeUnLock() { m_csFree.UnLock(); };
    void BusyLock() { m_csBusy.Lock(); };
    void BusyUnLock() { m_csBusy.UnLock(); };

    void ClearBusyBuffer(int unCount, unsigned char bDir = BUFFER_CLEAR_DIR_BEGIN)
    {
        m_csBusy.Lock();
        m_csFree.Lock();

        int dwRet = 0;
        T  *pBuffer;

        if (bDir == BUFFER_CLEAR_DIR_BEGIN)
        {

            do
            {
                dwRet = m_semBusy.WaitSemaphore(0);
                if (dwRet != WAIT_OBJECT_0)
                    break;


                pBuffer = m_lsBusy.front();
                m_lsBusy.pop_front();
                m_lsFree.push_back(pBuffer);

                m_semFree.ReleaseSemaphore(1L);

            } while (--unCount && !m_bStop);
        }

        if (bDir == BUFFER_CLEAR_DIR_END)
        {

            do
            {

                dwRet = m_semBusy.WaitSemaphore(0);
                if (dwRet != WAIT_OBJECT_0)
                    break;

                pBuffer = m_lsBusy.back();
                m_lsBusy.pop_back();
                m_lsFree.push_back(pBuffer);

                m_semFree.ReleaseSemaphore(1L);

            } while (--unCount && !m_bStop);
        }

        m_csFree.UnLock();
        m_csBusy.UnLock();
    };

protected:
    volatile bool	m_bStop;

    int		m_unBufferCount;
    int		m_unBufferSize;

    WLock		m_csFree;
    WLock		m_csBusy;

    BufferList	m_lsBusy;
    BufferList	m_lsFree;
    BufferList  m_lsTotal;

    WSemaphore 	m_semBusy;
    WSemaphore	m_semFree;
};

typedef WPoolTemplate<WFlexBuffer> WFlexBufferPool;
}

#endif // !defined(_WBUFFERPOOL_H_)
