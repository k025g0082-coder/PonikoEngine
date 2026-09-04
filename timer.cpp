#include "Timer.h"

float Timer::deltaTime_ = 0.0f;

void Timer::Initialize()
{
    QueryPerformanceFrequency(&frequency_);
    QueryPerformanceCounter(&previousTime_);
}

Timer* Timer::GetInstance() {
    static Timer instance;
    return &instance;
}

void Timer::UpdateDeltaTime()
{
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);

    deltaTime_ =
        static_cast<float>(
            currentTime.QuadPart - previousTime_.QuadPart
            ) / static_cast<float>(frequency_.QuadPart);

    previousTime_ = currentTime;
}