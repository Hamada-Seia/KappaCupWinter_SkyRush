#pragma once

#include "Level3DRender/LevelRender.h"

class Enemy
{
public:
    void Init(const Vector3& pos);
    void Update(const Vector3& playerPos, const Vector3& playerForward);
    void Render(RenderContext& rc);

    void Damage(int damage);

    Vector3 GetPosition() const;
    bool IsDead() const;

    void SetPosition(const Vector3& pos);

    // 突進を開始した瞬間か確認する
    bool IsChargeStarted() const;

    // 攻撃予告中か確認
    bool IsWarning() const;

    // 攻撃対象として保存した座標を取得
    Vector3 GetSavedPlayerPos() const;

    // この突進ですでにダメージを与えたか確認・変更する
    bool HasDamaged() const;
    void SetHasDamaged(bool hasDamaged);

private:
    ModelRender m_modelRender;
    Vector3 m_pos;

    // 敵が最初に出現した位置（帰還先）
    Vector3 m_homePos;

    // 突進終了後、初期位置へ戻っているか
    bool m_isReturning = false;

    //プレイヤーと距離を保ちつつ追従
    float m_moveSpeed = 7.0f;
    float m_keepDistance = 200.0f;

    // 突進攻撃
    bool m_isCharging = false;
    int m_attackTimer = 0;
    int m_chargeTimer = 0;

    // 攻撃予告
    bool m_isWarning = false;          // 赤ライン表示中か
    int m_warningTimer = 0;            // 予告時間
    Vector3 m_savedPlayerPos;          // 予告開始時のプレイヤー座標
    Vector3 m_chargeDirection;        // 突進方向


    // 突進開始時のダメージ判定を管理
    bool m_hasDamaged = false;

    // 突進開始を知らせるフラグ
    bool m_chargeStarted = false;

    //小敵のHP
    int m_maxHP = 30;
    int m_hp = 30;
};

