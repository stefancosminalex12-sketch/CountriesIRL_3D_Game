// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CIRLHUD.generated.h"

/**
 *  Minimal in-game HUD, bottom-left: health bar (always shown) with the stamina bar under it
 *  (only while stamina is not full, then it fades so the screen stays clean).
 *  Sizes are for a 1080p screen and scale with resolution.
 *  Placeholder canvas drawing; moves to UMG when the compass bar is built.
 */
UCLASS()
class ACIRLHUD : public AHUD
{
	GENERATED_BODY()

public:

	virtual void DrawHUD() override;

	/** Dev/testing: date and time in the top-left corner */
	void SetShowClock(bool bShow) { bShowClock = bShow; }
	bool IsShowingClock() const { return bShowClock; }

protected:

	/** Distance from the left and bottom screen edges (1080p pixels) */
	UPROPERTY(EditAnywhere, Category="HUD")
	FVector2D Margin = FVector2D(40.f, 48.f);

	UPROPERTY(EditAnywhere, Category="HUD")
	float BarWidth = 320.f;

	UPROPERTY(EditAnywhere, Category="HUD")
	float HealthBarHeight = 12.f;

	UPROPERTY(EditAnywhere, Category="HUD")
	float StaminaBarHeight = 9.f;

	UPROPERTY(EditAnywhere, Category="HUD")
	float BarSpacing = 7.f;

	/** Seconds the stamina bar stays visible after stamina is full again */
	UPROPERTY(EditAnywhere, Category="HUD")
	float StaminaFadeDelay = 1.f;

private:

	/** Draws a bar with a dark frame; Percent fills from the left */
	void DrawBar(float X, float Y, float Width, float Height, float Percent, const FLinearColor& Fill, float Alpha);

	void DrawClock(float Scale);

	bool bShowClock = false;

	float StaminaBarAlpha = 0.f;
	float TimeSinceStaminaFull = 0.f;
};
