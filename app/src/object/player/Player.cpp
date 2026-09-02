#include "Player.h"
#include <config/ResourcePath.h>
#include <dinput.h>

Player::Player()
{
	objectType_ = ObjectType2d::kPlayer;
	isDynamic_ = true;
}

Player::~Player()
{
}

void Player::Initialize()
{
	objectType_ = ObjectType2d::kPlayer;
	isDynamic_ = true;

	pSprite_ = std::make_unique<Sprite>();
	pSprite_->Initialize(Path::Image::InGame::kTestTile);
	pSprite_->SetColor({ 0.2f, 0.6f, 1.0f, 1.0f }); // 仮置き：青色
}

void Player::HandleInput(Input* pInput, MapCollision& mapCollision,
                         const std::vector<std::vector<int>>& currentMap,
                         const std::vector<std::unique_ptr<BaseObject2d>>& objects)
{
	if (!pInput || isMoving_)
	{
		return;
	}

	Vector2Int moveDir = { 0, 0 };

	// W キー (上)
	if (pInput->TriggerKey(DIK_W) || pInput->TriggerKeyC('W') || pInput->TriggerKeyC('w'))
	{
		moveDir = { 0, -1 };
	}
	// S キー (下)
	else if (pInput->TriggerKey(DIK_S) || pInput->TriggerKeyC('S') || pInput->TriggerKeyC('s'))
	{
		moveDir = { 0, 1 };
	}
	// A キー (左)
	else if (pInput->TriggerKey(DIK_A) || pInput->TriggerKeyC('A') || pInput->TriggerKeyC('a'))
	{
		moveDir = { -1, 0 };
	}
	// D キー (右)
	else if (pInput->TriggerKey(DIK_D) || pInput->TriggerKeyC('D') || pInput->TriggerKeyC('d'))
	{
		moveDir = { 1, 0 };
	}

	if (moveDir.x != 0 || moveDir.y != 0)
	{
		angle_ = moveDir;
		mapCollision.TryMove(currentMap, objects, *this, moveDir);
	}

}

void Player::Update()
{
	BaseObject2d::Update();
}

void Player::Draw()
{
	BaseObject2d::Draw();
}

