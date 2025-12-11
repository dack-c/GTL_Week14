#include "pch.h"

#include <d2d1_1.h>
#include <dwrite.h>
#include <dwrite_3.h>
#include <dxgi1_2.h>

#include "StatsOverlayD2D.h"
#include "UIManager.h"
#include "MemoryManager.h"
#include "Picking.h"
#include "PlatformTime.h"
#include "DecalStatManager.h"
#include "TileCullingStats.h"
#include "LightStats.h"
#include "ShadowStats.h"
#include "SkinningStats.h"
#include "Source/Runtime/Engine/Particle/ParticleStats.h"

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

static inline void SafeRelease(IUnknown* p) { if (p) p->Release(); }

UStatsOverlayD2D& UStatsOverlayD2D::Get()
{
	static UStatsOverlayD2D Instance;
	return Instance;
}

void UStatsOverlayD2D::Initialize(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, IDXGISwapChain* InSwapChain)
{
	D3DDevice = InDevice;
	D3DContext = InContext;
	SwapChain = InSwapChain;
	bInitialized = (D3DDevice && D3DContext && SwapChain);

	
	if (!bInitialized)
	{
		return;
	}

	D2D1_FACTORY_OPTIONS FactoryOpts{};
#ifdef _DEBUG
	FactoryOpts.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
	if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), &FactoryOpts, (void**)&D2DFactory)))
	{
		return;
	}

	IDXGIDevice* DxgiDevice = nullptr;
	if (FAILED(D3DDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&DxgiDevice)))
	{
		return;
	}

	if (FAILED(D2DFactory->CreateDevice(DxgiDevice, &D2DDevice)))
	{
		SafeRelease(DxgiDevice);
		return;
	}
	SafeRelease(DxgiDevice);

	if (FAILED(D2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &D2DContext)))
	{
		return;
	}

	if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&DWriteFactory)))
	{
		return;
	}

	if (DWriteFactory)
	{
		DWriteFactory->CreateTextFormat(
			L"Segoe UI",
			nullptr,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			16.0f,
			L"en-us",
			&TextFormat);

		if (TextFormat)
		{
			TextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
			TextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		}

		DWriteFactory->CreateTextFormat(
			L"Segoe UI",
			nullptr,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			16.0f,
			L"en-us",
			&UITextFormat);

		if (UITextFormat)
		{
			UITextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
			UITextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		}
	}

	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&WICFactory)
	);

	EnsureInitialized();

	LoadFontsFromDirectory("Data/UI/Fonts");
}

void UStatsOverlayD2D::Shutdown()
{
	ReleaseD2DResources();

	D3DDevice = nullptr;
	D3DContext = nullptr;
	SwapChain = nullptr;
	bInitialized = false;
}

void UStatsOverlayD2D::EnsureInitialized()
{
	if (!D2DContext)
	{
		return;
	}

	SafeRelease(BrushYellow);
	SafeRelease(BrushSkyBlue);
	SafeRelease(BrushLightGreen);
	SafeRelease(BrushOrange);
	SafeRelease(BrushCyan);
	SafeRelease(BrushViolet);
	SafeRelease(BrushDeepPink);
	SafeRelease(BrushBlack);

	SafeRelease(UIColorBrush);

	D2DContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &BrushYellow);
	D2DContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::SkyBlue), &BrushSkyBlue);
	D2DContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightGreen), &BrushLightGreen);
	D2DContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Orange), &BrushOrange);
	D2DContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Cyan), &BrushCyan);
	D2DContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Violet), &BrushViolet);
	D2DContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DeepPink), &BrushDeepPink);
	D2DContext->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0, 0.6f), &BrushBlack);
	D2DContext->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0, 0.0f), &UIColorBrush);

}

void UStatsOverlayD2D::ReleaseD2DResources()
{
	if (BitmapMap.size() > 0)
	{
		for (ID2D1Bitmap* Bitmap : BitmapMap.GetValues())
		{
			Bitmap->Release();
		}
	}
	SafeRelease(WICFactory);

	SafeRelease(BrushBlack);
	SafeRelease(BrushDeepPink);
	SafeRelease(BrushViolet);
	SafeRelease(BrushCyan);
	SafeRelease(BrushOrange);
	SafeRelease(BrushLightGreen);
	SafeRelease(BrushSkyBlue);
	SafeRelease(BrushYellow);

	SafeRelease(UIColorBrush);
	SafeRelease(UITextFormat);

	SafeRelease(CustomFontCollection);
	SafeRelease(TextFormat);
	SafeRelease(DWriteFactory);
	SafeRelease(D2DContext);
	SafeRelease(D2DDevice);
	SafeRelease(D2DFactory);

}

