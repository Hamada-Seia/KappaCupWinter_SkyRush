#include "k2EngineLowPreCompile.h"

#include "EffectEngine.h"

namespace nsK2EngineLow {
	EffectEngine* EffectEngine::m_instance = nullptr;	//唯一のインスタンス。

	EffectEngine::EffectEngine()
	{
		Effekseer::SetLogger([](Effekseer::LogType type, const std::string& message) {
			OutputDebugStringA(message.c_str());
			OutputDebugStringA("\n");
			});

		K2_ASSERT(
			m_instance == nullptr,
			"EffectEngineのインスタンスを複数作ることはできません。"
		);
		//auto format = DXGI_FORMAT_R8G8B8A8_UNORM;
		auto format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		auto d3dDevice = g_graphicsEngine->GetD3DDevice();
		auto commandQueue = g_graphicsEngine->GetCommandQueue();
		for (int i = 0; i < 2; i++) {
			// レンダラーを作成。
			m_renderer[i] = ::EffekseerRendererDX12::Create(
				d3dDevice,
				commandQueue,
				2,
				&format,
				1,
				DXGI_FORMAT_D32_FLOAT,
				false,
				8000
			);
			//メモリプールの作成。
			m_memoryPool[i] = EffekseerRenderer::CreateSingleFrameMemoryPool(m_renderer[i]->GetGraphicsDevice());
			// コマンドリストの作成
			m_commandList[i] = EffekseerRenderer::CreateCommandList(m_renderer[i]->GetGraphicsDevice(), m_memoryPool[i]);
		}

		// Rendererが正常に作られているか確認
		if (m_renderer[0] == nullptr) {
			OutputDebugStringA("ERROR: Renderer is nullptr!\n");
			return;
		}

		OutputDebugStringA("Renderer OK\n");

		// エフェクトマネージャーの作成。
		m_manager = ::Effekseer::Manager::Create(8000);

		// Managerが正常に作られたか確認
		if (m_manager == nullptr) {
			OutputDebugStringA("ERROR: Manager Create failed!\n");
			return;
		}

		OutputDebugStringA("Manager Create OK\n");

		
		m_manager->SetCurveLoader(Effekseer::MakeRefPtr<Effekseer::CurveLoader>());

		// Game::Start() で ResistEffect を呼んでも読み込めるよう、ローダーをここでも設定する
		//m_manager->SetTextureLoader(m_renderer[0]->CreateTextureLoader());
		//m_manager->SetModelLoader(m_renderer[0]->CreateModelLoader());
		//m_manager->SetMaterialLoader(m_renderer[0]->CreateMaterialLoader());

		OutputDebugStringA("EffectEngine Initialize Finished\n");

	}
	Effekseer::EffectRef EffectEngine::LoadEffect(const int number)
	{
		Effekseer::EffectRef effect;
		auto it = m_effectMap.find(number);
		if (it != m_effectMap.end()) {
			//ロード済み。
			effect = it->second;
		}
		else {
			//ロードできない。
			//ResistEffectでエフェクト読み込んでね！
			std::abort();
		}
		return effect;
	}
	EffectEngine::~EffectEngine()
	{

	}
	int EffectEngine::Play(Effekseer::EffectRef effect)
	{
		return m_manager->Play(effect, 0, 0, 0);
	}
	void EffectEngine::Stop(int effectHandle)
	{
		m_manager->StopEffect(effectHandle);
	}


	void EffectEngine::Update(float deltaTime)
	{
		int backBufferNo = g_graphicsEngine->GetBackBufferIndex();
		// Begin a command list
		// コマンドリストを開始する。
		EffekseerRendererDX12::BeginCommandList(m_commandList[backBufferNo], g_graphicsEngine->GetCommandList());
		m_renderer[backBufferNo]->SetCommandList(m_commandList[backBufferNo]);

		m_manager->Update();

		//レンダラーにカメラ行列を設定。
		m_renderer[backBufferNo]->SetCameraMatrix(*(const Effekseer::Matrix44*)&g_camera3D->GetViewMatrix());
		//レンダラーにプロジェクション行列を設定。
		m_renderer[backBufferNo]->SetProjectionMatrix(*(const Effekseer::Matrix44*)&g_camera3D->GetProjectionMatrix());

		m_renderer[backBufferNo]->SetTime(deltaTime);
	}

