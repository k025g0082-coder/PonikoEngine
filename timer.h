#pragma once

#include <Windows.h>

class Timer {
public:

    static Timer* GetInstance();

    // 初期化
    void Initialize();
    
    // 呼ぶのはエンジンの更新につき一回のみ
    void UpdateDeltaTime();

    // デルタタイム取得
    static float GetDeltaTime() { return deltaTime_; }

private:
    LARGE_INTEGER frequency_{};
    LARGE_INTEGER previousTime_{};
    static float deltaTime_;
};