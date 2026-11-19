#include "stdafx.h"
#include "Enemy.h"
#include "cstdlib"
#include "ctime"
#include "sound/SoundEngine.h"
#include "sound/SoundSource.h"

using namespace nsK2EngineLow;

void Enemy::Init(const Vector3& pos)
{
    m_modelRender.Init("Assets/modelData/NormalEnemy.tkm");
    // 敵モデルを2倍に拡大
    m_modelRender.SetScale(Vector3(2.0f, 2.0f, 2.0f));

    m_pos = pos;
    m_hp = m_maxHP;

    m_isCharging = false;
    static bool randomInitialized = false;
    if (!randomInitialized)
    {
        srand((unsigned int)time(nullptr));
        randomInitialized = true;
    }

    m_attackTimer = rand() % 180 + 120;
    m_chargeTimer = 0;
    m_modelRender.SetPosition(m_pos);

    Quaternion rot;
    rot.SetRotationDegY(180.0f);
    m_modelRender.SetRotation(rot);

    m_isCharging = false;
    m_hasDamaged = false;
    m_chargeStarted = false;

    m_isWarning = false;
    m_warningTimer = 0;
    m_savedPlayerPos = Vector3::Zero;
    m_chargeDirection = Vector3::Zero;
}

void Enemy::Update(const Vector3& playerPos)
{
    m_chargeStarted = false;

    if (IsDead())
        return;

    Vector3 diff = m_pos - playerPos;
    float distance = diff.Length();

    // ① 突進中
    if (m_isCharging)
    {
        m_pos += m_chargeDirection * (m_moveSpeed * 3.5f);

        m_chargeTimer--;

        if (m_chargeTimer <= 0)
        {
            m_isCharging = false;
            m_attackTimer = rand() % 180 + 120;
        }
    }
    // ② 攻撃予告中
    else if (m_isWarning)
    {
        m_warningTimer--;

        // 予告終了後、保存地点に向かって突進
        if (m_warningTimer <= 0)
        {
            m_isWarning = false;
            m_isCharging = true;
            m_chargeTimer = 30;

            m_chargeDirection = m_savedPlayerPos - m_pos;
            m_chargeDirection.Normalize();

            m_chargeStarted = true;
            m_hasDamaged = false;
        }
    }
    // ③ 通常時
    else
    {
        m_attackTimer--;

        if (m_attackTimer <= 0)
        {
            // 攻撃開始時のプレイヤー座標を保存
            m_savedPlayerPos = playerPos;

            // 予告開始
            m_isWarning = true;
            m_warningTimer = 45;
        }
        else
        {
            // 通常時は一定距離を維持
            if (distance > m_keepDistance + 10.0f)
            {
                diff.Normalize();
                m_pos -= diff * m_moveSpeed;
            }
            else if (distance < m_keepDistance - 10.0f)
            {
                diff.Normalize();
                m_pos += diff * m_moveSpeed;
            }
        }
    }

    m_modelRender.SetPosition(m_pos);
    m_modelRender.Update();
}

void Enemy::Render(RenderContext& rc)
{
    if (!IsDead())
    {
        m_modelRender.Draw(rc);
    }
}

void Enemy::Damage(int damage)
{
    if (IsDead())
        return;

    m_hp -= damage;

    if (m_hp <= 0)
    {
        m_hp = 0;

        // 敵撃破音を再生
        auto soundSource = NewGO<SoundSource>(0);
        soundSource->Init(1);
        soundSource->Play(false);
    }
}

Vector3 Enemy::GetPosition() const
{
    return m_pos;
}

bool Enemy::IsDead() const
{
    return m_hp <= 0;
}

void Enemy::SetPosition(const Vector3& pos)
{
    m_pos = pos;
    m_modelRender.SetPosition(m_pos);
}

// 突進開始フラグを取得
bool Enemy::IsChargeStarted() const
{
    return m_chargeStarted;
}

// ダメージを与えたか確認
bool Enemy::HasDamaged() const
{
    return m_hasDamaged;
}

// ダメージ判定の状態を変更
void Enemy::SetHasDamaged(bool hasDamaged)
{
    m_hasDamaged = hasDamaged;
}

// 攻撃予告中か取得
bool Enemy::IsWarning() const
{
    return m_isWarning;
}

// 保存したプレイヤー座標を取得
Vector3 Enemy::GetSavedPlayerPos() const
{
    return m_savedPlayerPos;
}