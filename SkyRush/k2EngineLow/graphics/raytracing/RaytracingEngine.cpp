#include "k2EngineLowPreCompile.h"

#include "RaytracingEngine.h"
#include "graphics/Material.h"

namespace nsK2EngineLow {
	namespace raytracing {
		using namespace std;


		// GPU用バッファを作成する関数
		ID3D12Resource* CreateBuffer(
			ID3D12Device5* pDevice,
			uint64_t size,
			D3D12_RESOURCE_FLAGS flags,
			D3D12_RESOURCE_STATES initState,
			const D3D12_HEAP_PROPERTIES& heapProps)
		{
			// バッファの設定
			D3D12_RESOURCE_DESC bufDesc = {};
			bufDesc.Alignment = 0;
			bufDesc.DepthOrArraySize = 1;
			bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			bufDesc.Flags = flags;
			bufDesc.Format = DXGI_FORMAT_UNKNOWN;
			bufDesc.Height = 1;
			bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			bufDesc.MipLevels = 1;
			bufDesc.SampleDesc.Count = 1;
			bufDesc.SampleDesc.Quality = 0;
			bufDesc.Width = size;

			// GPUバッファのポインタを初期化
			ID3D12Resource* pBuffer = nullptr;

			// GPUバッファを生成する
			HRESULT hr = pDevice->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&bufDesc,
				initState,
				nullptr,
				IID_PPV_ARGS(&pBuffer)
			);

			// 生成に失敗した場合はエラーコードを出力する
			if (FAILED(hr)) {

				// GPUデバイスが切り離された原因を取得する
				HRESULT removedReason = pDevice->GetDeviceRemovedReason();

				char message[256];

				sprintf_s(
					message,
					"CreateBuffer HRESULT: 0x%08X\nDeviceRemovedReason: 0x%08X\n",
					static_cast<unsigned int>(hr),
					static_cast<unsigned int>(removedReason)
				);

				OutputDebugStringA(message);
				return nullptr;
			}

			return pBuffer;
		}

