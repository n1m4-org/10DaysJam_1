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
#include <object/ObjectRule.h>

void GameScene::Initialize()
{
    /// インスタンスの取得
    pInput_ = Input::GetInstance();
    pSceneManager_ = SceneManager::GetInstance();
    pCubemapSystem_ = std::any_cast<CubemapSystem*>(pArgs_->Get("CubemapSystem"));
    pDx12_ = std::any_cast<DirectX12*>(pArgs_->Get("DirectX12"));
    pInputMapperUI_ = std::any_cast<InputMapper<InputActionUI>*>(pArgs_->Get("InputMapperUI"));
	pJSONIO_ = JSONIO::GetInstance();

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

	// マップデータの読み込み
    MapLoad("test_map.json");

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
    // 地形タイルの更新
	for (auto& tileRow : pSpriteTile_)
	{
		for (auto& tile : tileRow)
		{
			tile->Update();
		}
	}

    // プレイヤーのキー入力制御 (WASD移動・押し出し)
    if (pPlayer_)
    {
        pPlayer_->HandleInput(pInput_, mapCollision_, currentMap_, pMapObjects_);
    }

    // 全オブジェクトの座標およびアニメーション更新
    for (auto& object : pMapObjects_)
    {
        if (object)
        {
            object->UpdateSpritePosition(tileSize_, mapOffset_);
            object->Update();
        }
    }

    // オブジェクトの移動に合わせて最新の合成マップ（マップ番号）を更新
    this->UpdateCurrentMap();

    // 電波伝搬・減衰の計算 (初期強度 10 マス)
    isPcConnected_ = signalSystem_.UpdateSignal(mapWidth_, mapHeight_, currentMap_, pMapObjects_, signalStrengthMap_, pcReceivedStrength_);

    // PCオブジェクトへ電波受信状態と強度の通知
    for (auto& object : pMapObjects_)
    {
        if (object && object->GetObjectType() == ObjectType2d::kPC)
        {
            PC* pcObj = static_cast<PC*>(object.get());
            pcObj->UpdateSignal(isPcConnected_, pcReceivedStrength_);
        }
    }

    // 電波表示タイルの更新 (強度10=濃い黄金色から強度1=非常に薄い黄色へと1段階ごとにグラデーション)
    for (int y = 0; y < mapHeight_; ++y)
    {
        for (int x = 0; x < mapWidth_; ++x)
        {
            int strength = (y < static_cast<int>(signalStrengthMap_.size()) && x < static_cast<int>(signalStrengthMap_[y].size())) ? signalStrengthMap_[y][x] : 0;
            if (pSignalSpriteTile_[y][x])
            {
                bool hasSignal = (strength > 0);
                pSignalSpriteTile_[y][x]->SetEnableDraw(hasSignal);

                if (hasSignal)
                {
                    // 強度 1~10 を 0.0f~1.0f に正規化
                    float norm = static_cast<float>(strength - 1) / 9.0f;
                    if (norm < 0.0f) norm = 0.0f;
                    if (norm > 1.0f) norm = 1.0f;

                    // 10段階で濃い黄金色 (norm=1) ➔ 薄いレモンイエロー (norm=0) へ線形補間
                    float r = 1.0f;
                    float g = 0.96f - 0.16f * norm; // S=10: 0.80, S=1: 0.96
                    float b = 0.45f - 0.40f * norm; // S=10: 0.05, S=1: 0.45
                    float a = 0.16f + 0.72f * norm; // S=10: 0.88, S=1: 0.16

                    pSignalSpriteTile_[y][x]->SetColor({ r, g, b, a });
                    pSignalSpriteTile_[y][x]->SetPosition({ tileSize_ / 2.0f + tileSize_ * x + mapOffset_.x, tileSize_ / 2.0f + tileSize_ * y + mapOffset_.y });
                    pSignalSpriteTile_[y][x]->SetSize({ tileSize_ * 0.70f, tileSize_ * 0.70f });
                    pSignalSpriteTile_[y][x]->Update();

                }
            }
        }
    }



    // エディタの更新
	this->MapEdit();
}

