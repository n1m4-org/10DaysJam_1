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
#include <Utility/JSONIO/JSONIO.h>
#include <object/baseObject2d/BaseObject2d.h>
#include <object/player/Player.h>
#include <object/router/Router.h>
#include <object/repeater/Repeater.h>
#include <object/alumiWall/AlumiWall.h>
#include <object/pc/PC.h>
#include <logic/mapCollision/MapCollision.h>
#include <logic/signal/SignalSystem.h>

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

    void MapEdit();
    void UpdateTileSprite(int x, int y);
	void MapLoad(const std::string& path);
	void MapSave(const std::string& path);
	void UpdateCurrentMap();
	void InitializeTestObjects();


	// 指定タイプのオブジェクトを作成し配置するヘルパー関数
	BaseObject2d* CreateObject(ObjectType2d type, const Vector2Int& pos, const Vector2Int& dir = { 0, 1 });
	// 指定座標にあるオブジェクトを削除する
	void RemoveObjectAt(const Vector2Int& pos);

    std::unique_ptr<Canvas>             pCanvasBack_ = nullptr;      // !< キャンバス
    std::unique_ptr<Canvas>             pCanvasSprite_ = nullptr;      // !< キャンバス
    std::unique_ptr<GameEye>            gameEye_ = {};           // !< ゲームアイ
    std::unique_ptr<Skybox>             pSkybox_ = nullptr;      // !< スカイボックス
    std::vector<std::vector<std::unique_ptr<Sprite>>> pSpriteTile_;         // !< タイル
    std::vector<std::vector<std::unique_ptr<Sprite>>> pSignalSpriteTile_;   // !< 電波可視化用スプライト

    std::vector<std::unique_ptr<BaseObject2d>> pMapObjects_;  // !< マップオブジェクト
    Player* pPlayer_ = nullptr; // プレイヤーの参照キャッシュ

    std::vector<std::vector<int>> mapData_;  // !< 地形マップデータ
    std::vector<std::vector<int>> currentMap_;  // !< 現在のマップ（合成）
    std::vector<std::vector<int>> signalStrengthMap_; // !< 各マスの電波強度 (0~10)
    int pcReceivedStrength_ = 0;               // !< PCが受信した電波の強度
    bool isPcConnected_ = false;               // !< PCに電波が届いているか
    std::string savePath_ = "";                   // !< マップデータ保存先


	Vector2 mapOffset_ = { 350.0f, 0.0f };   // !< マップのオフセット

	float tileSize_ = 100.0f;   // !< タイルのサイズ
	int mapWidth_ = 9;      // !< マップの幅
	int mapHeight_ = 9;      // !< マップの高さ

    MapCollision mapCollision_; // !< 衝突・押し出し判定
    SignalSystem signalSystem_; // !< 電波伝搬システム


    /// 他クラスのインスタンス
    PostEffectExecutor* pPostEffectExecutor_ = nullptr;      // !< ポストエフェクト実行クラス
    DirectX12* pDx12_ = nullptr;      // !< DirectX12
    Input* pInput_ = nullptr;      // !< 入力
    SceneManager* pSceneManager_ = nullptr;      // !< シーン遷移
    CubemapSystem* pCubemapSystem_ = nullptr;      // !< キューブマップシステム
    InputMapper<InputActionUI>* pInputMapperUI_ = nullptr;      // !< 入力マッパー
	JSONIO* pJSONIO_ = nullptr;      // !< JSONIO
};