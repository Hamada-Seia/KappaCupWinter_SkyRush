
#pragma once

#include "Level3DRender/LevelRender.h"

class Bullets
{
public:
    static const int MAX_BULLETS = 30;

    void Init();
    void Update();
    void Render(RenderContext& rc);

    void Shot(
        const Vector3& pos,
        const Vector3& aimTarget,
        const Vector3& right,
        const Vector3& forward,
        const Vector3& velocity,
        float yawAngle
    );

    bool IsActive(int index) const;
    Vector3 GetPosition(int index) const;
    void Hit(int index);

private:
    ModelRender m_bulletRender[MAX_BULLETS];
    Vector3 m_bulletPos[MAX_BULLETS];
    Vector3 m_bulletDir[MAX_BULLETS];
    Vector3 m_bulletInheritedVelocity[MAX_BULLETS];

    bool m_bulletActive[MAX_BULLETS] = {};
    int m_bulletLife[MAX_BULLETS] = {};

    int m_nextBullet = 0;
};


