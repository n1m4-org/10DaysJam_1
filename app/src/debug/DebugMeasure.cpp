#include "DebugMeasure.h"
#include <common/ResourcePath.h>
#include <SpriteManager.h>

DebugMeasure::DebugMeasure()
{
    this->RegisterOnChange();
    this->InitializeSprite();
}

void DebugMeasure::InitializeSprite()
{
    pSprite_ = std::make_unique<Sprite>();
    pSprite_->Initialize(Path::Image::kDebugMeasure, {});
    if (enable_)
    {
        Hagine::SpriteManager::GetInstance()->RegisterExternal(pSprite_.get());
    }
}

void DebugMeasure::RegisterOnChange()
{
#ifdef _DEBUG
    debug_enable_.SetOnChange([this](const bool& flag)
    {
        if (flag)
        {
            Hagine::SpriteManager::GetInstance()->RegisterExternal(pSprite_.get());
        }
        else
        {
            Hagine::SpriteManager::GetInstance()->UnregisterExternal(pSprite_.get());
        }
    });
#endif // _DEBUG
}
