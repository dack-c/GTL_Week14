#include "pch.h"
#include "BlendSpace2D.h"
#include "Source/Runtime/Engine/Animation/AnimSequence.h"
#include "Source/Runtime/Engine/Animation/AnimDateModel.h"

IMPLEMENT_CLASS(UBlendSpace2D)

// ============================================================
// Parameter Setting
// ============================================================

void UBlendSpace2D::SetParameter(float InX, float InY)
{
	CurrentParameter.X = InX;
	CurrentParameter.Y = InY;
}

void UBlendSpace2D::SetParameter(const FVector2D& InParameter)
{
	CurrentParameter = InParameter;
}

// ============================================================
// Sample Management
// ============================================================

int32 UBlendSpace2D::AddSample(UAnimSequence* Animation, float X, float Y)
{
	if (!Animation)
	{
		UE_LOG("UBlendSpace2D::AddSample - Invalid animation");
		return -1;
	}

	int32 Index = Samples.Num();
	Samples.Add(FBlendSample2D(Animation, X, Y));
	bTriangulationDirty = true;

	UE_LOG("UBlendSpace2D::AddSample - Added '%s' at (%.1f, %.1f) (Total: %d samples)",
		Animation->ObjectName.ToString().c_str(), X, Y, Samples.Num());

	return Index;
}

void UBlendSpace2D::ClearSamples()
{
	Samples.Empty();
	Triangles.Empty();
	CurrentPlayTime = 0.0f;
	PreviousPlayTime = 0.0f;
	bTriangulationDirty = true;
}

const FBlendSample2D* UBlendSpace2D::GetSample(int32 Index) const
{
	if (Index >= 0 && Index < Samples.Num())
	{
		return &Samples[Index];
	}
	return nullptr;
}

bool UBlendSpace2D::SetSamplePosition(int32 Index, float X, float Y)
{
	if (Index < 0 || Index >= Samples.Num())
	{
		return false;
	}

	Samples[Index].Position.X = X;
	Samples[Index].Position.Y = Y;
	bTriangulationDirty = true;
	return true;
}

bool UBlendSpace2D::SetSampleAnimation(int32 Index, UAnimSequence* Animation)
{
	if (Index < 0 || Index >= Samples.Num())
	{
		return false;
	}

	if (!Animation)
	{
		return false;
	}

	Samples[Index].Animation = Animation;
	return true;
}

bool UBlendSpace2D::RemoveSample(int32 Index)
{
	if (Index < 0 || Index >= Samples.Num())
	{
		return false;
	}

	Samples.RemoveAt(Index);
	bTriangulationDirty = true;
	return true;
}

// ============================================================
// Parameter Range
// ============================================================

void UBlendSpace2D::SetParameterRange(const FVector2D& InMin, const FVector2D& InMax)
{
	MinParameter = InMin;
	MaxParameter = InMax;

	// Ensure Min <= Max
	if (MinParameter.X > MaxParameter.X)
	{
		float Temp = MinParameter.X;
		MinParameter.X = MaxParameter.X;
		MaxParameter.X = Temp;
	}
	if (MinParameter.Y > MaxParameter.Y)
	{
		float Temp = MinParameter.Y;
		MinParameter.Y = MaxParameter.Y;
		MaxParameter.Y = Temp;
	}
}

// ============================================================
// Triangulation
// ============================================================

void UBlendSpace2D::Triangulate()
{
	PerformDelaunayTriangulation();
	bTriangulationDirty = false;
}

void UBlendSpace2D::AddTriangle(int32 A, int32 B, int32 C)
{
	// 유효성 검사
	if (A < 0 || B < 0 || C < 0)
	{
		return;
	}

	// 중복 검사
	for (const FBlendTriangle& Tri : Triangles)
	{
		// 같은 세 점으로 이루어진 삼각형인지 확인 (순서 무관)
		bool bHasA = Tri.ContainsVertex(A);
		bool bHasB = Tri.ContainsVertex(B);
		bool bHasC = Tri.ContainsVertex(C);
		if (bHasA && bHasB && bHasC)
		{
			return; // 이미 존재함
		}
	}

	Triangles.Add(FBlendTriangle(A, B, C));
	UE_LOG("UBlendSpace2D::AddTriangle - Added triangle (%d, %d, %d)", A, B, C);
}

