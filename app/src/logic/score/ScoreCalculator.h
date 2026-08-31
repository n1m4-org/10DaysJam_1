#pragma once
#include <Features/Text/Text.h>
#include <memory>
#include <drawable/font/NumericView.h>
#include <entity/enemy/EnemyType.h>
#include <optional>
#include <unordered_map>
#include <Features/event/EventSubscription.h>


namespace ScorePerUnit
{
    static constexpr auto kEnemyNormal = 50u;
    static constexpr auto kEnemyRusher = 150u;
}

/// <summary>
/// スコア計算クラス
/// </summary>
class ScoreCalculator
{
public:
    /// <summary>
    /// スコア表示用テキスト等を初期化します。
    /// </summary>
    void Initialize();

    /// <summary>
    /// スコアの加算アニメーション等の更新を行います。
    /// </summary>
    void Update();

    /// <summary>
    /// スコアのテキスト描画を行います。
    /// </summary>
    void Draw1F();

    /// <summary>
    /// 終了処理を行います。
    /// </summary>
    void Finalize();

    /// <summary>
    /// 敵撃破数をカウントし、スコア加算を行います。
    /// </summary>
    void CountEnemyDeath(EnemyType type);

    /// <summary>
    /// スコアを取得します。
    /// </summary>
    /// <returns>現在のスコア値。</returns>
    float GetScore() const { return score_; }

    /// <summary>
    /// スコア倍率を設定します（例: x2, x3など）。倍率が有効な場合、毎フレーム加算されるスコア量が増加します。
    /// </summary>
    /// <param name="isActive">アクティブ</param>
    void SetScoreMultiplier2x(bool isActive) { multiplier_ = isActive ? 2u : 1u; }
    void SetScoreMultiplier3x(bool isActive) { multiplier_ = isActive ? 3u : 1u; }

private:
    void InitializeScoreTable();
    void InitializeNumericView();
    void UpdateNumericView();
    void UpdateDisplayScore();
    
    constexpr static uint32_t   kNumDigits_             = 8u;       // スコア最大桁数
    constexpr static float      kFontHeight_            = 48.0f;    // フォント幅
                     uint32_t   scoreIncrementPerFrame_ = 20u;      // 毎フレームビュー用に加算するスコア量（例: 20点/フレーム）。倍率がかかるとこの値が増加します。
                     Vector2    scoreLeftTop_           = {};       // スコア表示の左上位置

    /// メンバー変数
    float           score_              = 0.0f;
    unsigned int    enemyDeathCount_    = 0u;
    float           receiveAddScore_    = 0.0f;
    uint32_t        multiplier_         = 1u;     // スコア倍率（例: x2, x3など）
    std::unordered_map<EnemyType, uint32_t> scoreTable_;
    std::optional<EventSubscription> subKillEnemy_ = std::nullopt;

    // 毎フレーム加算するスコア量
    std::unique_ptr<Text>           pName_    = nullptr;
    std::unique_ptr<NumericView>    pScore_   = nullptr;
};