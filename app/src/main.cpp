#include "framework/ThirtySecBeforeDeadline.h"

int _stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    auto pCalms = std::make_unique<ThirtySecBeforeDeadline>();

    pCalms->Run();

    return 0;
}