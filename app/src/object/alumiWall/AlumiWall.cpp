#include "AlumiWall.h"
#include <config/ResourcePath.h>

AlumiWall::AlumiWall()
{
	objectType_ = ObjectType2d::kAlumiWall;
	isDynamic_ = true;
}

AlumiWall::~AlumiWall()
{
}

void AlumiWall::Initialize()
{
	objectType_ = ObjectType2d::kAlumiWall;
	isDynamic_ = true;

	pSprite_ = std::make_unique<Sprite>();
	pSprite_->Initialize(Path::Image::InGame::kTestTile);
	pSprite_->SetColor({ 0.7f, 0.7f, 0.8f, 1.0f }); // 仮置き：灰色/銀色
}

void AlumiWall::Update()
{
	BaseObject2d::Update();
}

void AlumiWall::Draw()
{
	BaseObject2d::Draw();
}

