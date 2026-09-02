#include "Repeater.h"
#include <config/ResourcePath.h>

Repeater::Repeater()
{
	objectType_ = ObjectType2d::kRepeater;
	isDynamic_ = true;
}

Repeater::~Repeater()
{
}

void Repeater::Initialize()
{
	objectType_ = ObjectType2d::kRepeater;
	isDynamic_ = true;

	pSprite_ = std::make_unique<Sprite>();
	pSprite_->Initialize(Path::Image::InGame::kTestTile);
	pSprite_->SetColor({ 0.2f, 0.9f, 0.4f, 1.0f }); // 仮置き：緑色
}

void Repeater::Update()
{
	BaseObject2d::Update();
}

void Repeater::Draw()
{
	BaseObject2d::Draw();
}

