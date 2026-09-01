#pragma once
#include <config/ResourcePath.h>

const char* ConvertNumTex(int num)
{
	if (num < 100 || num > 200)
	{
		return Path::Image::InGame::kTestTile; // 100未満または200以上の場合は"kTestTile"のテクスチャを返す
	}
	int wallTypeIndex = num - 100; // 100を引いて0から始まるインデックスに変換
	if (wallTypeIndex < 0 || wallTypeIndex >= static_cast<int>(Path::Image::InGame::WallType::WallTypeCount))
	{
		return Path::Image::InGame::kTestTile; // 範囲外の場合は"kTestTile"のテクスチャを返す
	}
	return Path::Image::InGame::kWalls[wallTypeIndex];
}