void UStatsOverlayD2D::ReadBitmap(const FString& FilePath)
{
	ReadBitmap(UTF8ToWide(FilePath));
}
void UStatsOverlayD2D::ReadBitmap(const FWideString& FilePath)
{
	if (BitmapMap.Contains(FilePath))
	{
		return;
	}

	FWideString NormalizedPath = NormalizePath(FilePath);

	if (!std::filesystem::exists(NormalizedPath))
	{
		return;
	}

	ID2D1Bitmap* bitmap = nullptr;

	// PNG/JPG를 WIC으로 읽기
	IWICBitmapDecoder* decoder = nullptr;
	WICFactory->CreateDecoderFromFilename(
		NormalizedPath.c_str(), nullptr,
		GENERIC_READ, WICDecodeMetadataCacheOnLoad,
		&decoder
	);

	// 프레임 가져오기
	IWICBitmapFrameDecode* frame = nullptr;
	decoder->GetFrame(0, &frame);

	// **WIC Format Converter**
	IWICFormatConverter* converter = nullptr;
	WICFactory->CreateFormatConverter(&converter);

	// 프레임을 32bit PBGRA로 변환
	converter->Initialize(
		frame,
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		nullptr,
		0.0f,
		WICBitmapPaletteTypeCustom
	);

	// D2D 비트맵 생성
	HRESULT hr = D2DContext->CreateBitmapFromWicBitmap(
		converter,
		nullptr,
		&bitmap
	);

	if (FAILED(hr))
	{
		// 디버그 로그 넣으면 도움됨
		// wprintf(L"Failed to create bitmap. HR = 0x%x\n", hr);
	}

	// 메모리 해제
	converter->Release();
	frame->Release();
	decoder->Release();

	BitmapMap[FilePath] = bitmap;
}
void UStatsOverlayD2D::DrawOnlyText(const wchar_t* InText, const D2D1_RECT_F& InRect, const FVector4& Color, const float FontSize, const wchar_t* FontName)
{
	static FWideString CachedFontName = L"";
	static float CachedFontSize = 0.0f;

	if (FontSize != CachedFontSize || wcscmp(FontName, CachedFontName.c_str()) != 0)
	{
		SafeRelease(UITextFormat);

		IDWriteFontCollection* FontCollection = CustomFontCollection ? CustomFontCollection : nullptr;

		DWriteFactory->CreateTextFormat(
			FontName,
			FontCollection,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			FontSize,
			L"en-us",
			&UITextFormat);

		if (UITextFormat)
		{
			UITextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			UITextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		}

		CachedFontName = FontName;
		CachedFontSize = FontSize;
	}
	UIColorBrush->SetColor(D2D1_COLOR_F(Color.X, Color.Y, Color.Z, Color.W));
	D2DContext->DrawTextW(
		InText,
		static_cast<UINT32>(wcslen(InText)),
		UITextFormat,
		InRect,
		UIColorBrush);
}

void UStatsOverlayD2D::DrawBitmap(const D2D1_RECT_F& InRect, const FString& FilePath, const float Opacity) const
{
	DrawBitmap(InRect, UTF8ToWide(FilePath), Opacity);
}
void UStatsOverlayD2D::DrawBitmap(const D2D1_RECT_F& InRect, const FWideString& FilePath, const float Opacity) const
{
	if (BitmapMap.Contains(FilePath))
	{
		D2DContext->DrawBitmap(BitmapMap.at(FilePath), InRect, Opacity);
	}
}

static void DrawTextBlock(
	ID2D1DeviceContext* InD2dCtx,
	IDWriteTextFormat* InTextFormat,
	const wchar_t* InText,
	const D2D1_RECT_F& InRect,
	ID2D1SolidColorBrush* InBgBrush,
	ID2D1SolidColorBrush* InTextBrush)
{
	if (!InD2dCtx || !InTextFormat || !InText || !InBgBrush || !InTextBrush)
	{
		return;
	}

	InD2dCtx->FillRectangle(InRect, InBgBrush);
	InD2dCtx->DrawTextW(
		InText,
		static_cast<UINT32>(wcslen(InText)),
		InTextFormat,
		InRect,
		InTextBrush);
}