void GameScene::Draw()
{
    CanvasScope canvasScopeBack(pCanvasSprite_.get());

    // 1. 最背面: 床・壁タイルを描画
    for (auto& tileRow : pSpriteTile_)
    {
        for (auto& tile : tileRow)
        {
            tile->Draw1F();
        }
    }

    // 2. 中間: 電波の通過レイヤを描画
    for (auto& signalRow : pSignalSpriteTile_)
    {
        for (auto& signalTile : signalRow)
        {
            if (signalTile && signalTile->GetEnableDraw())
            {
                signalTile->Draw1F();
            }
        }
    }

    // 3. 前面: オブジェクトを描画
    for (auto& object : pMapObjects_)
    {
        if (object)
        {
            object->Draw();
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

void GameScene::InitializeSkybox()
{
    auto pTM = TextureManager::GetInstance();
    pTM->LoadTexture(Path::Image::kTitleSkybox);

    pSkybox_ = std::make_unique<Skybox>();
    pSkybox_->Initialize(pCubemapSystem_);
    pSkybox_->SetSkyboxTexture(pTM->GetSrvHandleGPU(Path::Image::kTitleSkybox));

    pCanvasBack_->RegisterDrawable(pSkybox_.get());
}

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

void GameScene::InitializeSprites()
{
    TextureManager* tm = TextureManager::GetInstance();
    tm->LoadTexture(Path::Image::InGame::kTestTile);

    pSpriteTile_.resize(mapHeight_);
    pSignalSpriteTile_.resize(mapHeight_);

    for (int y = 0; y < mapHeight_; ++y)
    {
        pSpriteTile_[y].resize(mapWidth_);
        pSignalSpriteTile_[y].resize(mapWidth_);

        for (int x = 0; x < mapWidth_; ++x)
        {
            pSpriteTile_[y][x] = std::make_unique<Sprite>();
            UpdateTileSprite(x, y);

            pSignalSpriteTile_[y][x] = std::make_unique<Sprite>();
            pSignalSpriteTile_[y][x]->Initialize(Path::Image::InGame::kTestTile);
            pSignalSpriteTile_[y][x]->SetAnchorPoint({ 0.5f, 0.5f });
            pSignalSpriteTile_[y][x]->SetColor({ 1.0f, 0.8f, 0.05f, 0.45f }); // 電波用黄色（半透明）
            pSignalSpriteTile_[y][x]->SetPosition({ tileSize_ / 2.0f + tileSize_ * x + mapOffset_.x, tileSize_ / 2.0f + tileSize_ * y + mapOffset_.y });
            pSignalSpriteTile_[y][x]->SetSize({ tileSize_ * 0.70f, tileSize_ * 0.70f });

        } 
    }

    for (auto& object : pMapObjects_)
    {
        if (object)
        {
            object->UpdateSpritePosition(tileSize_, mapOffset_);
        }
    }
}


void GameScene::MapEdit()
{
#ifdef _DEBUG
    ImGui::Begin("Map Editor");

    // 電波の接続ステータスを表示
    if (isPcConnected_)
    {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.3f, 1.0f), "[SIGNAL STATUS]: PC CONNECTED");
    }
    else
    {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.3f, 1.0f), "[SIGNAL STATUS]: NO SIGNAL REACHED PC");
    }
    ImGui::Separator();

    // --- 1. 壁・タイルの種類 ---
    static const char* kWallTypeNames[] = {
		"Simple Tile", "LB", "LBT", "LRB", "LRT", "LT", "RB", "RBT", "RL", "RT", "TB", "B", "L", "R", "T"
    };

    // モード選択（0: 地形ペイント, 1: オブジェクト配置, 2: オブジェクト向き回転）
    static int editorMode = 0;
    ImGui::RadioButton("Terrain Mode", &editorMode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Object Mode", &editorMode, 1);

    ImGui::Separator();

    static int selectedWallIdx = 0;
    static int selectedObjectType = 1; // 1: Player, 2: Router, 3: Repeater, 4: AlumiWall, 5: PC, 0: Remove
    static int selectedDirIdx = 2;     // 0: Up (0,-1), 1: Right (1,0), 2: Down (0,1), 3: Left (-1,0)

    static const Vector2Int kDirVectors[] = { {0, -1}, {1, 0}, {0, 1}, {-1, 0} };
    static const char* kDirNames[] = { "Up (0, -1)", "Right (1, 0)", "Down (0, 1)", "Left (-1, 0)" };

    if (editorMode == 0)
    {
        ImGui::Text("Terrain Palette");
        ImGui::Combo("Wall Type", &selectedWallIdx, kWallTypeNames, IM_ARRAYSIZE(kWallTypeNames));
    }
    else
    {
        ImGui::Text("Object Palette");
        static const char* kObjectNames[] = { "Delete / Clear", "Player (Blue)", "Router (Yellow)", "Repeater (Green)", "AlumiWall (Gray)", "PC (Purple)" };
        ImGui::Combo("Object Type", &selectedObjectType, kObjectNames, IM_ARRAYSIZE(kObjectNames));

        if (selectedObjectType == 2 || selectedObjectType == 3) // Router または Repeater の場合、向き設定を表示
        {
            ImGui::Combo("Placement Direction", &selectedDirIdx, kDirNames, IM_ARRAYSIZE(kDirNames));
        }
    }

    ImGui::Separator();
    ImGui::Text("Map Grid (Click to paint/place, Right-Click to Rotate)");

    bool isMapEditedThisFrame = false;

    for (int y = 0; y < mapHeight_; ++y)
    {
        for (int x = 0; x < mapWidth_; ++x)
        {
            ImGui::PushID(y * mapWidth_ + x);

            // マスに存在するオブジェクトがあるか確認
            BaseObject2d* objAtPos = nullptr;
            for (const auto& obj : pMapObjects_)
            {
                if (obj && obj->GetPosition() == Vector2Int{ x, y })
                {
                    objAtPos = obj.get();
                    break;
                }
            }

            // ラベル表示
            std::string label;
            if (objAtPos)
            {
                switch (objAtPos->GetObjectType())
                {
                case ObjectType2d::kPlayer:    label = " P"; break;
                case ObjectType2d::kRouter:    label = " R"; break;
                case ObjectType2d::kRepeater:  label = " M"; break;
                case ObjectType2d::kAlumiWall:label = " W"; break;
                case ObjectType2d::kPC:       label = " C"; break;
                default: label = " O"; break;
                }
            }
            else
            {
                label = (mapData_[y][x] == 0) ? " ." : std::to_string(mapData_[y][x] - ObjectRule::kMapWallType);
            }

            ImGui::Button(label.c_str(), ImVec2(28, 28));

            // 左クリックで配置・操作
            if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                if (editorMode == 0) // 地形ペイント
                {
                    int targetTileType = (selectedWallIdx == 0) ? 0 : (selectedWallIdx + ObjectRule::kMapWallType);
                    if (mapData_[y][x] != targetTileType)
                    {
                        mapData_[y][x] = targetTileType;
                        UpdateTileSprite(x, y);
                        isMapEditedThisFrame = true;
                    }
                }
                else // オブジェクト配置
                {
                    ObjectType2d targetObjType = static_cast<ObjectType2d>(selectedObjectType);
                    if (targetObjType == ObjectType2d::None)
                    {
                        RemoveObjectAt({ x, y });
                    }
                    else
                    {
                        CreateObject(targetObjType, { x, y }, kDirVectors[selectedDirIdx]);
                    }
                    isMapEditedThisFrame = true;
                }
            }
            // 右クリックで既存オブジェクトの向きを時計回りに回転
            else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
                if (objAtPos)
                {
                    Vector2Int curDir = objAtPos->GetAngle();
                    Vector2Int nextDir = { 0, 1 };
                    if (curDir.x == 0 && curDir.y == -1) nextDir = { 1, 0 };      // 上 -> 右
                    else if (curDir.x == 1 && curDir.y == 0) nextDir = { 0, 1 };  // 右 -> 下
                    else if (curDir.x == 0 && curDir.y == 1) nextDir = { -1, 0 }; // 下 -> 左
                    else if (curDir.x == -1 && curDir.y == 0) nextDir = { 0, -1 };// 左 -> 上

                    objAtPos->SetAngle(nextDir);
                    isMapEditedThisFrame = true;
                }
            }

            ImGui::PopID();

            if (x < mapWidth_ - 1)
            {
                ImGui::SameLine();
            }
        }
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && isMapEditedThisFrame)
    {
        MapSave(savePath_);
    }

    ImGui::Separator();
    if (ImGui::Button("Save JSON"))
    {
        MapSave(savePath_);
    }
    ImGui::SameLine();
	ImGui::PushItemWidth(100);

	ImGui::DragFloat2("Map Offset", &mapOffset_.x, 1.0f, -1000.0f, 1000.0f);
    ImGui::SameLine();
	ImGui::DragFloat("Tile Size", &tileSize_, 1.0f, 1.0f, 256.0f);
	if (ImGui::Button("refresh"))
	{
		for (int y = 0; y < mapHeight_; ++y)
		{
			for (int x = 0; x < mapWidth_; ++x)
			{
				UpdateTileSprite(x, y);
			}
		}
        for (auto& object : pMapObjects_)
        {
            if (object)
            {
                object->UpdateSpritePosition(tileSize_, mapOffset_);
            }
        }
	}
    ImGui::PopItemWidth();

    ImGui::End();
