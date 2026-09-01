#pragma once
#include <debug/GameParameter.h>
#include <Sprite.h>

class DebugMeasure
{
public:
    /// エイリアス
    using Sprite = Hagine::Sprite;

    // ctor
    DebugMeasure();

private:
    void InitializeSprite();
    void RegisterOnChange();

    std::unique_ptr<Sprite> pSprite_ = nullptr;

    EnableDebug("Debug Measure");
    GameParameter(bool, enable_, false);
};