bool UBlendSpace2D::RemoveTriangle(int32 Index)
{
	if (Index < 0 || Index >= Triangles.Num())
	{
		return false;
	}

	Triangles.RemoveAt(Index);
	return true;
}

void UBlendSpace2D::ClearTriangles()
{
	Triangles.Empty();
}

void UBlendSpace2D::PerformDelaunayTriangulation()
{
	Triangles.Empty();

	if (Samples.Num() < 3)
	{
		return;
	}

	// 1. Create super triangle (contains all points)
	float MinX = MinParameter.X - 100.0f;
	float MaxX = MaxParameter.X + 100.0f;
	float MinY = MinParameter.Y - 100.0f;
	float MaxY = MaxParameter.Y + 100.0f;
	float DX = MaxX - MinX;
	float DY = MaxY - MinY;

	int32 SuperIdx0 = Samples.Num();
	int32 SuperIdx1 = Samples.Num() + 1;
	int32 SuperIdx2 = Samples.Num() + 2;

	// Add super triangle vertices (temporary)
	Samples.Add(FBlendSample2D(nullptr, MinX - DX, MinY - DY));
	Samples.Add(FBlendSample2D(nullptr, MinX + DX * 2.0f, MinY - DY));
	Samples.Add(FBlendSample2D(nullptr, MinX + DX * 0.5f, MaxY + DY * 2.0f));

	Triangles.Add(FBlendTriangle(SuperIdx0, SuperIdx1, SuperIdx2));

	// 2. Add each sample point
	for (int32 PointIdx = 0; PointIdx < SuperIdx0; ++PointIdx)
	{
		const FVector2D& Point = Samples[PointIdx].Position;

		// 2a. Find triangles whose circumcircle contains this point
		TArray<FBlendTriangle> BadTriangles;
		for (const FBlendTriangle& Tri : Triangles)
		{
			if (IsPointInCircumcircle(Point, Tri))
			{
				BadTriangles.Add(Tri);
			}
		}

		// 2b. Find boundary edges of bad triangles (polygon hole)
		TArray<TPair<int32, int32>> Polygon;
		for (const FBlendTriangle& Tri : BadTriangles)
		{
			for (int32 i = 0; i < 3; ++i)
			{
				int32 E0 = Tri.Indices[i];
				int32 E1 = Tri.Indices[(i + 1) % 3];

				// Check if this edge is shared with another bad triangle
				bool bShared = false;
				for (const FBlendTriangle& Other : BadTriangles)
				{
					if (&Tri == &Other) continue;
					for (int32 j = 0; j < 3; ++j)
					{
						int32 O0 = Other.Indices[j];
						int32 O1 = Other.Indices[(j + 1) % 3];
						if ((E0 == O0 && E1 == O1) || (E0 == O1 && E1 == O0))
						{
							bShared = true;
							break;
						}
					}
					if (bShared) break;
				}

				if (!bShared)
				{
					Polygon.Add(TPair<int32, int32>(E0, E1));
				}
			}
		}

		// 2c. Remove bad triangles
		for (const FBlendTriangle& Bad : BadTriangles)
		{
			for (int32 i = Triangles.Num() - 1; i >= 0; --i)
			{
				if (Triangles[i] == Bad)
				{
					Triangles.RemoveAt(i);
					break;
				}
			}
		}

		// 2d. Create new triangles (hole edges + new point)
		for (const auto& Edge : Polygon)
		{
			Triangles.Add(FBlendTriangle(Edge.first, Edge.second, PointIdx));
		}
	}

	// 3. Remove triangles connected to super triangle vertices
	for (int32 i = Triangles.Num() - 1; i >= 0; --i)
	{
		if (Triangles[i].ContainsVertex(SuperIdx0) ||
			Triangles[i].ContainsVertex(SuperIdx1) ||
			Triangles[i].ContainsVertex(SuperIdx2))
		{
			Triangles.RemoveAt(i);
		}
	}

	// 4. Remove super triangle vertices
	Samples.SetNum(SuperIdx0);

	UE_LOG("UBlendSpace2D::Triangulate - Created %d triangles from %d samples",
		Triangles.Num(), Samples.Num());
}