void UStatsOverlayD2D::Draw()
{
	if (!bInitialized || !SwapChain)
	{
		return;
	}

	if (!D2DContext || !TextFormat)
	{
		return;
	}

	IDXGISurface* Surface = nullptr;
	if (FAILED(SwapChain->GetBuffer(0, __uuidof(IDXGISurface), (void**)&Surface)))
	{
		return;
	}

	D2D1_BITMAP_PROPERTIES1 BmpProps = {};
	BmpProps.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	BmpProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	BmpProps.dpiX = 96.0f;
	BmpProps.dpiY = 96.0f;
	BmpProps.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

	ID2D1Bitmap1* TargetBmp = nullptr;
	if (FAILED(D2DContext->CreateBitmapFromDxgiSurface(Surface, &BmpProps, &TargetBmp)))
	{
		SafeRelease(Surface);
		return;
	}

	D2DContext->SetTarget(TargetBmp);

	D2DContext->BeginDraw();


	D3D11_VIEWPORT Viewport;
	UINT NumViewports = 1;

	D3DContext->RSGetViewports(&NumViewports, &Viewport);

	ViewportLTop = FVector2D(Viewport.TopLeftX, Viewport.TopLeftY);
	ViewportSize = FVector2D(Viewport.Width, Viewport.Height);

	std::sort(DrawInfose.begin(), DrawInfose.end(),
		[](const FDrawInfo* A, const FDrawInfo* B)
		{
			return A->RectTransform.ZOrder < B->RectTransform.ZOrder;
		});
	for (FDrawInfo* Info : DrawInfose)
	{
		Info->DrawUI();
		delete Info;
	}
	DrawInfose.clear();


	const float Margin = 12.0f;
	const float Space = 8.0f;   // 패널간의 간격
	const float PanelWidth = 250.0f;
	const float PanelHeight = 48.0f;
	float NextY = 70.0f;

	if (bShowFPS)
	{
		float Dt = UUIManager::GetInstance().GetDeltaTime();
		float Fps = Dt > 0.0f ? (1.0f / Dt) : 0.0f;
		float Ms = Dt * 1000.0f;

		wchar_t Buf[128];
		swprintf_s(Buf, L"FPS: %.1f\nFrame time: %.2f ms", Fps, Ms);

		D2D1_RECT_F rc = D2D1::RectF(Margin, NextY, Margin + PanelWidth, NextY + PanelHeight);
		DrawTextBlock(D2DContext, TextFormat, Buf, rc, BrushBlack, BrushYellow);

		NextY += PanelHeight + Space;
	}

	if (bShowPicking)
	{
		wchar_t Buf[256];
		double LastMs = FWindowsPlatformTime::ToMilliseconds(CPickingSystem::GetLastPickTime());
		double TotalMs = FWindowsPlatformTime::ToMilliseconds(CPickingSystem::GetTotalPickTime());
		uint32 Count = CPickingSystem::GetPickCount();
		double AvgMs = (Count > 0) ? (TotalMs / (double)Count) : 0.0;
		swprintf_s(Buf, L"Pick Count: %u\nLast: %.3f ms\nAvg: %.3f ms\nTotal: %.3f ms", Count, LastMs, AvgMs, TotalMs);

		const float PickPanelHeight = 96.0f;
		D2D1_RECT_F rc = D2D1::RectF(Margin, NextY, Margin + PanelWidth, NextY + PickPanelHeight);
		DrawTextBlock(D2DContext, TextFormat, Buf, rc, BrushBlack, BrushSkyBlue);

		NextY += PickPanelHeight + Space;
	}

	if (bShowMemory)
	{
		double Mb = static_cast<double>(FMemoryManager::TotalAllocationBytes) / (1024.0 * 1024.0);

		wchar_t Buf[128];
		swprintf_s(Buf, L"Memory: %.1f MB\nAllocs: %u", Mb, FMemoryManager::TotalAllocationCount);

		D2D1_RECT_F Rc = D2D1::RectF(Margin, NextY, Margin + PanelWidth, NextY + PanelHeight);
		DrawTextBlock(D2DContext, TextFormat, Buf, Rc, BrushBlack, BrushLightGreen);

		NextY += PanelHeight + Space;
	}

	if (bShowDecal)
	{
		// 1. FDecalStatManager로부터 통계 데이터를 가져옵니다.
		uint32_t TotalCount = FDecalStatManager::GetInstance().GetTotalDecalCount();
		//uint32_t VisibleDecalCount = FDecalStatManager::GetInstance().GetVisibleDecalCount();
		uint32_t AffectedMeshCount = FDecalStatManager::GetInstance().GetAffectedMeshCount();
		double TotalTime = FDecalStatManager::GetInstance().GetDecalPassTimeMS();
		double AverageTimePerDecal = FDecalStatManager::GetInstance().GetAverageTimePerDecalMS();
		double AverageTimePerDraw = FDecalStatManager::GetInstance().GetAverageTimePerDrawMS();

		// 2. 출력할 문자열 버퍼를 만듭니다.
		wchar_t Buf[256];
		swprintf_s(Buf, L"[Decal Stats]\nTotal: %u\nAffectedMesh: %u\n전체 소요 시간: %.3f ms\nAvg/Decal: %.3f ms\nAvg/Mesh: %.3f ms",
			TotalCount,
			AffectedMeshCount,
			TotalTime,
			AverageTimePerDecal,
			AverageTimePerDraw);

		// 3. 텍스트를 여러 줄 표시해야 하므로 패널 높이를 늘립니다.
		const float decalPanelHeight = 140.0f;
		D2D1_RECT_F rc = D2D1::RectF(Margin, NextY, Margin + PanelWidth, NextY + decalPanelHeight);

		DrawTextBlock(D2DContext, TextFormat, Buf, rc, BrushBlack, BrushOrange);

		NextY += decalPanelHeight + Space;
	}

	if (bShowTileCulling)
	{
		const FTileCullingStats& TileStats = FTileCullingStatManager::GetInstance().GetStats();

		wchar_t Buf[512];
		swprintf_s(Buf, L"[Tile Culling Stats]\nTiles: %u x %u (%u)\nLights: %u (P:%u S:%u)\nMin/Avg/Max: %u / %.1f / %u\nCulling Eff: %.1f%%\nBuffer: %u KB",
			TileStats.TileCountX,
			TileStats.TileCountY,
			TileStats.TotalTileCount,
			TileStats.TotalLights,
			TileStats.TotalPointLights,
			TileStats.TotalSpotLights,
			TileStats.MinLightsPerTile,
			TileStats.AvgLightsPerTile,
			TileStats.MaxLightsPerTile,
			TileStats.CullingEfficiency,
			TileStats.LightIndexBufferSizeBytes / 1024);

		const float tilePanelHeight = 160.0f;
		D2D1_RECT_F rc = D2D1::RectF(Margin, NextY, Margin + PanelWidth, NextY + tilePanelHeight);
		DrawTextBlock(D2DContext, TextFormat, Buf, rc, BrushBlack, BrushCyan);

		NextY += tilePanelHeight + Space;
	}

	if (bShowLights)
	{
		const FLightStats& LightStats = FLightStatManager::GetInstance().GetStats();

		wchar_t Buf[512];
		swprintf_s(Buf, L"[Light Stats]\nTotal Lights: %u\n  Point: %u\n  Spot: %u\n  Directional: %u\n  Ambient: %u",
			LightStats.TotalLights,
			LightStats.TotalPointLights,
			LightStats.TotalSpotLights,
			LightStats.TotalDirectionalLights,
			LightStats.TotalAmbientLights);

		const float lightPanelHeight = 140.0f;
		D2D1_RECT_F rc = D2D1::RectF(Margin, NextY, Margin + PanelWidth, NextY + lightPanelHeight);
		DrawTextBlock(D2DContext, TextFormat, Buf, rc, BrushBlack, BrushViolet);

		NextY += lightPanelHeight + Space;
	}

	if (bShowShadow)
	{
		const FShadowStats& ShadowStats = FShadowStatManager::GetInstance().GetStats();

		wchar_t Buf[512];
		swprintf_s(Buf, L"[Shadow Stats]\nShadow Lights: %u\n  Point: %u\n  Spot: %u\n  Directional: %u\n\nAtlas 2D: %u x %u (%.1f MB)\nAtlas Cube: %u x %u x %u (%.1f MB)\n\nTotal Memory: %.1f MB",
			ShadowStats.TotalShadowCastingLights,
			ShadowStats.ShadowCastingPointLights,
			ShadowStats.ShadowCastingSpotLights,
			ShadowStats.ShadowCastingDirectionalLights,
			ShadowStats.ShadowAtlas2DSize,
			ShadowStats.ShadowAtlas2DSize,
			ShadowStats.ShadowAtlas2DMemoryMB,
			ShadowStats.ShadowAtlasCubeSize,
			ShadowStats.ShadowAtlasCubeSize,
			ShadowStats.ShadowCubeArrayCount,
			ShadowStats.ShadowAtlasCubeMemoryMB,
			ShadowStats.TotalShadowMemoryMB);

		const float shadowPanelHeight = 260.0f;
		D2D1_RECT_F rc = D2D1::RectF(Margin, NextY, Margin + PanelWidth, NextY + shadowPanelHeight);
		DrawTextBlock(D2DContext, TextFormat, Buf, rc, BrushBlack, BrushDeepPink);

		NextY += shadowPanelHeight + Space;

		rc = D2D1::RectF(Margin, NextY, Margin + PanelWidth, NextY + 40);
		DrawTextBlock(D2DContext, TextFormat, FScopeCycleCounter::GetTimeProfile("ShadowMapPass").GetConstWChar_tWithKey("ShadowMapPass"), rc, BrushBlack, BrushDeepPink);

		NextY += shadowPanelHeight + Space;
	}
	
	if (bShowSkinning)
	{		
		// GPU 스키닝
		double GPUSkinning = GET_GPU_STAT("GPUSkinning")
		// CPU 스키닝
		double CPUSkinning = FScopeCycleCounter::GetTimeProfile("CPUSkinning").GetTime();
		double VertexBuffer = FScopeCycleCounter::GetTimeProfile("VertexBuffer").GetTime();
		double StructuredBuffer = FScopeCycleCounter::GetTimeProfile("StructuredBuffer").GetTime();
		double SkeletalAABB = FScopeCycleCounter::GetTimeProfile("SkeletalAABB").GetTime();

		const FSkinningStats& SkinningStats = FSkinningStatManager::GetInstance().GetStats();
		FWideString AllSkinningType = UTF8ToWide(SkinningStats.SkinningType);		
		wchar_t Buf[512];
		swprintf_s(
			Buf,
			L"[Skeletal Stats]\n All Skinning Type : %s\n Total Skeletals : %u\n Total Bones : %u\n Total Vertices : %u\n"
			L"[Times]\n"
			L" CPU Skinning : %.3f\n"
			L" Vertex Buffer : %.3f\n"
			L" GPU Draw Time : %.3f\n"
			L" Structured Buffer : %.3f\n"
			L" AABB : %.3f\n",
			AllSkinningType.c_str(),
			SkinningStats.TotalSkeletals,
			SkinningStats.TotalBones,
			SkinningStats.TotalVertices,
			CPUSkinning,
			VertexBuffer,
			GPUSkinning,
			StructuredBuffer,
			SkeletalAABB
		);

		const float SkinningPanelHeight = 180.0f;
		D2D1_RECT_F rc = D2D1::RectF(Margin, NextY, Margin + PanelWidth + 50.0f, NextY + SkinningPanelHeight);
		DrawTextBlock(D2DContext, TextFormat, Buf, rc, BrushBlack, BrushDeepPink);
		NextY += SkinningPanelHeight + Space;		
	}
	
	if (bShowParticle)
	{		
		const FParticleStats& ParticleStats = FParticleStatManager::GetInstance().GetStats();
    
		double SimulationTime = FScopeCycleCounter::GetTimeProfile("Particle_Simulation").GetTime();
		double CollectBatchesTime = FScopeCycleCounter::GetTimeProfile("Particle_CollectBatches").GetTime();
		double GPUDrawTime = FGPUProfiler::GetInstance().GetStat("Particle_Draw");

		wchar_t Buf[512];
		swprintf_s(
		   Buf,
		   L"[Particle Stats]\n"
		   L" Active Particles : %u\n"       // uint32
		   L" Draw Calls       : %u\n"       // uint32
		   L"[Times (ms)]\n"
		   L" Simulation (CPU) : %.3f\n"     // double (Tick)
		   L" Collect Batches (CPU): %.3f\n"     // double (CollectBatches/Sort/Map)
		   L" GPU Draw Time    : %.3f\n",    // double
       
		   ParticleStats.TotalActiveParticles,
		   ParticleStats.DrawCalls,
		   SimulationTime,
		   CollectBatchesTime,
		   GPUDrawTime
		);

		constexpr float ParticlePanelHeight = 160.0f;
		D2D1_RECT_F rc = D2D1::RectF(Margin, NextY, Margin + PanelWidth + 50.0f, NextY + ParticlePanelHeight);
		DrawTextBlock(D2DContext, TextFormat, Buf, rc, BrushBlack, BrushCyan);
		NextY += ParticlePanelHeight + Space;		
	}
	D2DContext->EndDraw();
	D2DContext->SetTarget(nullptr);

	FParticleStatManager::GetInstance().ResetStats();
	FScopeCycleCounter::TimeProfileInit();

	SafeRelease(TargetBmp);
	SafeRelease(Surface);
}




