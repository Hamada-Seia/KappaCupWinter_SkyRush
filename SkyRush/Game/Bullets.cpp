
#include "stdafx.h"
#include "Bullets.h"

using namespace nsK2EngineLow;

// 初期化
void Bullets::Init()
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        m_bulletRender[i].Init("Assets/modelData/YB.tkm");
        m_bulletActive[i] = false;
    }
}

// 弾の発射


void Bullets::Shot(
    const Vector3& pos,
    const Vector3& aimTarget,
    const Vector3& right,
    const Vector3& forward,
    const Vector3& velocity,
    float yawAngle)
{
    for (int shot = 0; shot < 2; shot++) {
        int i = (m_nextBullet + shot) % MAX_BULLETS;

        // 機体の前方へ移動してから左右の銃口位置を設定
        m_bulletPos[i] = pos + forward * 25.0f;

        if (shot == 0)
            m_bulletPos[i] -= right * 6.0f;
        else
            m_bulletPos[i] += right * 6.0f;

        // それぞれの銃口から同じ照準点へ向かう
        m_bulletDir[i] = aimTarget - m_bulletPos[i];
        m_bulletDir[i].Normalize();
        m_bulletInheritedVelocity[i] = velocity;
 
        m_bulletActive[i] = true;
        m_bulletLife[i] = 60;
        m_bulletRender[i].SetPosition(m_bulletPos[i]);

        Quaternion bulletRot;
        bulletRot.SetRotationDegY(yawAngle);
        m_bulletRender[i].SetRotation(bulletRot);
    }



    m_nextBullet = (m_nextBullet + 2) % MAX_BULLETS;

}



// 弾の更新
void Bullets::Update()
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!m_bulletActive[i])
            continue;

        Vector3 bulletMove =
            m_bulletDir[i] * 30.0f + m_bulletInheritedVelocity[i];

        m_bulletPos[i] += bulletMove;

        m_bulletLife[i]--;

        if (m_bulletLife[i] <= 0)
        {
            m_bulletActive[i] = false;
            continue;
        }

        m_bulletRender[i].SetPosition(m_bulletPos[i]);
        m_bulletRender[i].Update();
    }
}

// 描画
void Bullets::Render(RenderContext& rc)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (m_bulletActive[i])
        {
            m_bulletRender[i].Draw(rc);
        }
    }
}

// 弾が有効か確認
bool Bullets::IsActive(int index) const
{
    return m_bulletActive[index];
}

// 弾の位置を取得
Vector3 Bullets::GetPosition(int index) const
{
    return m_bulletPos[index];
}

// 弾を消す
void Bullets::Hit(int index)
{
    m_bulletActive[index] = false;
}
