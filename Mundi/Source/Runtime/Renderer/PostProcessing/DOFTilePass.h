#pragma once
#include "PostProcessing.h"

// DOF Tile 연산을 위한 Compute Shader 패스
// Tile Flatten: 8x8 타일별 Min/Max CoC 계산
// Tile Dilate: 이웃 타일로 Min/Max 확장
class FDOFTilePass
{
public:
    FDOFTilePass() = default;
    ~FDOFTilePass();

    // 리소스 해제
    void Release();

    // 타일 연산 실행 (내부에서 초기화 및 SRV 가져옴)
    void Execute(D3D11RHI* RHIDevice);

    // 결과 SRV 반환 (Blur에서 사용)
    ID3D11ShaderResourceView* GetDilatedTileSRV() const { return DilatedTileSRV; }

    // 타일 크기 정보
    UINT GetTileCountX() const { return TileCountX; }
    UINT GetTileCountY() const { return TileCountY; }

    bool IsInitialized() const { return bInitialized; }

private:
    // 리소스 초기화 (Execute에서 자동 호출)
    void Initialize(D3D11RHI* RHIDevice, UINT InputWidth, UINT InputHeight);

    static const UINT TILE_SIZE = 8;

    bool bInitialized = false;

    UINT InputWidth = 0;
    UINT InputHeight = 0;
    UINT TileCountX = 0;
    UINT TileCountY = 0;

    // Flatten 출력 (타일 텍스처)
    ID3D11Texture2D* FlattenedTileTexture = nullptr;
    ID3D11ShaderResourceView* FlattenedTileSRV = nullptr;
    ID3D11UnorderedAccessView* FlattenedTileUAV = nullptr;

    // Dilate 출력
    ID3D11Texture2D* DilatedTileTexture = nullptr;
    ID3D11ShaderResourceView* DilatedTileSRV = nullptr;
    ID3D11UnorderedAccessView* DilatedTileUAV = nullptr;

    // Constant Buffer
    ID3D11Buffer* TileConstantBuffer = nullptr;

    void CreateResources(D3D11RHI* RHIDevice);
    void ReleaseResources();
};