bool UBlendSpace2D::IsPointInCircumcircle(const FVector2D& Point, const FBlendTriangle& Tri) const
{
	const FVector2D& A = Samples[Tri.Indices[0]].Position;
	const FVector2D& B = Samples[Tri.Indices[1]].Position;
	const FVector2D& C = Samples[Tri.Indices[2]].Position;

	// Determinant method (assumes CCW order)
	float ax = A.X - Point.X;
	float ay = A.Y - Point.Y;
	float bx = B.X - Point.X;
	float by = B.Y - Point.Y;
	float cx = C.X - Point.X;
	float cy = C.Y - Point.Y;

	float det = (ax * ax + ay * ay) * (bx * cy - cx * by)
	          - (bx * bx + by * by) * (ax * cy - cx * ay)
	          + (cx * cx + cy * cy) * (ax * by - bx * ay);

	// Handle both CW and CCW triangles
	float area = (B.X - A.X) * (C.Y - A.Y) - (C.X - A.X) * (B.Y - A.Y);
	if (area < 0.0f)
	{
		det = -det;
	}

	return det > 0.0f;
}

// ============================================================
// Triangle Search & Interpolation
// ============================================================

int32 UBlendSpace2D::FindContainingTriangle(const FVector2D& Point) const
{
	for (int32 i = 0; i < Triangles.Num(); ++i)
	{
		if (IsPointInTriangle(Point, Triangles[i]))
		{
			return i;
		}
	}
	return -1;
}

bool UBlendSpace2D::IsPointInTriangle(const FVector2D& Point, const FBlendTriangle& Tri) const
{
	const FVector2D& A = Samples[Tri.Indices[0]].Position;
	const FVector2D& B = Samples[Tri.Indices[1]].Position;
	const FVector2D& C = Samples[Tri.Indices[2]].Position;

	auto Sign = [](const FVector2D& P1, const FVector2D& P2, const FVector2D& P3)
	{
		return (P1.X - P3.X) * (P2.Y - P3.Y) - (P2.X - P3.X) * (P1.Y - P3.Y);
	};

	float D1 = Sign(Point, A, B);
	float D2 = Sign(Point, B, C);
	float D3 = Sign(Point, C, A);

	bool bHasNeg = (D1 < 0) || (D2 < 0) || (D3 < 0);
	bool bHasPos = (D1 > 0) || (D2 > 0) || (D3 > 0);

	return !(bHasNeg && bHasPos);
}

void UBlendSpace2D::GetBarycentricCoords(const FVector2D& Point, const FBlendTriangle& Tri,
                                          float& OutU, float& OutV, float& OutW) const
{
	const FVector2D& A = Samples[Tri.Indices[0]].Position;
	const FVector2D& B = Samples[Tri.Indices[1]].Position;
	const FVector2D& C = Samples[Tri.Indices[2]].Position;

	FVector2D V0 = FVector2D(B.X - A.X, B.Y - A.Y);
	FVector2D V1 = FVector2D(C.X - A.X, C.Y - A.Y);
	FVector2D V2 = FVector2D(Point.X - A.X, Point.Y - A.Y);

	float Dot00 = V0.X * V0.X + V0.Y * V0.Y;
	float Dot01 = V0.X * V1.X + V0.Y * V1.Y;
	float Dot02 = V0.X * V2.X + V0.Y * V2.Y;
	float Dot11 = V1.X * V1.X + V1.Y * V1.Y;
	float Dot12 = V1.X * V2.X + V1.Y * V2.Y;

	float Denom = Dot00 * Dot11 - Dot01 * Dot01;
	if (FMath::Abs(Denom) < 0.0001f)
	{
		// Degenerate triangle
		OutU = 1.0f;
		OutV = 0.0f;
		OutW = 0.0f;
		return;
	}

	float InvDenom = 1.0f / Denom;
	OutV = (Dot11 * Dot02 - Dot01 * Dot12) * InvDenom;  // Weight for B
	OutW = (Dot00 * Dot12 - Dot01 * Dot02) * InvDenom;  // Weight for C
	OutU = 1.0f - OutV - OutW;                          // Weight for A
}