#endif // _DEBUG
}

BaseObject2d* GameScene::CreateObject(ObjectType2d type, const Vector2Int& pos, const Vector2Int& dir)
{
    // 指定位置に既存のオブジェクトがあれば削除
    RemoveObjectAt(pos);

    std::unique_ptr<BaseObject2d> newObj = nullptr;

    switch (type)
    {
    case ObjectType2d::kPlayer:
        newObj = std::make_unique<Player>();
        break;
    case ObjectType2d::kRouter:
        newObj = std::make_unique<Router>();
        break;
    case ObjectType2d::kRepeater:
        newObj = std::make_unique<Repeater>();
        break;
    case ObjectType2d::kAlumiWall:
        newObj = std::make_unique<AlumiWall>();
        break;
    case ObjectType2d::kPC:
        newObj = std::make_unique<PC>();
        break;
    default:
        return nullptr;
    }

    if (newObj)
    {
        newObj->SetPosition(pos);
        newObj->SetAngle(dir);
        newObj->Initialize();
        newObj->UpdateSpritePosition(tileSize_, mapOffset_);

        BaseObject2d* ptr = newObj.get();
        if (type == ObjectType2d::kPlayer)
        {
            pPlayer_ = static_cast<Player*>(ptr);
        }

        pMapObjects_.push_back(std::move(newObj));
        UpdateCurrentMap();
        return ptr;
    }

    return nullptr;
}

