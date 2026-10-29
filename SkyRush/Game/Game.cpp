
#include "stdafx.h"
#include "Game.h"

bool Game::Start()
{
    m_modelRender.Init("Assets/modelData/fighter.tkm");

    // 自機（モデル）の初期位置
    m_pos = Vector3::Zero;
    m_modelRender.SetPosition(m_pos);

    return true;
}

void Game::Update()
{
    float stickX = g_pad[0]->GetLStickXF();

    m_pos.x += stickX * 5.0f;

    float targetAngle = -stickX * 25.0f;
    m_bankAngle += (targetAngle - m_bankAngle) * 0.15f;

    Quaternion rot;
    rot.SetRotationDegZ(m_bankAngle);

    m_modelRender.SetPosition(m_pos);
    m_modelRender.SetRotation(rot);
    m_modelRender.Update();
}

void Game::Render(RenderContext& rc)
{
    m_modelRender.Draw(rc);
}