		void Engine::Init(const InitData& initData)
		{
			auto d3dDevice = g_graphicsEngine->GetD3DDevice();

			// レイトレの結果の出力先のテクスチャを作成。
			D3D12_RESOURCE_DESC resDesc = {};
			resDesc.DepthOrArraySize = 1;
			resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			resDesc.Format = initData.m_outputColorBufferFormat;
			resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			resDesc.Height = g_graphicsEngine->GetFrameBufferHeight();
			resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resDesc.MipLevels = 1;
			resDesc.SampleDesc.Count = 1;
			resDesc.Width = g_graphicsEngine->GetFrameBufferWidth();
			m_outputResource.Init(resDesc);
			m_outputTexture.InitFromD3DResource(m_outputResource.Get());

			//レイジェネレーション用の定数バッファ。
			Camera cam;
			cam.pos = g_camera3D->GetPosition();
			cam.mViewProjInv = g_camera3D->GetViewProjectionMatrixInv();
			cam.aspect = g_camera3D->GetAspect();
			cam.fNear = g_camera3D->GetNear();
			cam.fFar = g_camera3D->GetFar();
			// レイトレエンジン側でダブルバッファにしているので、内部ではダブルバッファにしない。
			m_rayGenerationCB[0].Init(sizeof(Camera), &cam, false);
			m_rayGenerationCB[1].Init(sizeof(Camera), &cam, false);

			for (int bufferNo = 0; bufferNo < 2; bufferNo++) {
				m_expandSRV[bufferNo] = std::make_unique< ExpanadSRV>();
				m_expandSRV[bufferNo]->Init(
					initData.m_expandShaderResource,
					initData.m_expandShaderResourceSize
				);
			}
			
		}
		void Engine::CommitRegistGeometry(RenderContext& rc)
		{
			if (!m_isDirty) {
				return;
			}

			m_world.CommitRegistGeometry(rc);
			
			for (int i = 0; i < 2; i++) {
				// 各種リソースをディスクリプタヒープに登録する。
				m_descriptorHeaps[i].Init(
					i, 
					m_world, 
					m_outputResource, 
					m_rayGenerationCB[i], 
					m_skycubeBox,
					m_expandSRV[i]->m_structuredBuffer
				);
				// PSOを作成。
				m_pipelineStateObject[i].Init(m_descriptorHeaps[i]);
				// シェーダーテーブルを作成。
				m_shaderTable[i].Init(i, m_world, m_pipelineStateObject[i], m_descriptorHeaps[i]); 
			}
			
			// ダーティフラグをオフにする。
			m_isDirty = false;
		}
		void Engine::Dispatch(RenderContext& rc)
		{
			// GPUデバイスの状態を確認する
			HRESULT hr = g_graphicsEngine->GetD3DDevice()->GetDeviceRemovedReason();

			char message[128];
			sprintf_s(
				message,
				"Dispatch Entry Device Status: 0x%08X\n",
				static_cast<unsigned int>(hr)
			);
			OutputDebugStringA(message);

			if (m_world.GetNumInstance() == 0) {
				// インスタンスが登録されていない。
				return;
			}
			
			CommitRegistGeometry(rc);

			int backBufferNo = g_graphicsEngine->GetBackBufferIndex();
			// レイトレワールド構築前のGPU状態を確認
			HRESULT beforeBuild = g_graphicsEngine->GetD3DDevice()->GetDeviceRemovedReason();

			char buildmessage[128];
			sprintf_s(
				buildmessage,
				"Before World Build: 0x%08X\n",
				static_cast<unsigned int>(beforeBuild)
			);
			OutputDebugStringA(buildmessage);

			// レイトレワールドを構築する
			m_world.Build(rc);

			// レイトレワールド構築後のGPU状態を確認
			HRESULT afterBuild = g_graphicsEngine->GetD3DDevice()->GetDeviceRemovedReason();

			sprintf_s(
				buildmessage,
				"After World Build: 0x%08X\n",
				static_cast<unsigned int>(afterBuild)
			);
			OutputDebugStringA(buildmessage);

			// カメラを更新。
			Camera cam;
			cam.pos = g_camera3D->GetPosition();
			cam.mViewProjInv = g_camera3D->GetViewProjectionMatrixInv();
			cam.aspect = g_camera3D->GetAspect();
			cam.fNear = g_camera3D->GetNear();
			cam.fFar = g_camera3D->GetFar();
			m_rayGenerationCB[backBufferNo].CopyToVRAM(cam);

			// 拡張ストラクチャードバッファを更新。
			m_expandSRV[backBufferNo]->m_structuredBuffer.Update(m_expandSRV[backBufferNo]->m_srcData);

			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource = m_outputResource.Get();
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			rc.ResourceBarrier(barrier);

			D3D12_DISPATCH_RAYS_DESC raytraceDesc = {};
			raytraceDesc.Width = g_graphicsEngine->GetFrameBufferWidth();
			raytraceDesc.Height = g_graphicsEngine->GetFrameBufferHeight();
			raytraceDesc.Depth = 1;


			auto shaderTableEntrySize = m_shaderTable[backBufferNo].GetShaderTableEntrySize();
			auto numRayGenShader = m_shaderTable[backBufferNo].GetNumRayGenShader();
			auto numMissShader = m_shaderTable[backBufferNo].GetNumMissShader();
			auto numHitShader = m_shaderTable[backBufferNo].GetNumHitShader();

			// レイ生成シェーダーのシェーダーテーブルの開始アドレスとサイズを設定。
			raytraceDesc.RayGenerationShaderRecord.StartAddress = m_shaderTable[backBufferNo].GetGPUVirtualAddress();
			raytraceDesc.RayGenerationShaderRecord.SizeInBytes = shaderTableEntrySize;

			// ミスシェーダーのシェーダーテーブルの開始アドレスとサイズを設定。
			size_t missOffset = numRayGenShader * shaderTableEntrySize;
			raytraceDesc.MissShaderTable.StartAddress = m_shaderTable[backBufferNo].GetGPUVirtualAddress() + missOffset;
			raytraceDesc.MissShaderTable.StrideInBytes = shaderTableEntrySize;
			raytraceDesc.MissShaderTable.SizeInBytes = shaderTableEntrySize * numMissShader;

			// ヒットグループシェーダーの開始アドレスとサイズを設定。
			size_t hitOffset = (numRayGenShader + numMissShader) * shaderTableEntrySize;
			raytraceDesc.HitGroupTable.StartAddress = m_shaderTable[backBufferNo].GetGPUVirtualAddress() + hitOffset;
			raytraceDesc.HitGroupTable.StrideInBytes = shaderTableEntrySize;
			raytraceDesc.HitGroupTable.SizeInBytes = shaderTableEntrySize * numHitShader * m_world.GetNumInstance();

			// グローバルルートシグネチャを設定。
			rc.SetComputeRootSignature(m_pipelineStateObject[backBufferNo].GetGlobalRootSignature());

			// Dispatch
			//グローバルルートシグネチチャに登録されているディスクリプタヒープを登録する。
			const DescriptorHeap* descriptorHeaps[] = {
				&m_descriptorHeaps[backBufferNo].GetSrvUavCbvDescriptorHeap(),
				&m_descriptorHeaps[backBufferNo].GetSamplerDescriptorHeap()
			};
			rc.SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);

			rc.SetPipelineState(m_pipelineStateObject[backBufferNo]);
			rc.DispatchRays(raytraceDesc);

			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource = m_outputResource.Get();
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
			rc.ResourceBarrier(barrier);
		}
	}//namespace raytracing
}//namespace nsK2EngineLow 