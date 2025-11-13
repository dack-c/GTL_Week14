#pragma once
#include "Object.h"

/**
 * @brief 애니메이션 클립이 순수 데이터 모델
 * 모든 종류의 애니메이션 데이터를 표현하는 인터페이스
 * 
 * 예를들어)
 * UAnimSequence가 겉에서 보면 “달리기 애니”, “점프 애니” 같은 에셋인데,
 * 그 “달리기 애니”의 실제 내용물(각 본이 몇 초에 어디로 회전/이동하는지, 커브 값은 뭔지)을 정리해서
 * 들고 있는 내부 데이터 저장소가 UAnimDataModel 역할
 */

struct FRawAnimSequenceTrack
{
    TArray<FVector> PosKeys;   // 위치 키프레임
    TArray<FQuat>   RotKeys;   // 회전 키프레임 (Quaternion)
    TArray<FVector> ScaleKeys; // 스케일 키프레임
};

struct FBoneAnimationTrack
{
    FName Name;                        // Bone 이름
    FRawAnimSequenceTrack InternalTrack; // 실제 애니메이션 데이터
};


class UAnimDataModel : public UObject
{
    DECLARE_CLASS(UAnimDataModel, UObject)

public:
    UAnimDataModel() = default;
    virtual ~UAnimDataModel() = default;


public:
    // TODO : 개귀찮은 getter/setter함수 추가 예정

    
private:
    TArray<FBoneAnimationTrack> BoneAnimationTracks;
    float PlayLength;
    int32 FrameRate;
    int32 NumberOfFrames;
    int32 NumberOfKeys;
    
    //FAnimationCurveData CurveData;
    
};
