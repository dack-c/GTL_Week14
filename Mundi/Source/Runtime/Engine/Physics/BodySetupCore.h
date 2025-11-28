#pragma once
/// <summary>
/// 코어 엔진이 알아야 할 최소 물리 메타데이터
/// </summary>
class UBodySetupCore : public UObject
{
	DECLARE_CLASS(UBodySetupCore, UObject)
public:
	FName BoneName;

	// Collision Enabled, Channel Response 같은 공통 설정
	// 나중에 메타데이터도 설정 
};