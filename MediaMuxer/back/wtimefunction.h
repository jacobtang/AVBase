#ifndef __WTIME_FUNCTION_H
#define __WTIME_FUNCTION_H

#include <sys/time.h>
#include <unistd.h>
#include <time.h>

namespace OPENVP
{

extern int GetTickCount();

extern int timeGetTime();

extern void	 Sleep(int millSecond);

#define _tctime	ctime


}

#endif
