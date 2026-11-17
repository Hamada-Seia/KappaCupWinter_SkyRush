#include "stdafx.h"
#include "Windows.h"
#include "PlayerMove.h"

using namespace nsK2EngineLow;

void PlayerMove::Init()
{
    m_speed = 5.0f;
    m_pos = Vector3::Zero;
    m_velocity = Vector3::Zero;
    m_bankAngle = 0.0f;
    m_yawAngle = 0.0f;
    m_pitchAngle = 0.0f;
}

void PlayerMove::Update()
{
    float moveX = 0.0f;
    float moveY = 0.0f;
    float throttle = 0.0f;

    // PAD操作
    if (g_pad[0] != nullptr)
    {
        moveX += g_pad[0]->GetLStickXF();
        throttle += g_pad[0]->GetLStickYF();

        if (g_pad[0]->IsPress(enButtonLB1))
            moveY -= 1.0f;

        if (g_pad[0]->IsPress(enButtonRB1))
            moveY += 1.0f;
    }

    // キーボード操作
    if (GetAsyncKeyState('A') & 0x8000) moveX -= 1.0f;
    if (GetAsyncKeyState('D') & 0x8000) moveX += 1.0f;
    if (GetAsyncKeyState('W') & 0x8000) throttle += 1.0f;
    if (GetAsyncKeyState('S') & 0x8000) throttle -= 1.0f;
    if (GetAsyncKeyState('Q') & 0x8000) moveY -= 1.0f;
    if (GetAsyncKeyState('E') & 0x8000) moveY += 1.0f;

    // 入力の制限
    if (moveX > 1.0f) moveX = 1.0f;
    if (moveX < -1.0f) moveX = -1.0f;

    if (throttle > 1.0f) throttle = 1.0f;
    if (throttle < -1.0f) throttle = -1.0f;

    // 加速・減速
    m_speed += throttle * 0.1f;

    if (m_speed > m_maxSpeed) m_speed = m_maxSpeed;
    if (m_speed < m_minSpeed) m_speed = m_minSpeed;

    // 旋回
    m_yawAngle += moveX * 2.0f;

    float rad = m_yawAngle * 3.14159265f / 180.0f;

    // 機首方向
    float forwardX = sinf(rad);
    float forwardZ = cosf(rad);

    // 横方向
    float rightX = cosf(rad);
    float rightZ = -sinf(rad);

    // 自機の移動量を計算
    m_velocity.x = forwardX * m_speed + rightX * moveX * 5.0f;
    m_velocity.z = forwardZ * m_speed + rightZ * moveX * 5.0f;
    m_velocity.y = moveY * 5.0f;

    // 移動量を反映
    m_pos += m_velocity;

    // 左右の傾き
    float targetAngle = -moveX * 25.0f;
    m_bankAngle += (targetAngle - m_bankAngle) * 0.15f;

    // 上下の傾き
    float targetPitch = -moveY * 25.0f;
    m_pitchAngle += (targetPitch - m_pitchAngle) * 0.15f;
}

Vector3 PlayerMove::GetPosition() const
{
    return m_pos;
}

// 開始地点（0, 0, 0）から自機までの距離を取得
float PlayerMove::GetDistanceFromStart() const
{
    float distance = sqrtf(
        m_pos.x * m_pos.x +
        m_pos.y * m_pos.y +
        m_pos.z * m_pos.z
    );

    return distance;
}

Vector3 PlayerMove::GetForward() const
{
    Quaternion rot = GetRotation();
    Vector3 forward = Vector3::Front;
    rot.Apply(forward);
    return forward;
}

Vector3 PlayerMove::GetRight() const
{
    float rad = m_yawAngle * 3.14159265f / 180.0f;
    return Vector3(cosf(rad), 0.0f, -sinf(rad));
}

Quaternion PlayerMove::GetRotation() const
{
    Quaternion rot;
    rot.SetRotationDegY(m_yawAngle);

    Quaternion bankRot;
    bankRot.SetRotationDegZ(m_bankAngle);

    Quaternion pitchRot;
    pitchRot.SetRotationDegX(m_pitchAngle);

    rot *= bankRot;
    rot *= pitchRot;

    return rot;
}

float PlayerMove::GetYawAngle() const
{
    return m_yawAngle;
}

Vector3 PlayerMove::GetVelocity() const
{
    return m_velocity;
}