#include "EffectSequencer.h"


void EffectSequencer::Get(int index, int** start, int** end, int* type, unsigned int* color)
{
    SequenceItem& item = items_[index];
    if (color)
        *color = 0xFFAA8080; // same color for everyone, return color based on type
    if (start)
        *start = &item.mFrameStart;
    if (end)
        *end = &item.mFrameEnd;
    if (type)
        *type = item.mType;
}