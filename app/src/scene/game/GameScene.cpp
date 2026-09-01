#include "GameScene.h"
#include <Effects/SceneTransition/TransShutter.h>
#include <drawable/sprite/SpriteSystem.h>
#include <drawable/object3d/Object3dSystem.h>
#include <drawable/line/LineSystem.h>
#include <any>
#include <Core/DirectX12/TextureManager.h>
#include <config/ResourcePath.h>
#include <Color.h>
#include <cmath>
#include <Features/Audio/AudioManager.h>
#include <Features/Layer/CanvasScope.h>
#include <Math/ViewportUnits.hpp>
#include <NiGui.h>
#include <Math/Easing.h>
#include <Xinput.h>

#include <logic/ConvertNumTex/ConvertNumTex.h>

void GameScene::Initialize()
{
    /// インスタンスの取得
    pInput_ = Input::GetInstance();
    pSceneManager_ = SceneManager::GetInstance();
    pCubemapSystem_ = std::any_cast<CubemapSystem*>(pArgs_->Get("CubemapSystem"));
    pDx12_ = std::any_cast<DirectX12*>(pArgs_->Get("DirectX12"));
    pInputMapperUI_ = std::any_cast<InputMapper<InputActionUI>*>(pArgs_->Get("InputMapperUI"));

    /// Canvasの初期化
    {
        Canvas::Params params = {};
        params.name = "GameCanvas";
        params.pDx12 = pDx12_;
        params.pCubemapSystem = pCubemapSystem_;
#ifdef _DEBUG
        params.pImGuiManager = std::any_cast<ImGuiManager*>(pArgs_->Get("ImGuiManager"));
#endif // _DEBUG

        pCanvasBack_ = std::make_unique<Canvas>();
        pCanvasBack_->Initialize(params);

        params.name = "GameCanvas2";
        pCanvasSprite_ = std::make_unique<Canvas>();
        pCanvasSprite_->Initialize(params);

        pLayer_->AddCanvas(pCanvasBack_.get());
        pLayer_->AddCanvas(pCanvasSprite_.get());
    }

    // ゲームアイの初期化
    this->InitializeGameEye();

    // スカイボックスの初期化
    this->InitializeSkybox();

    mapData_.resize(mapHeight_);
    for (int y = 0; y < mapHeight_; ++y)
    {
        mapData_[y].resize(mapWidth_);
        for (int x = 0; x < mapWidth_; ++x)
        {
            mapData_[y][x] = 0;
        }
    }

    // スプライトの初期化
    this->InitializeSprites();
}

void GameScene::Finalize()
{
    gameEye_.reset();
    pLayer_->RemoveCanvas(pCanvasBack_.get());
    pLayer_->RemoveCanvas(pCanvasSprite_.get());
    pCanvasBack_->Finalize();
    pCanvasSprite_->Finalize();
}

void GameScene::Update()
{
	for (auto& tileRow : pSpriteTile_)
	{
		for (auto& tile : tileRow)
		{
			tile->Update();
		}
	}
	this->MapEdit();
}

void GameScene::Draw()
{
    CanvasScope canvasScopeBack(pCanvasSprite_.get());
    for (auto& tileRow : pSpriteTile_)
    {
        for (auto& tile : tileRow)
        {
            tile->Draw1F();
        }
    }
}

void GameScene::InitializeGameEye()
{
    /// ゲームアイの初期化
    gameEye_ = std::make_unique<GameEye>();
    gameEye_->SetName("gameEye");
    gameEye_->SetTranslate(Vector3(0, 15.0f, -30.0f));
    gameEye_->SetRotate(Vector3(-1.2f, 0, 0));
    gameEye_->SetFov(1.2f);

    /// ゲームアイをセット
    Object3dSystem::GetInstance()->SetGlobalEye(gameEye_.get());
    SpriteSystem::GetInstance()->SetGlobalEye(gameEye_.get());
    LineSystem::GetInstance()->SetGlobalEye(gameEye_.get());
    pCubemapSystem_->SetGlobalEye(gameEye_.get());
}

void GameScene::InitializeSprites()
{
    TextureManager* tm = TextureManager::GetInstance();
    tm->LoadTexture(Path::Image::InGame::kTestTile);

    pSpriteTile_.resize(mapHeight_);
    for (int y = 0; y < mapHeight_; ++y)
    {
        pSpriteTile_[y].resize(mapWidth_);
        for (int x = 0; x < mapWidth_; ++x)
        {
            pSpriteTile_[y][x] = std::make_unique<Sprite>();
            UpdateTileSprite(x, y);
        } 
    }
}

