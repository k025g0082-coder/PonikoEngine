#include "Easing.h"

/*
    一覧
*/
const char* Easing::easingTags_[(int)EasingType::count] = {
      "None",
      "InSine",
      "OutSine",
      "InOutSine",
      "InExpo",
      "InOutExpo",
      "InBack",
      "OutBack",
      "OutBounce",
      "BouncyMountain",
      "SmoothTrapezoid",
      "Jump"
};