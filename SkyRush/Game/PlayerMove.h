#pragma once

#include "Level3DRender/LevelRender.h"

class PlayerMove
{
public:
    void Init();
    void Update();

    Vector3 GetPosition() const;

    // 開始地点から自機までの距離を取得
    float GetDistanceFromStart() const;

    Vector3 GetForward() const;
    Vector3 GetVelocity() const;
    Vector3 GetRight() const;
    Quaternion GetRotation() const;
    float GetYawAngle() const;

private:
    Vector3 m_pos;
    Vector3 m_velocity;
    float m_bankAngle = 0.0f;
    float m_yawAngle = 0.0f;
    float m_speed = 5.0f;       // 現在の飛行速度
    float m_minSpeed = 2.0f;    // 最低速度
    float m_maxSpeed = 15.0f;   // 最高速度
    float m_pitchAngle = 0.0f; // 上下方向の傾き
};
