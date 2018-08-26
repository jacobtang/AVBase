#include "wtimefunction.h"

namespace OPENVP
{

#if	defined _OS_IOS
#include <time.h>
#include <sys/time.h>
#include <mach/clock.h>
#include <mach/mach.h>
int GetTickCount()
{
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    return mts.tv_sec * 1000 + mts.tv_nsec / 1000000;
}
#else
int GetTickCount()
{
	struct timespec on; //NOLINT(legal/name)
	clock_gettime(CLOCK_MONOTONIC, &on);
	return on.tv_sec * 1000 + on.tv_nsec / 1000000;
}
#endif

int timeGetTime()
{
    return GetTickCount();
}

void Sleep(int illSecond)
{
    usleep(dwMillSecond * 1000);
}

#endif

}
