// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CIRLHUD.generated.h"

/**
 *  Minimal in-game HUD. Keeps the screen clean (design: no clutter): the stamina bar only shows
 *  while stamina is being used or refilling, then fades out.
 *  Placeholder drawing; moves to UMG when the compass bar is built.
 */
UCLASS()
class ACIRLHUD : public AHUD
{
	GENERATED_BODY()

public:

	virtual void DrawHUD() override;

protected:

	UPROPERTY(EditAnywhere, Category="HUD")
	FVector2D StaminaBarSize = FVector2D(260.f, 6.f);

	/** Bar position from the top of the screen, as a fraction of screen height */
	UPROPERTY(EditAnywhere, Category="HUD")
	float StaminaBarHeight = 0.9f;

	/** Seconds the bar stays visible after stamina is full again */
	UPROPERTY(EditAnywhere, Category="HUD")
	float StaminaFadeDelay = 1.f;

private:

	void DrawStaminaBar();

	float StaminaBarAlpha = 0.f;
	float TimeSinceStaminaFull = 0.f;
};
