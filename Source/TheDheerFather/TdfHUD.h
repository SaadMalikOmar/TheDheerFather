#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TdfHUD.generated.h"

/** Canvas-drawn HUD: health/kealth + stamina bars, status flags and a speed readout. */
UCLASS()
class THEDHEERFATHER_API ATdfHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

protected:
	void DrawBar(float X, float Y, float Width, float Height, float Fraction, const FLinearColor& FillColor, const FString& Label);
};
