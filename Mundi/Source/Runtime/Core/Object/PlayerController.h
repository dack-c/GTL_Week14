#pragma once
#include "Controller.h"
#include "APlayerController.generated.h"	

class APlayerController : public AController
{
public:
	GENERATED_REFLECTION_BODY()

	APlayerController();
	virtual ~APlayerController() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupInput();

	void SetMouseLookEnabled(bool bEnable) { bMouseLookEnabled = bEnable; }
	bool IsMouseLookEnabled() const { return  bMouseLookEnabled; }
	void SetUseMovementInput(bool bEnable) { bUseMovementInput = bEnable; }
	bool IsUsingMovementInput() const { return bUseMovementInput; }
	void SetSensitivity(float InSensitivity) { Sensitivity = InSensitivity; }
	float GetSensitivity() const { return Sensitivity; }
protected:
    void ProcessMovementInput(float DeltaTime);
    void ProcessRotationInput(float DeltaTime);

protected:
    bool bMouseLookEnabled = true;

	bool bUseMovementInput = true;

private:
	float Sensitivity = 0.1;


};
