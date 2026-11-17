#pragma once

class PlayerHP
{
public:
    void Init();
    void Damage(int damage);

    int GetHP() const;
    bool IsDead() const;

private:
    int m_maxHP = 350;
    int m_hp = 350;
};