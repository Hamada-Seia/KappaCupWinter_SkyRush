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
    m_modelRender.SetScale(Vector3(1.0f, 1.0f, 1.0f));

    m_pos = pos;

    // 初期位置を帰還先として保存
    m_homePos = pos;

    m_hp = m_maxHP;

    m_isCharging = false;

    // 突進・帰還状態をリセット
    m_isCharging = false;
    m_isReturning = false;

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

void Enemy::Update(const Vector3& playerPos, const Vector3& playerForward)
{
    m_chargeStarted = false;

    if (IsDead())
        return;

    // ① 初期位置へ帰還中
    if (m_isReturning)
    {
        // 初期位置までの方向と距離
        Vector3 toHome = m_homePos - m_pos;
        float distance = toHome.Length();

        // 初期位置に到着したら待機状態へ
        if (distance <= m_moveSpeed * 3.5f)
        {
            m_pos = m_homePos;
            m_isReturning = false;

            // 次の攻撃までの時間を設定
            m_attackTimer = rand() % 180 + 120;
        }
        else
        {
            // 初期位置へ戻る
            toHome.Normalize();
            m_pos += toHome * (m_moveSpeed * 3.5f);
        }
    }
    // ② 突進中
    else if (m_isCharging)
    {
        // 保存したプレイヤー位置へ突進
        m_pos += m_chargeDirection * (m_moveSpeed * 3.5f);

        m_chargeTimer--;

        // 突進終了
        if (m_chargeTimer <= 0)
        {
            m_isCharging = false;
            m_isReturning = true;
        }
    }
    // ③ 攻撃予告中
    else if (m_isWarning)
    {
        m_warningTimer--;

        // 予告終了後、保存地点へ突進
        if (m_warningTimer <= 0)
        {
            m_isWarning = false;
            m_isCharging = true;
            m_chargeTimer = 30;

            // 保存したプレイヤー位置への方向を計算
            m_chargeDirection = m_savedPlayerPos - m_pos;

            // 方向を正規化
            if (m_chargeDirection.Length() > 0.001f)
            {
                m_chargeDirection.Normalize();
            }
            else
            {
                m_chargeDirection = Vector3::Front;
            }

            // 突進開始を知らせる
            m_chargeStarted = true;

            // 今回の突進のダメージ判定をリセット
            m_hasDamaged = false;
        }
    }
    // ④ 通常待機中
    else
    {
        m_attackTimer--;

        if (m_attackTimer <= 0)
        {
            // 攻撃開始時のプレイヤー座標を保存
            m_savedPlayerPos = playerPos;

            // 攻撃予告を開始
            m_isWarning = true;
            m_warningTimer = 45;
        }
    }

    // 敵モデルの位置を更新
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