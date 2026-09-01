#pragma once

#include <scene/SceneBase.h>
#include <Core/Window/Window.h>
#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/PostEffectExecutor.h>
#include <Features/GameEye/GameEye.h>
#include <Features/Input/Input.h>
#include <Features/SceneManager/SceneManager.h>
#include <Features/Cubemap/Skybox.h>
#include <Features/Cubemap/CubemapSystem.h>
#include <Features/Layer/Canvas.h>
#include <Features/Input/InputMapper.hpp>
#include <Features/Audio/Audio.h>
#include <Math/ViewportUnits.hpp>
#include <drawable/sprite/Sprite.h>
#include <logic/input/InputAction.h>

/// <summary>
/// タイトルシーン
/// </summary>
class GameScene : public SceneBase
{
public:
    GameScene(ISceneArgs* _pArg) : SceneBase(_pArg) {};

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 終了
    /// </summary>
    void Finalize() override;

    /// <summary>
    /// 更新
    /// </summary>
    void Update() override;

    /// <summary>
    /// 描画
    /// </summary>
    void Draw() override;


private:
    void InitializeGameEye();
    void InitializeSprites();
    void InitializeSkybox();


    std::unique_ptr<Canvas>             pCanvasBack_ = nullptr;      // !< タイトルキャンバス
    std::unique_ptr<Canvas>             pCanvasSprite_ = nullptr;      // !< タイトルキャンバス
    std::unique_ptr<GameEye>            gameEye_ = {};           // !< ゲームアイ
    std::unique_ptr<Skybox>             pSkybox_ = nullptr;      // !< スカイボックス
    std::array<std::unique_ptr<Sprite>, 9> pSpriteTile_;         // !< タイル

    /// 他クラスのインスタンス
    PostEffectExecutor* pPostEffectExecutor_ = nullptr;      // !< ポストエフェクト実行クラス
    DirectX12* pDx12_ = nullptr;      // !< DirectX12
    Input* pInput_ = nullptr;      // !< 入力w
    SceneManager* pSceneManager_ = nullptr;      // !< シーン遷移
    CubemapSystem* pCubemapSystem_ = nullptr;      // !< キューブマップシステム
    InputMapper<InputActionUI>* pInputMapperUI_ = nullptr;      // !< 入力マッパー
};