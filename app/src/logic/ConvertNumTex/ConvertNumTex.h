#pragma once
#include <config/ResourcePath.h>
#include <object/ObjectRule.h>

const char* ConvertNumTex(int num)
{
	if (num < ObjectRule::kMapWallType || num > ObjectRule::kMapWallType + 100)
	{
		return Path::Image::InGame::kTestTile;
	}
	int wallTypeIndex = num - ObjectRule::kMapWallType;
	if (wallTypeIndex < 0 || wallTypeIndex >= static_cast<int>(Path::Image::InGame::WallType::WallTypeCount))
	{
		return Path::Image::InGame::kTestTile; // 範囲外の場合は"kTestTile"のテクスチャを返す
	}
	return Path::Image::InGame::kWalls[wallTypeIndex];
}