void GameScene::RemoveObjectAt(const Vector2Int& pos)
{
    for (auto it = pMapObjects_.begin(); it != pMapObjects_.end(); ++it)
    {
        if (*it && (*it)->GetPosition() == pos)
        {
            if ((*it).get() == pPlayer_)
            {
                pPlayer_ = nullptr;
            }
            pMapObjects_.erase(it);
            break;
        }
    }
    UpdateCurrentMap();
}

void GameScene::MapSave(const std::string& path)
{
    nlohmann::json rootJson;
    rootJson["tiles"] = mapData_;

    nlohmann::json objectsJson = nlohmann::json::array();
    for (const auto& obj : pMapObjects_)
    {
        if (obj)
        {
            nlohmann::json objJson;
            objJson["type"] = static_cast<int>(obj->GetObjectType());
            objJson["x"] = obj->GetPosition().x;
            objJson["y"] = obj->GetPosition().y;
            objJson["dirX"] = obj->GetAngle().x;
            objJson["dirY"] = obj->GetAngle().y;
            objectsJson.push_back(objJson);
        }
    }
    rootJson["objects"] = objectsJson;

    pJSONIO_->Save(path, rootJson);
}

void GameScene::InitializeTestObjects()
{
    pMapObjects_.clear();
    pPlayer_ = nullptr;

    // テスト用の初期オブジェクト配置 (ルーター:右向き, 中継器:下向き)
    CreateObject(ObjectType2d::kPlayer, { 2, 2 }, { 0, 1 });
    CreateObject(ObjectType2d::kRouter, { 1, 1 }, { 1, 0 }); // 右向きルーター
    CreateObject(ObjectType2d::kRepeater, { 5, 1 }, { 0, 1 }); // 下向き中継器
    CreateObject(ObjectType2d::kAlumiWall, { 4, 4 }, { 0, 1 });
    CreateObject(ObjectType2d::kPC, { 5, 7 }, { 0, 1 });
}

