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
	angle_ = Vector2Int{ 0, 1 };
	beforeAngle_ = angle_;

	pSprite_ = std::make_unique<Sprite>();
	originalSpriteSize_ = pSprite_->GetSize();
	SetSpriteTexture(static_cast<size_t>(Path::Image::PlayerTextureNames::front));
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
		if (mapCollision.TryMove(currentMap, objects, *this, moveDir))
		{
			anmationFrame_ == 0 ? anmationFrame_ = 1 : anmationFrame_ = 0; // アニメーションフレーム切り替え
			Vector2 spriteSize = pSprite_->GetSize();
			pSprite_->SetTextureLeftTop({ static_cast<float>(anmationFrame_) * 128.0f, 0.0f });
		}
	}

}

void Player::UpdateSpriteTextureBasedOnAngle()
{
	if(beforeAngle_ != angle_)
	{
		if (angle_ == Vector2Int{ 0, -1 }) // 上
		{
			SetSpriteTexture(static_cast<size_t>(Path::Image::PlayerTextureNames::back));
		}
		else if (angle_ == Vector2Int{ 0, 1 }) // 下
		{
			SetSpriteTexture(static_cast<size_t>(Path::Image::PlayerTextureNames::front));
		}
		else if (angle_ == Vector2Int{ -1, 0 }) // 左
		{
			SetSpriteTexture(static_cast<size_t>(Path::Image::PlayerTextureNames::left));
		}
		else if (angle_ == Vector2Int{ 1, 0 }) // 右
		{
			SetSpriteTexture(static_cast<size_t>(Path::Image::PlayerTextureNames::right));
		}
	}
}

void Player::SetSpriteTexture(size_t textureIndex)
{
	pSprite_->Initialize(Path::Image::kPlayerTextures[textureIndex]);
	Vector2 spriteSize = pSprite_->GetSize();
	pSprite_->SetTextureSize({ spriteSize.x / 2.0f, spriteSize.y });
	pSprite_->SetTextureLeftTop({ static_cast<float>(anmationFrame_) * spriteSize.x / 2.0f, 0.0f });
}

void Player::Update()
{
	BaseObject2d::Update();
	UpdateSpriteTextureBasedOnAngle();
	beforeAngle_ = angle_;
}

void Player::Draw()
{
	BaseObject2d::Draw();
}

