/*
Copyright (c) 2013 Khaled Mammou - Advanced Micro Devices, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#pragma once
#ifndef O3DGC_TIMER_H
#define O3DGC_TIMER_H

#include "o3dgcCommon.h"

#ifdef _WIN32
/* Thank you, Microsoft, for file WinDef.h with min/max redefinition. */
#define NOMINMAX
#include <windows.h>
#elif __APPLE__
#include <mach/clock.h>
#include <mach/mach.h>
#else
#include <time.h>
#include <sys/time.h>
#endif



namespace o3dgc
{
#ifdef _WIN32
    class Timer
    {
    public: 
        Timer(void)
        {
            m_start.QuadPart = 0;
            m_stop.QuadPart  = 0;
            QueryPerformanceFrequency( &m_freq ) ;
        };
        ~Timer(void){};
        void Tic() 
        {
            QueryPerformanceCounter(&m_start) ;
        }
        void Toc() 
        {
            QueryPerformanceCounter(&m_stop);
        }
        double GetElapsedTime() // in ms
        {
            LARGE_INTEGER delta;
            delta.QuadPart = m_stop.QuadPart - m_start.QuadPart;
            return (1000.0 * delta.QuadPart) / (double)m_freq.QuadPart;
        }
    private:
        LARGE_INTEGER m_start;
        LARGE_INTEGER m_stop;
        LARGE_INTEGER m_freq;

    };
#elif __APPLE__
    class Timer
    {
    public: 
        Timer(void)
        {
            memset(this, 0, sizeof(Timer));
            host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, & m_cclock);
        };
        ~Timer(void)
        {
            mach_port_deallocate(mach_task_self(),  m_cclock);
        };
        void Tic() 
        {
            clock_get_time( m_cclock, &m_start);
        }
        void Toc() 
        {
            clock_get_time( m_cclock, &m_stop);
        }
        double GetElapsedTime() // in ms
        {
            return 1000.0 * (m_stop.tv_sec - m_start.tv_sec + (1.0E-9) * (m_stop.tv_nsec - m_start.tv_nsec));
        }
    private:
        clock_serv_t    m_cclock;
        mach_timespec_t m_start;
        mach_timespec_t m_stop;
    };
#else
    class Timer
    {
    public: 
        Timer(void)
        {
            memset(this, 0, sizeof(Timer));
        };
        ~Timer(void){};
        void Tic() 
        {
            clock_gettime(CLOCK_REALTIME, &m_start);
        }
        void Toc() 
        {
            clock_gettime(CLOCK_REALTIME, &m_stop);
        }
        double GetElapsedTime() // in ms
        {
            return 1000.0 * (m_stop.tv_sec - m_start.tv_sec + (1.0E-9) * (m_stop.tv_nsec - m_start.tv_nsec));
        }
    private:
         struct timespec m_start;
         struct timespec m_stop;
    };
#endif

}
#endif // O3DGC_TIMER_H
