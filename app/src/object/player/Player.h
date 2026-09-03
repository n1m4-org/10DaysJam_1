#pragma once
#include <object/baseObject2d/BaseObject2d.h>
#include <Features/Input/Input.h>
#include <logic/mapCollision/MapCollision.h>
#include <vector>
#include <memory>

class Player : public BaseObject2d
{
public:
	Player();
	~Player() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;

	// WASD入力と押し出し制御
	void HandleInput(Input* pInput, MapCollision& mapCollision,
	                 const std::vector<std::vector<int>>& currentMap,
	                 const std::vector<std::unique_ptr<BaseObject2d>>& objects);

private:
	void UpdateSpriteTextureBasedOnAngle();

	void SetSpriteTexture(size_t textureIndex);

	Vector2Int beforeAngle_ = { 0, -1 }; // 前回の角度を保持

	int anmationFrame_ = 0; // アニメーションフレームのカウンタ

	Vector2 originalSpriteSize_ = { 100.0f, 100.0f }; // 元のスプライトサイズ
};