int32 UBlendSpace2D::FindClosestSample(const FVector2D& Point) const
{
	if (Samples.Num() == 0)
	{
		return -1;
	}

	int32 ClosestIdx = 0;
	float ClosestDistSq = FLT_MAX;

	for (int32 i = 0; i < Samples.Num(); ++i)
	{
		float DX = Point.X - Samples[i].Position.X;
		float DY = Point.Y - Samples[i].Position.Y;
		float DistSq = DX * DX + DY * DY;

		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			ClosestIdx = i;
		}
	}

	return ClosestIdx;
}

// ============================================================
// Pose Evaluation
// ============================================================

void UBlendSpace2D::EvaluateAnimation(UAnimSequence* Animation, float Time, TArray<FTransform>& OutPose)
{
	if (!Animation)
	{
		return;
	}

	UAnimDataModel* DataModel = Animation->GetDataModel();
	if (!DataModel)
	{
		return;
	}

	int32 NumBones = DataModel->GetNumBoneTracks();
	OutPose.SetNum(NumBones);

	float PlayLength = Animation->GetPlayLength();
	if (PlayLength <= 0.0f)
	{
		return;
	}

	// Normalize time
	float NormalizedTime = fmod(Time, PlayLength);
	if (NormalizedTime < 0.0f)
	{
		NormalizedTime += PlayLength;
	}

	const TArray<FBoneAnimationTrack>& BoneTracks = DataModel->GetBoneAnimationTracks();
	for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
	{
		if (BoneIndex < BoneTracks.Num())
		{
			OutPose[BoneIndex] = DataModel->EvaluateBoneTrackTransform(BoneTracks[BoneIndex].Name, NormalizedTime);
		}
		else
		{
			OutPose[BoneIndex] = FTransform();
		}
	}

	// If using root motion, zero out root translation
	if (Animation->IsUsingRootMotion() && NumBones > 0)
	{
		FTransform RootTransform = OutPose[0];
		RootTransform.Translation = DataModel->EvaluateBoneTrackTransform(Animation->BoneNames[0], 0.0f, true).Translation;
		OutPose[0] = RootTransform;
	}
}

void UBlendSpace2D::BlendPosesBarycentric(
	const TArray<FTransform>& PoseA,
	const TArray<FTransform>& PoseB,
	const TArray<FTransform>& PoseC,
	float U, float V, float W,
	TArray<FTransform>& OutPose)
{
	int32 NumBones = PoseA.Num();
	if (PoseB.Num() < NumBones) NumBones = PoseB.Num();
	if (PoseC.Num() < NumBones) NumBones = PoseC.Num();

	OutPose.SetNum(NumBones);

	for (int32 i = 0; i < NumBones; ++i)
	{
		// Position: weighted average
		FVector BlendedPos =
			PoseA[i].Translation * U +
			PoseB[i].Translation * V +
			PoseC[i].Translation * W;

		// Scale: weighted average
		FVector BlendedScale =
			PoseA[i].Scale3D * U +
			PoseB[i].Scale3D * V +
			PoseC[i].Scale3D * W;

		// Rotation: sequential Slerp based on largest weight
		FQuat BlendedRot;
		if (U >= V && U >= W)
		{
			// A has largest weight
			float TotalVW = V + W;
			if (TotalVW > 0.001f)
			{
				FQuat BC = FQuat::Slerp(PoseB[i].Rotation, PoseC[i].Rotation, W / TotalVW);
				BlendedRot = FQuat::Slerp(PoseA[i].Rotation, BC, TotalVW);
			}
			else
			{
				BlendedRot = PoseA[i].Rotation;
			}
		}
		else if (V >= W)
		{
			// B has largest weight
			float TotalUW = U + W;
			if (TotalUW > 0.001f)
			{
				FQuat AC = FQuat::Slerp(PoseA[i].Rotation, PoseC[i].Rotation, W / TotalUW);
				BlendedRot = FQuat::Slerp(PoseB[i].Rotation, AC, TotalUW);
			}
			else
			{
				BlendedRot = PoseB[i].Rotation;
			}
		}
		else
		{
			// C has largest weight
			float TotalUV = U + V;
			if (TotalUV > 0.001f)
			{
				FQuat AB = FQuat::Slerp(PoseA[i].Rotation, PoseB[i].Rotation, V / TotalUV);
				BlendedRot = FQuat::Slerp(PoseC[i].Rotation, AB, TotalUV);
			}
			else
			{
				BlendedRot = PoseC[i].Rotation;
			}
		}

		OutPose[i] = FTransform(BlendedPos, BlendedRot, BlendedScale);
	}
}

