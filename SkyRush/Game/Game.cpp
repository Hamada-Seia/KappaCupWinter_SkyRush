#include "stdafx.h"
#include "Windows.h"
#include "Game.h"
#include "sound/SoundEngine.h"
#include "sound/SoundSource.h"

using namespace nsK2EngineLow;

bool Game::Start()
{
    // 射撃音
    g_soundEngine->ResistWaveFileBank(0, "Assets/Shader/sound/Shot.wav");
    // 敵撃破音
    g_soundEngine->ResistWaveFileBank(1, "Assets/Shader/Sound/Gekiha.wav");
    // 乱気流エリアの警告音
    g_soundEngine->ResistWaveFileBank(2, "Assets/Shader/Sound/Keikoku.wav");
    // 自機のエンジン音
    g_soundEngine->ResistWaveFileBank(3, "Assets/Shader/Sound/Engine.wav");
    // 敵の突進音
    g_soundEngine->ResistWaveFileBank(4, "Assets/Shader/Sound/EneAtta.wav");

    //戦闘機
    m_modelRender.Init("Assets/modelData/fighter.tkm");

    //海面
    m_seaRender.Init("Assets/modelData/Kaimen.tkm");

    // 危険区域用の紫色の海面
    m_purpleSeaRender.Init("Assets/modelData/PKaimen.tkm");

    // レティクル
    m_reticle.Init("Assets/modelData/rtkr_green.DDS", 110.0f, 90.0f);

    // エイムボックス
    m_ab.Init("Assets/modelData/ab_green.DDS", 350.0f, 300.0f);

    // 画面
    m_titleUI.Init("Assets/modelData/Title.DDS", 1920.0f, 1080.0f);
    m_clearUI.Init("Assets/modelData/GameClear.DDS", 1920.0f, 1080.0f);
    m_gameOverUI.Init("Assets/modelData/GameOver.DDS", 1920.0f, 1080.0f);

    // HPバーの背景（中央基準）
    m_hpBase.Init("Assets/modelData/HP.DDS", 400.0f, 32.0f);
    m_hpBase.SetPosition(Vector3(-700.0f, -450.0f, 0.0f));

    // HPのラベル
    m_hpLabel.SetText(L"HP");
    m_hpLabel.SetPosition(-900.0f, -390.0f, 0.0f);
    m_hpLabel.SetScale(1.2f);
    m_hpLabel.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    // HPの数値
    m_hpNumber.SetText(L"350 / 350");
    m_hpNumber.SetPosition(-500.0f, -430.0f, 0.0f);
    m_hpNumber.SetScale(1.0f);
    m_hpNumber.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    // HPゲージ
    m_hpGauge.Init("Assets/modelData/HPgauge.DDS", 400.0f, 32.0f);

    // 左端を基準にする
    m_hpGauge.SetPivot(Vector2(0.0f, 0.5f));

    // 背景の左端に合わせる
    m_hpGauge.SetPosition(Vector3(-900.0f, -450.0f, 0.0f));

    // HEATバーの背景
    m_heatBase.Init("Assets/modelData/HP.DDS", 400.0f, 32.0f);
    m_heatBase.SetPosition(Vector3(550.0f, -450.0f, 0.0f));

    // HEATのラベル
    m_heatLabel.SetText(L"HEAT");
    m_heatLabel.SetPosition(350.0f, -390.0f, 0.0f);
    m_heatLabel.SetScale(1.2f);
    m_heatLabel.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    // HEATの数値
    m_heatNumber.SetText(L"0 / 100");
    m_heatNumber.SetPosition(750.0f, -430.0f, 0.0f);
    m_heatNumber.SetScale(1.0f);
    m_heatNumber.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    // HEATゲージ本体
    m_heatGauge.Init("Assets/modelData/HPgauge.DDS", 400.0f, 32.0f);

    // 右端を基準にする
    m_heatGauge.SetPivot(Vector2(1.0f, 0.5f));

    // 背景の右端に合わせる
    m_heatGauge.SetPosition(Vector3(750.0f, -450.0f, 0.0f));

    // 飛行範囲の警告文
    m_boundaryWarning.SetText(L"");
    m_boundaryWarning.SetPosition(-300.0f, 180.0f, 0.0f);
    m_boundaryWarning.SetScale(1.5f);
    m_boundaryWarning.SetColor(1.0f, 1.0f, 0.0f, 1.0f);

    // 自機の移動初期化
    m_playerMove.Init();

    m_modelRender.SetPosition(m_playerMove.GetPosition());

    // 弾の初期化
    m_bullets.Init();

    // 海面の位置
    m_seaRender.SetPosition(0.0f, -5.0f, 0.0f);
    // 海面の大きさ
    m_seaRender.SetScale(Vector3(200.0f, 1.0f, 200.0f));

    // 紫色の海面も通常の海面と同じ位置・大きさにする
    m_purpleSeaRender.SetPosition(0.0f, -5.0f, 0.0f);
    m_purpleSeaRender.SetScale(Vector3(200.0f, 1.0f, 200.0f));

    //PlayerHPの初期化
    m_playerHP.Init();

    //Enemyの初期化
    m_enemy.Init(Vector3(0.0f, 0.0f, 300.0f));
    m_defeatedEnemyCount = 0;
    m_gameState = GameState::Title;

    // 警告範囲の状態をリセット
    m_wasInWarningArea = false;

    // 自機のエンジン音源を作成
    m_engineSound = NewGO<SoundSource>(0);
    m_engineSound->Init(3);

    // エンジン音の音量
    m_engineSound->SetVolume(1.6f);

    // エンジン噴射エフェクトを登録
    //EffectEngine::GetInstance()->ResistEffect(0,u"Assets/effect/EngineEffect.efkefc");

    // 敵撃破エフェクトを登録
    //EffectEngine::GetInstance()->ResistEffect(1, u"Assets/effect/ExplosionEffect.efkefc");

    return true;
}

