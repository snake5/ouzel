// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "CompileConfig.h"

#ifdef OUZEL_PLATFORM_OSX
#include <sys/time.h>
#endif

#if defined(OUZEL_PLATFORM_IOS) || defined(OUZEL_PLATFORM_TVOS)
#include <sys/syslog.h>
#endif

#ifdef OUZEL_PLATFORM_WINDOWS
#include <windows.h>
#include <strsafe.h>
#endif

#include "Utils.h"

namespace ouzel
{
    char TEMP_BUFFER[65536];

    void log(const char* format, ...)
    {
        char strBuffer[256];

        va_list list;
        va_start(list, format);
        
        vsprintf(strBuffer, format, list);
        
        va_end(list);
        
#if defined(OUZEL_PLATFORM_OSX)
        printf("%s\n", TEMP_BUFFER);
#elif defined(OUZEL_PLATFORM_IOS) || defined(OUZEL_PLATFORM_TVOS)
        syslog(LOG_WARNING, "log string");
#elif defined(OUZEL_PLATFORM_WINDOWS)
        wchar_t szBuffer[256];
        MultiByteToWideChar(CP_ACP, 0, strBuffer, -1, szBuffer, 256);
        StringCchCat(szBuffer, sizeof(szBuffer), L"\n");
        OutputDebugString(szBuffer);
#endif
    }
    
    uint64_t getCurrentMicroSeconds()
    {
#if defined(OUZEL_PLATFORM_OSX)
        struct timeval currentTime;
        
        gettimeofday(&currentTime, NULL);
        return currentTime.tv_sec * 1000000L + currentTime.tv_usec;
#elif defined(OUZEL_PLATFORM_WINDOWS)
        LARGE_INTEGER li;
        if (!QueryPerformanceFrequency(&li))
        {
            log("Failed to query frequency");
            return 0;
        }
        uint64_t frequency = li.QuadPart / 1000000L;
        
        QueryPerformanceCounter(&li);
        
        return li.QuadPart / frequency;
#else
        return 0;
#endif
    }
}
