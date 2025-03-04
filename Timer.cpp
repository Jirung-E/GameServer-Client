#include "stdafx.h"
#include <timeapi.h>
#include "Timer.h"

#pragma comment(lib, "winmm.lib")


CGameTimer::CGameTimer() {
    if(::QueryPerformanceFrequency((LARGE_INTEGER*)&m_nPerformanceFrequency)) {
        m_bHardwareHasPerformanceCounter = true;
        ::QueryPerformanceCounter((LARGE_INTEGER*)&m_nLastTime);
        m_fTimeScale = 1.0f / m_nPerformanceFrequency;
    }
    else {
        m_bHardwareHasPerformanceCounter = false;
        m_nLastTime = ::timeGetTime();
        m_fTimeScale = 0.001f;
    }

    m_nSampleCount = 0;
    m_nCurrentFrameRate = 0;
    m_nFramePerSecond = 0;
    m_fFPSTimeElapsed = 0.0f;
}

CGameTimer::~CGameTimer() {

}

void CGameTimer::Start() {
}

void CGameTimer::Stop() {
}

void CGameTimer::Reset() {
    __int64 nPerformanceCounter;
    ::QueryPerformanceCounter((LARGE_INTEGER*)&nPerformanceCounter);
    
    m_nLastTime = nPerformanceCounter;
    m_nCurrentTime = nPerformanceCounter;

    m_bStopped = false;
}

void CGameTimer::Tick(float fLockFPS) {
    if(m_bHardwareHasPerformanceCounter) {
        ::QueryPerformanceCounter((LARGE_INTEGER*)&m_nCurrentTime);
    }
    else {
        m_nCurrentTime = ::timeGetTime();
    }

    float fTimeElapsed = (m_nCurrentTime - m_nLastTime) * m_fTimeScale;

    if(fLockFPS > 0.0f) {
        while(fTimeElapsed < 1.0f / fLockFPS) {
            if(m_bHardwareHasPerformanceCounter) {
                ::QueryPerformanceCounter((LARGE_INTEGER*)&m_nCurrentTime);
            }
            else {
                m_nCurrentTime = ::timeGetTime();
            }

            fTimeElapsed = (m_nCurrentTime - m_nLastTime) * m_fTimeScale;
        }
    }

    m_nLastTime = m_nCurrentTime;

    if(fabsf(fTimeElapsed - m_fTimeElapsed) < 1.0f) {
        ::memmove(&m_fFrameTime[1], m_fFrameTime, (MAX_SAMPLE_COUNT - 1) * sizeof(float));
        m_fFrameTime[0] = fTimeElapsed;
        if(m_nSampleCount < MAX_SAMPLE_COUNT) m_nSampleCount++;
    }

    m_nFramePerSecond++;
    m_fFPSTimeElapsed += fTimeElapsed;
    if(m_fFPSTimeElapsed > 1.0f) {
        m_nCurrentFrameRate = m_nFramePerSecond;
        m_nFramePerSecond = 0;
        m_fFPSTimeElapsed = 0.0f;
    }

    m_fTimeElapsed = 0.0f;
    for(ULONG i=0; i<m_nSampleCount; i++) {
        m_fTimeElapsed += m_fFrameTime[i];
    }
    if(m_nSampleCount > 0) {
        m_fTimeElapsed /= m_nSampleCount;
    }
}

unsigned long CGameTimer::GetFrameRate(LPTSTR lpszString, int nCharacters) {
    if(lpszString) {
        _itow_s(m_nCurrentFrameRate, lpszString, nCharacters, 10);
        wcscat_s(lpszString, nCharacters, _T(" FPS)"));
    }

    return m_nCurrentFrameRate;
}

float CGameTimer::GetTimeElapsed() {
    return m_fTimeElapsed;
}
