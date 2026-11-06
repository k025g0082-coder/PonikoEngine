#pragma once

#include <cmath>
#include <algorithm>

enum class EasingType
{
    None,
    InSine,
    OutSine,
    InOutSine,
    InExpo,
    InOutExpo,
    InBack,
    OutBack,
    OutBounce,
    BouncyMountain,
    SmoothTrapezoid,
    Jump
};

class Easing
{
public:

    static float Get(float time, EasingType type)
    {
        // 通常のイージングは0～1に制限
        time = std::clamp(time, 0.0f, 1.0f);

        constexpr float PI = 3.14159265f;

        constexpr float c1 = 1.70158f;
        constexpr float c3 = c1 + 1.0f;

        constexpr float n1 = 7.5625f;
        constexpr float d1 = 2.75f;

        switch (type)
        {
        case EasingType::None:
            return time;

        case EasingType::InSine:
            return 1.0f - cosf((time * PI) / 2.0f);

        case EasingType::OutSine:
            return sinf((time * PI) / 2.0f);

        case EasingType::InOutSine:
            return -(cosf(PI * time) - 1.0f) / 2.0f;

        case EasingType::InExpo:
            if (time == 0.0f) return 0.0f;
            return powf(2.0f, 10.0f * time - 10.0f);

        case EasingType::InOutExpo:
            if (time == 0.0f) return 0.0f;
            if (time == 1.0f) return 1.0f;

            if (time < 0.5f)
                return powf(2.0f, 20.0f * time - 10.0f) / 2.0f;

            return (2.0f - powf(2.0f, -20.0f * time + 10.0f)) / 2.0f;

        case EasingType::InBack:
            return c3 * time * time * time - c1 * time * time;

        case EasingType::OutBack:
            return 1.0f + c3 * powf(time - 1.0f, 3.0f)
                + c1 * powf(time - 1.0f, 2.0f);

        case EasingType::OutBounce:
            if (time < 1.0f / d1)
                return n1 * time * time;

            if (time < 2.0f / d1)
            {
                float t = time - 1.5f / d1;
                return n1 * t * t + 0.75f;
            }

            if (time < 2.5f / d1)
            {
                float t = time - 2.25f / d1;
                return n1 * t * t + 0.9375f;
            }

            {
                float t = time - 2.625f / d1;
                return n1 * t * t + 0.984375f;
            }

        case EasingType::BouncyMountain:
            return sinf(time * PI)
                * (1.0f + 0.2f * sinf(time * 2.0f * PI));

        case EasingType::SmoothTrapezoid:
        {
            auto smoothstep = [](float edge0, float edge1, float x)
                {
                    float t = std::clamp(
                        (x - edge0) / (edge1 - edge0),
                        0.0f,
                        1.0f
                    );

                    return t * t * (3.0f - 2.0f * t);
                };

            float up = smoothstep(0.0f, 0.25f, time);
            float down = 1.0f - smoothstep(0.75f, 1.0f, time);

            return up * down;
        }

        case EasingType::Jump:
            return 4.0f * time * (1.0f - time);
        }

        return time;
    }
};