void UBlendSpace2D::BlendTwoPoses(
	const TArray<FTransform>& PoseA,
	const TArray<FTransform>& PoseB,
	float Alpha,
	TArray<FTransform>& OutPose)
{
	int32 NumBones = FMath::Min(PoseA.Num(), PoseB.Num());
	OutPose.SetNum(NumBones);

	for (int32 i = 0; i < NumBones; ++i)
	{
		FVector BlendedPos = FVector::Lerp(PoseA[i].Translation, PoseB[i].Translation, Alpha);
		FQuat BlendedRot = FQuat::Slerp(PoseA[i].Rotation, PoseB[i].Rotation, Alpha);
		FVector BlendedScale = FVector::Lerp(PoseA[i].Scale3D, PoseB[i].Scale3D, Alpha);

		OutPose[i] = FTransform(BlendedPos, BlendedRot, BlendedScale);
	}
}

// ============================================================
// Main Update
// ============================================================

void UBlendSpace2D::Update(float X, float Y, float DeltaTime, TArray<FTransform>& OutPose)
{
	if (Samples.Num() == 0)
	{
		return;
	}

	// Clamp parameter
	CurrentParameter.X = FMath::Clamp(X, MinParameter.X, MaxParameter.X);
	CurrentParameter.Y = FMath::Clamp(Y, MinParameter.Y, MaxParameter.Y);

	// Handle special cases
	if (Samples.Num() == 1)
	{
		// Single sample: output directly
		DominantSequence = Samples[0].Animation;
		EvaluateAnimation(Samples[0].Animation, CurrentPlayTime, OutPose);

		if (Samples[0].Animation)
		{
			float PlayLength = Samples[0].Animation->GetPlayLength();
			PreviousPlayTime = CurrentPlayTime;
			CurrentPlayTime += DeltaTime;
			if (PlayLength > 0.0f)
			{
				CurrentPlayTime = fmod(CurrentPlayTime, PlayLength);
			}
		}
		return;
	}

	if (Samples.Num() == 2)
	{
		// Two samples: linear interpolation
		const FVector2D& PosA = Samples[0].Position;
		const FVector2D& PosB = Samples[1].Position;

		FVector2D Dir = FVector2D(PosB.X - PosA.X, PosB.Y - PosA.Y);
		float LenSq = Dir.X * Dir.X + Dir.Y * Dir.Y;

		float Alpha = 0.0f;
		if (LenSq > 0.0001f)
		{
			FVector2D ToPoint = FVector2D(CurrentParameter.X - PosA.X, CurrentParameter.Y - PosA.Y);
			float T = (ToPoint.X * Dir.X + ToPoint.Y * Dir.Y) / LenSq;
			Alpha = FMath::Clamp(T, 0.0f, 1.0f);
		}

		DominantSequence = (Alpha <= 0.5f) ? Samples[0].Animation : Samples[1].Animation;

		TArray<FTransform> PoseA, PoseB;
		EvaluateAnimation(Samples[0].Animation, CurrentPlayTime, PoseA);
		EvaluateAnimation(Samples[1].Animation, CurrentPlayTime, PoseB);
		BlendTwoPoses(PoseA, PoseB, Alpha, OutPose);

		float PlayLengthA = Samples[0].Animation ? Samples[0].Animation->GetPlayLength() : 0.0f;
		float PlayLengthB = Samples[1].Animation ? Samples[1].Animation->GetPlayLength() : 0.0f;
		float MinPlayLength = FMath::Min(PlayLengthA, PlayLengthB);

		PreviousPlayTime = CurrentPlayTime;
		CurrentPlayTime += DeltaTime;
		if (MinPlayLength > 0.0f)
		{
			CurrentPlayTime = fmod(CurrentPlayTime, MinPlayLength);
		}
		return;
	}

	// 3+ samples: use Delaunay triangulation (only if no manual triangles exist)
	if (bTriangulationDirty && Triangles.Num() == 0)
	{
		PerformDelaunayTriangulation();
		bTriangulationDirty = false;
	}

	// Find containing triangle
	int32 TriIndex = FindContainingTriangle(CurrentParameter);

	if (TriIndex < 0)
	{
		// Outside all triangles: use closest sample
		int32 ClosestIdx = FindClosestSample(CurrentParameter);
		if (ClosestIdx >= 0)
		{
			DominantSequence = Samples[ClosestIdx].Animation;
			EvaluateAnimation(Samples[ClosestIdx].Animation, CurrentPlayTime, OutPose);

			if (Samples[ClosestIdx].Animation)
			{
				float PlayLength = Samples[ClosestIdx].Animation->GetPlayLength();
				PreviousPlayTime = CurrentPlayTime;
				CurrentPlayTime += DeltaTime;
				if (PlayLength > 0.0f)
				{
					CurrentPlayTime = fmod(CurrentPlayTime, PlayLength);
				}
			}
		}
		return;
	}

	// Get barycentric coordinates
	const FBlendTriangle& Tri = Triangles[TriIndex];
	float U, V, W;
	GetBarycentricCoords(CurrentParameter, Tri, U, V, W);

	// Clamp weights to valid range
	U = FMath::Clamp(U, 0.0f, 1.0f);
	V = FMath::Clamp(V, 0.0f, 1.0f);
	W = FMath::Clamp(W, 0.0f, 1.0f);

	// Normalize
	float Total = U + V + W;
	if (Total > 0.001f)
	{
		U /= Total;
		V /= Total;
		W /= Total;
	}

	// Determine dominant sequence (highest weight)
	if (U >= V && U >= W)
	{
		DominantSequence = Samples[Tri.Indices[0]].Animation;
	}
	else if (V >= W)
	{
		DominantSequence = Samples[Tri.Indices[1]].Animation;
	}
	else
	{
		DominantSequence = Samples[Tri.Indices[2]].Animation;
	}

	// Evaluate three poses
	TArray<FTransform> PoseA, PoseB, PoseC;
	EvaluateAnimation(Samples[Tri.Indices[0]].Animation, CurrentPlayTime, PoseA);
	EvaluateAnimation(Samples[Tri.Indices[1]].Animation, CurrentPlayTime, PoseB);
	EvaluateAnimation(Samples[Tri.Indices[2]].Animation, CurrentPlayTime, PoseC);

	// Blend with barycentric weights
	BlendPosesBarycentric(PoseA, PoseB, PoseC, U, V, W, OutPose);

	// Update play time
	float PlayLengthA = Samples[Tri.Indices[0]].Animation ? Samples[Tri.Indices[0]].Animation->GetPlayLength() : 0.0f;
	float PlayLengthB = Samples[Tri.Indices[1]].Animation ? Samples[Tri.Indices[1]].Animation->GetPlayLength() : 0.0f;
	float PlayLengthC = Samples[Tri.Indices[2]].Animation ? Samples[Tri.Indices[2]].Animation->GetPlayLength() : 0.0f;
	float MinPlayLength = FMath::Min(FMath::Min(PlayLengthA, PlayLengthB), PlayLengthC);

	PreviousPlayTime = CurrentPlayTime;
	CurrentPlayTime += DeltaTime;
	if (MinPlayLength > 0.0f)
	{
		CurrentPlayTime = fmod(CurrentPlayTime, MinPlayLength);
	}
}

// ============================================================
// IAnimPoseProvider Interface
// ============================================================

void UBlendSpace2D::EvaluatePose(float Time, float DeltaTime, TArray<FTransform>& OutPose)
{
	Update(CurrentParameter.X, CurrentParameter.Y, DeltaTime, OutPose);
}

float UBlendSpace2D::GetPlayLength() const
{
	if (Samples.Num() == 0)
	{
		return 0.0f;
	}

	if (Samples[0].Animation)
	{
		return Samples[0].Animation->GetPlayLength();
	}

	return 0.0f;
}

int32 UBlendSpace2D::GetNumBoneTracks() const
{
	if (Samples.Num() == 0)
	{
		return 0;
	}

	if (Samples[0].Animation)
	{
		UAnimDataModel* DataModel = Samples[0].Animation->GetDataModel();
		if (DataModel)
		{
			return DataModel->GetNumBoneTracks();
		}
	}

	return 0;
}
