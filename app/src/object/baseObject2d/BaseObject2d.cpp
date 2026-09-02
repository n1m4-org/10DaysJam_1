#include "BaseObject2d.h"
#include <algorithm>

BaseObject2d::BaseObject2d()
{
}

BaseObject2d::~BaseObject2d()
{
}

void BaseObject2d::SetPosition(const Vector2Int& position)
{
	position_ = position;
	targetPosition_ = position;
	isMoving_ = false;
	moveProgress_ = 0.0f;
}

void BaseObject2d::Move(const Vector2Int& delta)
{
	position_ = position_ + delta;
	targetPosition_ = position_;
	isMoving_ = true;
	moveProgress_ = 0.0f;
}


void BaseObject2d::UpdateSpritePosition(float tileSize, const Vector2& mapOffset)
{
	Vector2 calcPos = {
		tileSize / 2.0f + tileSize * position_.x + mapOffset.x,
		tileSize / 2.0f + tileSize * position_.y + mapOffset.y
	};

	if (!isMoving_)
	{
		currentRenderPos_ = calcPos;
		targetRenderPos_ = calcPos;
	}
	else
	{
		// 移動開始時のスタート位置から新しい目標位置へ
		targetRenderPos_ = calcPos;
	}

	if (pSprite_)
	{
		pSprite_->SetAnchorPoint({ 0.5f, 0.5f });
		pSprite_->SetPosition(currentRenderPos_);
		pSprite_->SetSize(size_);
		pSprite_->SetRotation(GetRotationAngleRad());
	}


}

void BaseObject2d::Update()
{
	if (isMoving_)
	{
		// 描画位置を目標値に近付ける（イージング / 線形補間）
		float deltaProgress = 0.15f; // スムーズ移動の補間係数
		currentRenderPos_.x += (targetRenderPos_.x - currentRenderPos_.x) * deltaProgress;
		currentRenderPos_.y += (targetRenderPos_.y - currentRenderPos_.y) * deltaProgress;

		float distSq = (targetRenderPos_.x - currentRenderPos_.x) * (targetRenderPos_.x - currentRenderPos_.x) +
		               (targetRenderPos_.y - currentRenderPos_.y) * (targetRenderPos_.y - currentRenderPos_.y);

		if (distSq < 0.1f)
		{
			currentRenderPos_ = targetRenderPos_;
			isMoving_ = false;
		}

		if (pSprite_)
		{
			pSprite_->SetPosition(currentRenderPos_);
		}
	}

	if (pSprite_)
	{
		pSprite_->Update();
	}
}

void BaseObject2d::Draw()
{
	if (pSprite_)
	{
		pSprite_->Draw1F();
	}
}