void UStatsOverlayD2D::RegisterTextUI(const FRectTransform& InRectTransform, const FString& Text, const FVector4& Color, const float InFontSize, const FString& InFontName)
{
	DrawInfose.Push(new FDrawInfoText(InRectTransform, Text, Color, InFontSize, InFontName));
}

void UStatsOverlayD2D::RegisterSpriteUI(const FRectTransform& InRectTransform, const FString& FilePath, const float Opacity)
{
	ReadBitmap(FilePath);
	DrawInfose.Push(new FDrawInfoSprite(InRectTransform, FilePath, Opacity));
}

void UStatsOverlayD2D::LoadFontsFromDirectory(const FString& DirectoryPath)
{
	FWideString WidePath = UTF8ToWide(DirectoryPath);
	FWideString NormalizedPath = NormalizePath(WidePath);

	if (!std::filesystem::exists(NormalizedPath) || !std::filesystem::is_directory(NormalizedPath))
	{
		UE_LOG("[UI][Font] Font directory not found: %s\n", DirectoryPath.c_str());
		return;
	}

	TArray<FWideString> FontFiles;

	for (const auto& Entry : std::filesystem::directory_iterator(NormalizedPath))
	{
		if (Entry.is_regular_file())
		{
			FWideString Extension = Entry.path().extension().wstring();
			std::transform(Extension.begin(), Extension.end(), Extension.begin(), ::tolower);

			if (Extension == L".ttf" || Extension == L".otf")
			{
				FontFiles.Push(Entry.path().wstring());
			}
		}
	}

	if (FontFiles.empty() || !DWriteFactory)
	{
		return;
	}

	IDWriteFactory3* Factory3 = nullptr;
	HRESULT hr = DWriteFactory->QueryInterface(__uuidof(IDWriteFactory3), (void**)&Factory3);
	if (FAILED(hr))
	{
		UE_LOG("[UI][Font] DirectWrite 3.0 not available, using system fonts only\n");
		return;
	}

	IDWriteFontSetBuilder* FontSetBuilder = nullptr;
	hr = Factory3->CreateFontSetBuilder(&FontSetBuilder);
	if (FAILED(hr))
	{
		SafeRelease(Factory3);
		return;
	}

	for (const FWideString& FontPath : FontFiles)
	{
		IDWriteFontFile* FontFile = nullptr;
		hr = Factory3->CreateFontFileReference(FontPath.c_str(), nullptr, &FontFile);
		if (SUCCEEDED(hr))
		{
			IDWriteFontFaceReference* FontFaceRef = nullptr;
			hr = Factory3->CreateFontFaceReference(
				FontFile,
				0,
				DWRITE_FONT_SIMULATIONS_NONE,
				&FontFaceRef
			);
			if (SUCCEEDED(hr))
			{
				hr = FontSetBuilder->AddFontFaceReference(FontFaceRef);
				if (SUCCEEDED(hr))
				{
					UE_LOG("[UI][Font] Added to DirectWrite collection: %s\n", WideToUTF8(FontPath).c_str());
				}
				SafeRelease(FontFaceRef);
			}
			SafeRelease(FontFile);
		}
	}

	IDWriteFontSet* FontSet = nullptr;
	hr = FontSetBuilder->CreateFontSet(&FontSet);
	if (SUCCEEDED(hr))
	{
		SafeRelease(CustomFontCollection);
		hr = Factory3->CreateFontCollectionFromFontSet(FontSet, &CustomFontCollection);
		if (SUCCEEDED(hr))
		{
			UE_LOG("[UI][Font] DirectWrite custom font collection created successfully\n");
		}
		SafeRelease(FontSet);
	}

	SafeRelease(FontSetBuilder);
	SafeRelease(Factory3);
}