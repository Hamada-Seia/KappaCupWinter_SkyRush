#pragma once

#include "Level3DRender/LevelRender.h"

class Enemy
{
public:
    void Init(const Vector3& pos);
    void Update(const Vector3& playerPos);
    void Render(RenderContext& rc);

    void Damage(int damage);

    Vector3 GetPosition() const;
    bool IsDead() const;

    void SetPosition(const Vector3& pos);

    // 突進を開始した瞬間か確認する
    bool IsChargeStarted() const;

    // この突進ですでにダメージを与えたか確認・変更する
    bool HasDamaged() const;
    void SetHasDamaged(bool hasDamaged);

private:
    ModelRender m_modelRender;
    Vector3 m_pos;

    //プレイヤーと距離を保ちつつ追従
    float m_moveSpeed = 7.0f;
    float m_keepDistance = 200.0f;

    // 突進攻撃
    bool m_isCharging = false;
    int m_attackTimer = 0;
    int m_chargeTimer = 0;

    // 突進開始時のダメージ判定を管理
    bool m_hasDamaged = false;

    // 突進開始を知らせるフラグ
    bool m_chargeStarted = false;

    //小敵のHP
    int m_maxHP = 30;
    int m_hp = 30;
};

