#include "stdafx.h"
#include "PlayerHP.h"

void PlayerHP::Init()
{
    m_hp = m_maxHP;
}

void PlayerHP::Damage(int damage)
{
    if (m_hp <= 0)
        return;

    m_hp -= damage;

    if (m_hp < 0)
        m_hp = 0;
}

int PlayerHP::GetHP() const
{
    return m_hp;
}

bool PlayerHP::IsDead() const
{
    return m_hp <= 0;
}
