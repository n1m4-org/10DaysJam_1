#include "framework/Game.h"

int _stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    auto pCalms = std::make_unique<Game>();

    pCalms->Run();

    return 0;
}