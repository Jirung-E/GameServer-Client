#pragma once


const ULONG MAX_SAMPLE_COUNT = 50;

class CGameTimer {
private:
    bool m_bHardwareHasPerformanceCounter;
    float m_fTimeScale;
    float m_fTimeElapsed;
    __int64 m_nCurrentTime;
    __int64 m_nLastTime;
    __int64 m_nPerformanceFrequency;

    float m_fFrameTime[MAX_SAMPLE_COUNT];
    ULONG m_nSampleCount;

    unsigned long m_nCurrentFrameRate;
    unsigned long m_nFramePerSecond;
    float m_fFPSTimeElapsed;

    bool m_bStopped;

public:
    CGameTimer();
    virtual ~CGameTimer();

public:
    void Start();
    void Stop();
    void Reset();
    void Tick(float fLockFPS = 0.0f);
    unsigned long GetFrameRate(LPTSTR lpszString = NULL, int nCharacters = 0);
    float GetTimeElapsed();
};