#include "framework/Calms.h"

int _stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    auto pCalms = std::make_unique<Calms>();

    pCalms->Run();

    return 0;
}