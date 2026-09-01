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
	for (auto& tile : pSpriteTile_)
	{
		tile->Update();
	}
}

void GameScene::Draw()
{
    CanvasScope canvasScopeBack(pCanvasSprite_.get());
	for (auto& tile : pSpriteTile_)
	{
		tile->Draw1F();
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

    for (int i = 0; i < 9; ++i)
    {
        pSpriteTile_[i] = std::make_unique<Sprite>();
        pSpriteTile_[i]->Initialize(Path::Image::InGame::kTestTile);
        pSpriteTile_[i]->SetName("Tile" + std::to_string(i));
        pSpriteTile_[i]->SetAnchorPoint({ 0.5f, 0.5f });
        pSpriteTile_[i]->SetPosition({ 25.0f, 25.0f + 50.0f * i });
		pSpriteTile_[i]->SetSize({ 50.0f, 50.0f });
    }

    /// タイトルテキストの初期化
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