void GameScene::MapLoad(const std::string& path)
{
    savePath_ = std::string(Path::Resource::kJsonDir) + Path::Json::kMapDir + path.c_str();
    nlohmann::json mapJson = pJSONIO_->Load(savePath_);

    pMapObjects_.clear();
    pPlayer_ = nullptr;

    bool hasObjectsInJson = false;

    if (mapJson.contains("tiles"))
    {
        // 拡張形式 (tiles + objects)
        nlohmann::json tilesJson = mapJson["tiles"];
        mapHeight_ = static_cast<int>(tilesJson.size());
        mapWidth_ = mapHeight_ > 0 ? static_cast<int>(tilesJson[0].size()) : 0;

        mapData_.resize(mapHeight_);
        for (int y = 0; y < mapHeight_; ++y)
        {
            mapData_[y].resize(mapWidth_);
            for (int x = 0; x < mapWidth_; ++x)
            {
                mapData_[y][x] = tilesJson[y][x].get<int>();
            }
        }

        if (mapJson.contains("objects") && mapJson["objects"].is_array() && !mapJson["objects"].empty())
        {
            hasObjectsInJson = true;
            for (const auto& objJson : mapJson["objects"])
            {
                ObjectType2d type = static_cast<ObjectType2d>(objJson["type"].get<int>());
                int x = objJson["x"].get<int>();
                int y = objJson["y"].get<int>();
                Vector2Int dir = { 0, 1 };
                if (objJson.contains("dirX") && objJson.contains("dirY"))
                {
                    dir.x = objJson["dirX"].get<int>();
                    dir.y = objJson["dirY"].get<int>();
                }
                CreateObject(type, { x, y }, dir);
            }
        }
    }
    else
    {
        // 従来形式 (単純な2次元配列)
        mapHeight_ = static_cast<int>(mapJson.size());
        mapWidth_ = mapHeight_ > 0 ? static_cast<int>(mapJson[0].size()) : 0;

        mapData_.resize(mapHeight_);
        for (int y = 0; y < mapHeight_; ++y)
        {
            mapData_[y].resize(mapWidth_);
            for (int x = 0; x < mapWidth_; ++x)
            {
                mapData_[y][x] = mapJson[y][x].get<int>();
            }
        }
    }

    // JSONにオブジェクト情報がなかった場合はテスト用配置を作成して最新形式で保存
    if (!hasObjectsInJson)
    {
        InitializeTestObjects();
        MapSave(savePath_);
    }

    UpdateCurrentMap();
}



void GameScene::UpdateCurrentMap()
{
    currentMap_ = mapData_;
	for (auto&& object : pMapObjects_)
	{
		Vector2Int position = object->GetPosition();
        if (position.y >= 0 && static_cast<size_t>(position.y) < currentMap_.size() &&
            position.x >= 0 && static_cast<size_t>(position.x) < currentMap_[position.y].size())
        {
            currentMap_[position.y][position.x] = object->IsDynamic() ? 
                static_cast<int>(object->GetObjectType()) + ObjectRule::kDynamicObjectType :
                static_cast<int>(object->GetObjectType()) + ObjectRule::kStaticObjectType;
        }
	}
}