void GameScene::InitializeSkybox()
{
    auto pTM = TextureManager::GetInstance();
    pTM->LoadTexture(Path::Image::kTitleSkybox);

    pSkybox_ = std::make_unique<Skybox>();
    pSkybox_->Initialize(pCubemapSystem_);
    pSkybox_->SetSkyboxTexture(pTM->GetSrvHandleGPU(Path::Image::kTitleSkybox));

    pCanvasBack_->RegisterDrawable(pSkybox_.get());
}

void GameScene::MapEdit()
{
#ifdef _DEBUG
    ImGui::Begin("Map Editor");

    // --- 1. 壁の種類を定義した文字列配列 ---
    static const char* kWallTypeNames[] = {
		"All", "LB", "LBT", "LRB", "LRT", "LT", "RB", "RBT", "RL", "RT", "TB", "B", "L", "R", "T"
    };

    // 選択中のタイルタイプ（パレット用）
    static int currentSelectType = 0; // 0: SimpleTile, 100~110: WallTypes

    // --- 2. パレット選択 UI ---
    ImGui::Text("Palette");
    if (ImGui::RadioButton("Simple Tile", currentSelectType == 0))
    {
        currentSelectType = 0;
    }
    ImGui::SameLine();

    // enum class WallType の選択用 Combo
    static int selectedWallIdx = 0;
    if (ImGui::Combo("Wall Type", &selectedWallIdx, kWallTypeNames, IM_ARRAYSIZE(kWallTypeNames)))
    {
        currentSelectType = 100 + selectedWallIdx; // 100〜 の ID に変換
    }

    ImGui::Separator();
    ImGui::Text("Map Grid (Click to paint)");

    // --- 3. グリッド配置 UI (ペイント操作) ---
    // ボタンの見た目をコンパクトにしてマップ状に並べる
    for (int y = 0; y < mapHeight_; ++y)
    {
        for (int x = 0; x < mapWidth_; ++x)
        {
            ImGui::PushID(y * mapWidth_ + x);

            // 現在のタイルIDに応じたラベル表示 (例: "T" や "W0", "W1" など)
            std::string label = (mapData_[y][x] == 0) ? " ." : std::to_string(mapData_[y][x] - 100);

            // 20x20 の正方形ボタンとして配置
            if (ImGui::Button(label.c_str(), ImVec2(24, 24)))
            {
                // クリックされたら選択中のパレット ID を書き込み、即座にスプライトを更新
                mapData_[y][x] = currentSelectType;
                UpdateTileSprite(x, y);
            }

            // ホバー中にドラッグ描画したい場合は以下を追加
            if (ImGui::IsItemActive() && ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()))
            {
                if (mapData_[y][x] != currentSelectType)
                {
                    mapData_[y][x] = currentSelectType;
                    UpdateTileSprite(x, y);
                }
            }

            ImGui::PopID();

            if (x < mapWidth_ - 1)
            {
                ImGui::SameLine();
            }
        }
    }

    ImGui::Separator();
	ImGui::DragFloat2("Map Offset", &mapOffset_.x, 1.0f, -1000.0f, 1000.0f);
    ImGui::SameLine();
	ImGui::DragFloat("Tile Size", &tileSize_, 1.0f, 1.0f, 256.0f) ;
	if (ImGui::Button("refresh"))
	{
		for (int y = 0; y < mapHeight_; ++y)
		{
			for (int x = 0; x < mapWidth_; ++x)
			{
				UpdateTileSprite(x, y);
			}
		}
	}

    ImGui::End();
#endif // _DEBUG
}

// タイル 1 つ分を更新するヘルパー関数
void GameScene::UpdateTileSprite(int x, int y)
{
    int tileType = mapData_[y][x];
    const char* texturePath = ConvertNumTex(tileType);

    pSpriteTile_[y][x]->Initialize(texturePath);
    pSpriteTile_[y][x]->SetName("Tile" + std::to_string(y * mapWidth_ + x));
    pSpriteTile_[y][x]->SetAnchorPoint({ 0.5f, 0.5f });
    pSpriteTile_[y][x]->SetPosition({ tileSize_ / 2.0f + tileSize_ * x + mapOffset_.x, tileSize_ / 2.0f + tileSize_ * y + mapOffset_.y });
    pSpriteTile_[y][x]->SetSize({ tileSize_, tileSize_ });
}