	void EffectEngine::BeginFrame()
	{
		int backBufferNo = g_graphicsEngine->GetBackBufferIndex();
		m_memoryPool[backBufferNo]->NewFrame();
		// 描画モジュールの設定。
		m_manager->SetSpriteRenderer(m_renderer[backBufferNo]->CreateSpriteRenderer());
		m_manager->SetRibbonRenderer(m_renderer[backBufferNo]->CreateRibbonRenderer());
		m_manager->SetRingRenderer(m_renderer[backBufferNo]->CreateRingRenderer());
		m_manager->SetTrackRenderer(m_renderer[backBufferNo]->CreateTrackRenderer());
		m_manager->SetModelRenderer(m_renderer[backBufferNo]->CreateModelRenderer());

		// ローダーの設定。
		m_manager->SetTextureLoader(m_renderer[backBufferNo]->CreateTextureLoader());
		m_manager->SetModelLoader(m_renderer[backBufferNo]->CreateModelLoader());
		m_manager->SetMaterialLoader(m_renderer[backBufferNo]->CreateMaterialLoader());
	}
	void EffectEngine::Draw()
	{
		int backBufferNo = g_graphicsEngine->GetBackBufferIndex();
		// Begin to rendering effects
		// エフェクトの描画開始処理を行う。
		m_renderer[backBufferNo]->BeginRendering();

		// Render effects
		// エフェクトの描画を行う。
		m_manager->Draw();

		// Finish to rendering effects
		// エフェクトの描画終了処理を行う。
		m_renderer[backBufferNo]->EndRendering();

		// Finish a command list
		// コマンドリストを終了する。
		m_renderer[backBufferNo]->SetCommandList(nullptr);
		EffekseerRendererDX12::EndCommandList(m_commandList[backBufferNo]);
	}

	//void EffectEngine::ResistEffect(const int number, const char16_t* filePath)
	//{
	//	Effekseer::EffectRef effect;
	//	auto it = m_effectMap.find(number);
	//	if (it == m_effectMap.end()) {
	//		//新規。
	//		effect = Effekseer::Effect::Create(m_manager, filePath);
	//		m_effectMap.insert({ number, effect });
	//	}
	//}

	void EffectEngine::ResistEffect(const int number, const char16_t* filePath)
	{
		// Managerが生成されているか確認
		if (m_manager == nullptr) {
			OutputDebugStringA("ERROR: m_manager is nullptr!\n");
			return;
		}

		// ファイルパスが空でないか確認
		if (filePath == nullptr) {
			OutputDebugStringA("ERROR: filePath is nullptr!\n");
			return;
		}

		OutputDebugStringA("Manager OK\n");
		OutputDebugStringA("\n");

		Effekseer::EffectRef effect;
		auto it = m_effectMap.find(number);

		if (it == m_effectMap.end()) {

			// エフェクト読み込み開始の確認
			OutputDebugStringA("Effect Create Start\n");

			// ManagerのSettingを取得
			auto setting = m_manager->GetSetting();

			if (setting == nullptr) {
				OutputDebugStringA("ERROR: Setting is nullptr!\n");
				return;
			}

			OutputDebugStringA("Setting OK\n");

			// EffectLoaderを確認
			auto effectLoader = setting->GetEffectLoader();

			if (effectLoader == nullptr) {
				OutputDebugStringA("ERROR: EffectLoader is nullptr!\n");
				return;
			}

			OutputDebugStringA("EffectLoader OK\n");

			//effect = Effekseer::Effect::Create(m_manager, filePath);

			// EffectLoader単体でファイルを読み込めるか確認
			void* data = nullptr;
			int32_t size = 0;

			OutputDebugStringA("Effect Load Start\n");

			bool loadResult = effectLoader->Load(filePath, data, size);

			if (!loadResult) {
				OutputDebugStringA("ERROR: EffectLoader Load failed!\n");
				return;
			}

			OutputDebugStringA("EffectLoader Load OK\n");

			// 読み込んだデータの状態を確認
			char message[256];
			sprintf_s(
				message,
				"Effect data: address=%p, size=%d\n",
				data,
				size
			);
			OutputDebugStringA(message);

			// データが空なら生成処理を行わない
			if (data == nullptr || size <= 0) {
				OutputDebugStringA("ERROR: Effect data is invalid!\n");
				effectLoader->Unload(data, size);
				return;
			}

			// 読み込んだデータからエフェクトを生成
			effect = Effekseer::Effect::Create(m_manager, data, size);

			if (effect == nullptr) {
				OutputDebugStringA("ERROR: Create from data failed!\n");
			}
			else {
				OutputDebugStringA("Create from data OK!\n");
			}

			// 読み込んだデータを解放
			effectLoader->Unload(data, size);

			if (effect == nullptr) {
				return;
			}

			// エフェクトを登録
			m_effectMap.insert({ number, effect });


			if (effect == nullptr) {
				OutputDebugStringA("ERROR: Effect::Create failed!\n");
				return;
			}

			m_effectMap.insert({ number, effect });
		}
	}
}