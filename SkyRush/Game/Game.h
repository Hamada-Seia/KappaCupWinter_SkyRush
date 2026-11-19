#pragma once

#include "Level3DRender/LevelRender.h"
#include "Bullets.h"
#include "PlayerMove.h"
#include "PlayerHP.h"
#include "Enemy.h"
#include "graphics/SpriteRender.h"
#include "graphics/FontRender.h"
#include "graphics/effect/EffectEmitter.h"
#include "graphics/effect/EffectEngine.h"

class Game : public IGameObject
{
public:
    Game() {}
    ~Game() {}

    bool Start();
    void Update();
    void Render(RenderContext& rc);

    enum class GameState
    {
        Title,
        Playing,
        Clear,
        GameOver
    };

private:
    ModelRender m_modelRender;
    ModelRender m_seaRender;

    // 危険区域用の紫色の海面
    ModelRender m_purpleSeaRender;
    // 紫色の海面を表示中か
    bool m_isPurpleSea = false;

    // 自機の移動
    PlayerMove m_playerMove;

    // 海面の端までの距離（海面倍率200の場合）
    const float SEA_HALF_X = 13700.30f;
    const float SEA_HALF_Z = 13344.70f;

    // 海面の端からの残り距離で危険度を判定
    const float WARNING_REMAINING = 3600.0f;
    const float DAMAGE_REMAINING = 2800.0f;
    const float GAMEOVER_REMAINING = 1000.0f;

    // 飛行範囲ダメージ
    int m_boundaryDamageTimer = 0;
    const int BOUNDARY_DAMAGE_INTERVAL = 15;
    const int BOUNDARY_DAMAGE = 5;

    // 警告範囲に入っているかどうか
    bool m_wasInWarningArea = false;

    // 自機のエンジン音
    SoundSource* m_engineSound = nullptr;

    // 弾の管理
    Bullets m_bullets;
    int m_shotCoolTime = 0;

    // オーバーヒート管理
    float m_heat = 0.0f;             // 現在の熱量
    const float MAX_HEAT = 100.0f;   // 最大熱量
    const float HEAT_PER_SHOT = 5.0f; // 1発ごとの熱量
    const float COOL_SPEED = 0.8f;   // 冷却速度

    bool m_isOverheated = false;     // オーバーヒート中か
    int m_overheatTimer = 0;         // 射撃禁止時間
    const int OVERHEAT_TIME = 180;   // 約3秒

    // 自機のエンジン噴射エフェクト
    EffectEmitter* m_engineEffect = nullptr;

    //PlayerのHP
    PlayerHP m_playerHP;

    //Enemy
    Enemy m_enemy;

    // UI表示
    nsK2Engine::SpriteRender m_reticle;
    nsK2Engine::SpriteRender m_ab;

    // HP表示
    nsK2Engine::SpriteRender m_hpBase;
    nsK2Engine::SpriteRender m_hpGauge;

    // HP文字表示
    nsK2Engine::FontRender m_hpLabel;
    nsK2Engine::FontRender m_hpNumber;

    // オーバーヒート表示
    nsK2Engine::SpriteRender m_heatBase;
    nsK2Engine::SpriteRender m_heatGauge;

    // オーバーヒート文字表示
    nsK2Engine::FontRender m_heatLabel;
    nsK2Engine::FontRender m_heatNumber;

    // 飛行範囲の警告表示
    nsK2Engine::FontRender m_boundaryWarning;

    // 画面UI
    nsK2Engine::SpriteRender m_titleUI;
    nsK2Engine::SpriteRender m_clearUI;
    nsK2Engine::SpriteRender m_gameOverUI;

    // 小敵の攻撃予告ライン
    nsK2Engine::SpriteRender m_warningLine;

    // ゲームの進行状態
    GameState m_gameState = GameState::Title;

    // 撃墜した小敵の数
    int m_defeatedEnemyCount = 0;

    // 小敵の最大出現数(アルファ版)
    const int MAX_ENEMY_COUNT = 5;
};
