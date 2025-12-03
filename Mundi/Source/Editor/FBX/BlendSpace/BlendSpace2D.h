#pragma once
#include "Object.h"
#include "Source/Runtime/Engine/Animation/AnimTypes.h"
#include "Source/Runtime/Core/Math/Vector.h"

class UAnimSequence;

/**
 * @brief 2D Blend Space Sample Point
 */
struct FBlendSample2D
{
	UAnimSequence* Animation = nullptr;
	FVector2D Position;  // X: Direction, Y: Speed

	FBlendSample2D() = default;
	FBlendSample2D(UAnimSequence* InAnimation, float InX, float InY)
		: Animation(InAnimation), Position(InX, InY) {}
	FBlendSample2D(UAnimSequence* InAnimation, const FVector2D& InPosition)
		: Animation(InAnimation), Position(InPosition) {}
};

/**
 * @brief Triangle for Delaunay triangulation
 */
struct FBlendTriangle
{
	int32 Indices[3];  // Sample array indices (A, B, C)

	FBlendTriangle() : Indices{-1, -1, -1} {}
	FBlendTriangle(int32 A, int32 B, int32 C) : Indices{A, B, C} {}

	bool IsValid() const
	{
		return Indices[0] >= 0 && Indices[1] >= 0 && Indices[2] >= 0;
	}

	bool ContainsVertex(int32 VertexIndex) const
	{
		return Indices[0] == VertexIndex ||
		       Indices[1] == VertexIndex ||
		       Indices[2] == VertexIndex;
	}

	bool operator==(const FBlendTriangle& Other) const
	{
		return Indices[0] == Other.Indices[0] &&
		       Indices[1] == Other.Indices[1] &&
		       Indices[2] == Other.Indices[2];
	}
};

/**
 * @brief 2D Blend Space
 * - Blends multiple animations based on two parameters (X: Direction, Y: Speed)
 * - Uses Delaunay triangulation for dynamic sample placement
 * - Barycentric interpolation within triangles
 *
 * @example Usage
 *
 * // 1. Create BlendSpace2D
 * UBlendSpace2D* LocomotionBS = NewObject<UBlendSpace2D>();
 *
 * // 2. Add samples (9 directions)
 * LocomotionBS->AddSample(Idle,    0.0f,    0.0f);    // Center
 * LocomotionBS->AddSample(WalkF,   0.0f,  100.0f);    // Forward
 * LocomotionBS->AddSample(WalkB,   0.0f, -100.0f);    // Backward
 * LocomotionBS->AddSample(WalkL, -100.0f,   0.0f);    // Left
 * LocomotionBS->AddSample(WalkR,  100.0f,   0.0f);    // Right
 *
 * // 3. Update every frame
 * FVector2D InputDir = GetNormalizedInput() * 100.0f;
 * TArray<FTransform> OutputPose;
 * LocomotionBS->Update(InputDir.X, InputDir.Y, DeltaTime, OutputPose);
 */
class UBlendSpace2D : public UObject, public IAnimPoseProvider
{
	DECLARE_CLASS(UBlendSpace2D, UObject)

public:
	UBlendSpace2D() = default;
	virtual ~UBlendSpace2D() = default;

	// ============================================================
	// IAnimPoseProvider Interface
	// ============================================================

	virtual void EvaluatePose(float Time, float DeltaTime, TArray<FTransform>& OutPose) override;
	virtual float GetPlayLength() const override;
	virtual int32 GetNumBoneTracks() const override;
	virtual UAnimSequence* GetDominantSequence() const override { return DominantSequence; }
	virtual float GetCurrentPlayTime() const override { return CurrentPlayTime; }
	virtual float GetPreviousPlayTime() const override { return PreviousPlayTime; }

	// ============================================================
	// Parameter Setting
	// ============================================================

	void SetParameter(float InX, float InY);
	void SetParameter(const FVector2D& InParameter);
	FVector2D GetParameter() const { return CurrentParameter; }

	// ============================================================
	// Sample Management
	// ============================================================

	int32 AddSample(UAnimSequence* Animation, float X, float Y);
	void ClearSamples();
	int32 GetNumSamples() const { return Samples.Num(); }
	const TArray<FBlendSample2D>& GetSamples() const { return Samples; }
	const FBlendSample2D* GetSample(int32 Index) const;
	bool SetSamplePosition(int32 Index, float X, float Y);
	bool SetSampleAnimation(int32 Index, UAnimSequence* Animation);
	bool RemoveSample(int32 Index);

	// ============================================================
	// Triangulation
	// ============================================================

	const TArray<FBlendTriangle>& GetTriangles() const { return Triangles; }

	/** Delaunay 자동 삼각분할 */
	void Triangulate();

	/** 수동 삼각형 추가 */
	void AddTriangle(int32 A, int32 B, int32 C);

	/** 삼각형 제거 */
	bool RemoveTriangle(int32 Index);

	/** 모든 삼각형 제거 */
	void ClearTriangles();

	/** 삼각형 개수 */
	int32 GetNumTriangles() const { return Triangles.Num(); }

	// ============================================================
	// Parameter Range
	// ============================================================

	void SetParameterRange(const FVector2D& InMin, const FVector2D& InMax);
	FVector2D GetMinParameter() const { return MinParameter; }
	FVector2D GetMaxParameter() const { return MaxParameter; }

	// ============================================================
	// Evaluation
	// ============================================================

	void Update(float X, float Y, float DeltaTime, TArray<FTransform>& OutPose);
	void ResetPlayTime() { CurrentPlayTime = 0.0f; PreviousPlayTime = 0.0f; }

private:
	// Sample data
	TArray<FBlendSample2D> Samples;
	TArray<FBlendTriangle> Triangles;

	// Current state
	FVector2D CurrentParameter = FVector2D(0.0f, 0.0f);
	float CurrentPlayTime = 0.0f;
	float PreviousPlayTime = 0.0f;

	// Parameter range
	FVector2D MinParameter = FVector2D(-100.0f, -100.0f);
	FVector2D MaxParameter = FVector2D(100.0f, 100.0f);

	// Dominant sequence for notify
	UAnimSequence* DominantSequence = nullptr;

	// Dirty flag
	bool bTriangulationDirty = true;

	// ============================================================
	// Internal Helpers
	// ============================================================

	void PerformDelaunayTriangulation();
	bool IsPointInCircumcircle(const FVector2D& Point, const FBlendTriangle& Tri) const;

	int32 FindContainingTriangle(const FVector2D& Point) const;
	bool IsPointInTriangle(const FVector2D& Point, const FBlendTriangle& Tri) const;
	void GetBarycentricCoords(const FVector2D& Point, const FBlendTriangle& Tri,
	                          float& OutU, float& OutV, float& OutW) const;
	int32 FindClosestSample(const FVector2D& Point) const;

	void EvaluateAnimation(UAnimSequence* Animation, float Time, TArray<FTransform>& OutPose);
	void BlendPosesBarycentric(const TArray<FTransform>& PoseA,
	                           const TArray<FTransform>& PoseB,
	                           const TArray<FTransform>& PoseC,
	                           float U, float V, float W,
	                           TArray<FTransform>& OutPose);
	void BlendTwoPoses(const TArray<FTransform>& PoseA,
	                   const TArray<FTransform>& PoseB,
	                   float Alpha,
	                   TArray<FTransform>& OutPose);
};