void Game::Update()


{
    // タイトル画面
    if (m_gameState == GameState::Title)
    {
        bool startGame = false;

        if (GetAsyncKeyState(VK_RETURN) & 0x8000)
            startGame = true;

        if (g_pad[0] != nullptr)
        {
            if (g_pad[0]->IsPress(enButtonA))
                startGame = true;
        }

        if (startGame)
        {
            m_playerMove.Init();
            m_playerHP.Init();
            m_bullets.Init();
            m_enemy.Init(Vector3(0.0f, 0.0f, 200.0f));

            m_defeatedEnemyCount = 0;
            m_shotCoolTime = 0;

            // 飛行範囲ダメージのタイマーをリセット
            m_boundaryDamageTimer = 0;

            // 警告範囲の状態をリセット
            m_wasInWarningArea = false;

            // オーバーヒート状態をリセット
            m_heat = 0.0f;
            m_isOverheated = false;
            m_overheatTimer = 0;

            // エンジン音をループ再生
            m_engineSound->Play(true);
            m_gameState = GameState::Playing;
        }

        return;
    }


    // クリア・ゲームオーバー画面
    if (m_gameState == GameState::Clear ||
        m_gameState == GameState::GameOver)
    {
        // タイトルに戻る入力
        bool returnToTitle = false;

        // Spaceキー
        if (GetAsyncKeyState(VK_SPACE) & 0x8000)
        {
            returnToTitle = true;
        }

        // Enterキー
        if (GetAsyncKeyState(VK_RETURN) & 0x8000)
        {
            returnToTitle = true;
        }

        // XboxコントローラーのAボタン
        if (g_pad[0] != nullptr)
        {
            if (g_pad[0]->IsPress(enButtonA))
            {
                returnToTitle = true;
            }
        }

        // タイトル画面へ戻る
        if (returnToTitle)
        {
            m_gameState = GameState::Title;
        }

        return;
    }


    // ゲーム終了後は更新しない
    if (m_gameState != GameState::Playing)
    {
        return;
    }

    // 自機が撃墜されたらゲームオーバー
    if (m_playerHP.IsDead())
    {
        m_engineSound->Stop();
        m_gameState = GameState::GameOver;
        return;
    }

    // HP残量の割合を計算
    float hpRate = (float)m_playerHP.GetHP() / 350.0f;

    // HP残量に合わせてゲージの横幅を変更
    m_hpGauge.SetScale(Vector3(hpRate, 1.0f, 1.0f));

    // HP30%以下なら赤、それ以外は緑
    if (hpRate <= 0.3f)
    {
        m_hpGauge.SetMulColor(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
    }
    else
    {
        m_hpGauge.SetMulColor(Vector4(0.0f, 1.0f, 0.0f, 1.0f));
    }
    // HPゲージの更新
    m_hpBase.Update();
    m_hpGauge.Update();

    // 現在のHPを取得
    int currentHP = m_playerHP.GetHP();

    // HPの数値を文字列に変換
    wchar_t hpText[64];
    swprintf_s(hpText, L"%d / 350", currentHP);

    // HP表示を更新
    m_hpNumber.SetText(hpText);


    // 現在の熱量の割合を計算
    float heatRate = m_heat / MAX_HEAT;

    // 熱量に合わせてゲージの横幅を変更
    m_heatGauge.SetScale(Vector3(heatRate, 1.0f, 1.0f));

    // 通常はオレンジ、オーバーヒート中は赤
    if (m_isOverheated)
    {
        m_heatGauge.SetMulColor(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
    }
    else
    {
        m_heatGauge.SetMulColor(Vector4(1.0f, 0.5f, 0.0f, 1.0f));
    }

    // HEATゲージの更新
    m_heatBase.Update();
    m_heatGauge.Update();

    // 現在の熱量を整数にする
    int currentHeat = (int)m_heat;

    // HEAT数値を文字列に変換
    wchar_t heatText[64];
    swprintf_s(heatText, L"%d / 100", currentHeat);

    // HEAT表示を更新
    m_heatNumber.SetText(heatText);

    // 自機が撃墜されていない場合だけ移動
    if (!m_playerHP.IsDead())
    {
        m_playerMove.Update();
    }


// 飛行範囲制限
// 自機の現在位置を取得
    Vector3 boundaryPlayerPos = m_playerMove.GetPosition();

    // X方向・Z方向それぞれの海面の端までの残り距離
    float remainingX = SEA_HALF_X - fabsf(boundaryPlayerPos.x);
    float remainingZ = SEA_HALF_Z - fabsf(boundaryPlayerPos.z);

    // 四辺のうち、最も近い端までの距離を取得
    float remainingDistance = remainingX;

    if (remainingZ < remainingDistance)
    {
        remainingDistance = remainingZ;
    }

    // 継続ダメージ範囲に入ったら紫色の海面に切り替える
    m_isPurpleSea = (remainingDistance <= DAMAGE_REMAINING);

    // 警告範囲に入ったか判定
    bool isInWarningArea = (remainingDistance <= WARNING_REMAINING);

    // 警告範囲に入った瞬間だけ警告音を再生
    if (isInWarningArea && !m_wasInWarningArea)
    {
        auto soundSource = NewGO<SoundSource>(0);
        soundSource->Init(2);
        soundSource->Play(false);
    }

    // 現在の警告範囲の状態を保存
    m_wasInWarningArea = isInWarningArea;

    // ゲームオーバー
    if (remainingDistance <= GAMEOVER_REMAINING)
    {
        m_gameState = GameState::GameOver;
        return;
    }

    // 継続ダメージ
    if (remainingDistance <= DAMAGE_REMAINING)
    {
        m_boundaryDamageTimer++;

        if (m_boundaryDamageTimer >= BOUNDARY_DAMAGE_INTERVAL)
        {
            m_playerHP.Damage(BOUNDARY_DAMAGE);
            m_boundaryDamageTimer = 0;
        }
    }
    else
    {
        // ダメージ範囲から戻ったらタイマーをリセット
        m_boundaryDamageTimer = 0;
    }


    // 海面の内側へ戻る方向を調べる
    Vector3 inwardDirection = Vector3::Zero;

    // 左右の端と前後の端、どちらが近いか判定
    if (remainingX < remainingZ)
    {
        // X方向の端が近い
        inwardDirection.x =
            (boundaryPlayerPos.x > 0.0f) ? -1.0f : 1.0f;
    }
    else
    {
        // Z方向の端が近い
        inwardDirection.z =
            (boundaryPlayerPos.z > 0.0f) ? -1.0f : 1.0f;
    }

    // 自機の向きと右方向を取得
    Vector3 playerForward = m_playerMove.GetForward();
    Vector3 playerRight = m_playerMove.GetRight();

    // 戻る方向が機首の前後どちらか判定
    float forwardDot =
        inwardDirection.x * playerForward.x +
        inwardDirection.z * playerForward.z;

    // 戻る方向が自機から見て左右どちらか判定
    float rightDot =
        inwardDirection.x * playerRight.x +
        inwardDirection.z * playerRight.z;

    // 表示する方向を決定
    const wchar_t* directionText;

    if (forwardDot > 0.7f)
    {
        directionText = L"KEEP COURSE";
    }
    else if (forwardDot < -0.7f)
    {
        directionText = L"TURN AROUND";
    }
    else if (rightDot > 0.0f)
    {
        directionText = L"TURN RIGHT";
    }
    else
    {
        directionText = L"TURN LEFT";
    }

    // 飛行範囲の警告表示を切り替える
    if (remainingDistance <= DAMAGE_REMAINING)
    {
        // 継続ダメージ範囲
        wchar_t warningText[128];

        swprintf_s(warningText, L"DANGER: %ls", directionText);

        m_boundaryWarning.SetText(warningText);
        m_boundaryWarning.SetColor(1.0f, 0.0f, 0.0f, 1.0f);
    }
    else if (remainingDistance <= WARNING_REMAINING)
    {
        // 警告範囲
        wchar_t warningText[128];

        swprintf_s(warningText, L"WARNING: %ls", directionText);

        m_boundaryWarning.SetText(warningText);
        m_boundaryWarning.SetColor(1.0f, 1.0f, 0.0f, 1.0f);
    }
    else
    {
        // 通常範囲では警告を消す
        m_boundaryWarning.SetText(L"");
    }


    m_enemy.Update(m_playerMove.GetPosition());

    // 自機と敵の接触判定
    if (!m_enemy.IsDead() && !m_playerHP.IsDead())
    {
        Vector3 playerPos = m_playerMove.GetPosition();
        Vector3 enemyPos = m_enemy.GetPosition();

        Vector3 diff = playerPos - enemyPos;

        if (diff.Length() < 100.0f && !m_enemy.HasDamaged())
        {
            // 自機にダメージを1回与える
            m_playerHP.Damage(50);

            // 接触ダメージ音を再生
            auto soundSource = NewGO<SoundSource>(0);
            soundSource->Init(4);
            soundSource->Play(false);

            // この突進では再びダメージを与えない
            m_enemy.SetHasDamaged(true);
        }
    }

    m_modelRender.SetPosition(m_playerMove.GetPosition());
    m_modelRender.SetRotation(m_playerMove.GetRotation());
    m_modelRender.Update();

    // カメラの追従
    Vector3 playerPos = m_playerMove.GetPosition();
    Vector3 forward = m_playerMove.GetForward();

    Vector3 cameraPos = playerPos - forward * 75.0f;
    cameraPos.y += 38.0f;

    Vector3 cameraTarget = playerPos + forward * 20.0f;
    cameraTarget.y += 30.0f;

    g_camera3D->SetPosition(cameraPos);
    g_camera3D->SetTarget(cameraTarget);

    // カメラの行列を最新状態にする
    g_camera3D->GetViewProjectionMatrix();
    Vector2 enemyScreenPos;

    // 照準位置を敵の原点より上にする
    Vector3 enemyAimPos = m_enemy.GetPosition();
    enemyAimPos.y += 5.0f;

    g_camera3D->CalcScreenPositionFromWorldPosition(
        enemyScreenPos,
        enemyAimPos
    );

    // カメラの向きを計算
    Vector3 cameraForward = cameraTarget - cameraPos;

    // カメラから敵への方向
    Vector3 toEnemy = m_enemy.GetPosition() - cameraPos;

    // 敵がカメラの前にいるか判定
    float cameraDistance =
        cameraForward.x * toEnemy.x +
        cameraForward.y * toEnemy.y +
        cameraForward.z * toEnemy.z;

    // 敵がABの範囲内にいるか判定
    bool isLockedOn =
        !m_enemy.IsDead() &&
        cameraDistance > 0.0f &&
        enemyScreenPos.x >= -190.0f &&
        enemyScreenPos.x <= 160.0f &&
        enemyScreenPos.y >= -150.0f &&
        enemyScreenPos.y <= 150.0f;

    if (isLockedOn)
    {
        // 敵がABの範囲内
        m_reticle.SetPosition(Vector3(
            enemyScreenPos.x,
            enemyScreenPos.y,
            0.0f
        ));
    }
    else
    {
        // 敵がABの外にいる
        m_reticle.SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    }

    // オーバーヒート中の処理
    if (m_isOverheated)
    {
        m_overheatTimer--;

        // 停止時間が終わったら射撃可能にする
        if (m_overheatTimer <= 0)
        {
            m_isOverheated = false;
            m_heat = 0.0f;

            // 射撃間隔もリセットする
            m_shotCoolTime = 0;
        }
    }

    //プレイヤーが生きている場合だけ射撃
    if (!m_playerHP.IsDead())
    {
        // 射撃入力
        bool isAttack = false;

        if (GetAsyncKeyState(VK_SPACE) & 0x8000)
            isAttack = true;

        if (g_pad[0] != nullptr)
        {
            if (g_pad[0]->IsPress(enButtonRB2))
                isAttack = true;
        }

        // 射撃していない間は熱量を冷却
        if (!isAttack && !m_isOverheated)
        {
            m_heat -= COOL_SPEED;

            // 熱量がマイナスにならないようにする
            if (m_heat < 0.0f)
            {
                m_heat = 0.0f;
            }
        }

        // 射撃間隔
        if (m_shotCoolTime > 0)
        {
            m_shotCoolTime--;
        }

        // オーバーヒートしていない場合だけ射撃可能
        if (isAttack && m_shotCoolTime <= 0 && !m_isOverheated)
        {
            // 弾の発射位置
            Vector3 shotPos = m_playerMove.GetPosition();

            // 左右の砲門の位置関係
            Vector3 right = m_playerMove.GetRight();

            // 狙う場所
            Vector3 aimTarget;

            if (isLockedOn)
            {
                aimTarget = enemyAimPos;
            }
            else
            {
                aimTarget = g_camera3D->GetPosition()
                    + g_camera3D->GetForward() * 1000.0f;
            }

            // 弾を発射
            m_bullets.Shot(
                shotPos,
                aimTarget,
                right,
                m_playerMove.GetForward(),
                m_playerMove.GetVelocity(),
                m_playerMove.GetYawAngle()
            );

            // 射撃音を再生（左右の砲門で1回だけ）
            auto soundSource = NewGO<SoundSource>(0);
            soundSource->Init(0);
            soundSource->Play(false);


            // 射撃した分だけ熱量を増やす
            m_heat += HEAT_PER_SHOT;

            // 最大熱量に達したらオーバーヒート
            if (m_heat >= MAX_HEAT)
            {
                m_heat = MAX_HEAT;
                m_isOverheated = true;
                m_overheatTimer = OVERHEAT_TIME;
            }

            // 次の射撃までの時間
            m_shotCoolTime = 8;
        }


        // エイムボックスを少し左へ
        m_ab.SetPosition(Vector3(-10.0f, 0.0f, 0.0f));
        m_ab.Update();

        // UIの更新
        m_reticle.Update();
    }
    // 弾の更新
    m_bullets.Update();

    // 敵が生きている場合だけ当たり判定
    if (!m_enemy.IsDead())
    {
        for (int i = 0; i < Bullets::MAX_BULLETS; i++)
        {
            if (!m_bullets.IsActive(i))
                continue;

            Vector3 bulletPos = m_bullets.GetPosition(i);
            Vector3 enemyPos = m_enemy.GetPosition();

            Vector3 diff = bulletPos - enemyPos;

            float distance = diff.Length();

            if (distance < 20.0f)
            {
                //敵へのダメージ
                m_enemy.Damage(10);
                m_bullets.Hit(i);

                // 敵を撃墜したら撃墜数を増やす
                if (m_enemy.IsDead())
                {
                    m_defeatedEnemyCount++;
                    break;
                }
            }
        }
    }
    if (m_defeatedEnemyCount >= MAX_ENEMY_COUNT)
    {
        m_engineSound->Stop();
        m_gameState = GameState::Clear;
        return;
    }

    // 次の小敵を出現させる
    if (m_enemy.IsDead() && m_defeatedEnemyCount < MAX_ENEMY_COUNT)
    {
        m_enemy.Init(Vector3(0.0f, 0.0f, 200.0f));
    }


    // 海面モデルの更新
    m_seaRender.Update();
    m_purpleSeaRender.Update();


}

void Game::Render(RenderContext& rc)
{
    // タイトル画面
    if (m_gameState == GameState::Title)
    {
        m_titleUI.Draw(rc);
        return;
    }

    // クリア画面
    if (m_gameState == GameState::Clear)
    {
        m_clearUI.Draw(rc);
        return;
    }

    // ゲームオーバー画面
    if (m_gameState == GameState::GameOver)
    {
        m_gameOverUI.Draw(rc);
        return;
    }

    // ゲームプレイ中
    if (!m_playerHP.IsDead())
    {
        m_modelRender.Draw(rc);
    }
    // 危険区域かどうかで海面を切り替える
    if (m_isPurpleSea)
    {
        m_purpleSeaRender.Draw(rc);
    }
    else
    {
        m_seaRender.Draw(rc);
    }
    m_bullets.Render(rc);
    m_enemy.Render(rc);
    m_reticle.Draw(rc);
    m_ab.Draw(rc);

    // HPバーを描画
    m_hpBase.Draw(rc);
    m_hpGauge.Draw(rc);

    // HPの文字と数値を描画
    m_hpLabel.Draw(rc);
    m_hpNumber.Draw(rc);

    // HEATバーを描画
    m_heatBase.Draw(rc);
    m_heatGauge.Draw(rc);

    // HEATの文字と数値を描画
    m_heatLabel.Draw(rc);
    m_heatNumber.Draw(rc);

    // 飛行範囲の警告文を描画
    m_boundaryWarning.Draw(rc);
}