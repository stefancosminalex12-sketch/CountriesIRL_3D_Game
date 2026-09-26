// CountriesIRL 3D Game

#include "Core/CIRLHUD.h"
#include "Characters/BallCharacter.h"
#include "Characters/StaminaComponent.h"
#include "Engine/Canvas.h"

void ACIRLHUD::DrawHUD()
{
	Super::DrawHUD();
	DrawStaminaBar();
}

void ACIRLHUD::DrawStaminaBar()
{
	const ABallCharacter* Ball = Cast<ABallCharacter>(GetOwningPawn());
	const UStaminaComponent* Stamina = Ball ? Ball->GetStamina() : nullptr;
	if (!Stamina || !Canvas)
	{
		return;
	}

	const float DeltaTime = GetWorld()->GetDeltaSeconds();
	const float Percent = Stamina->GetStaminaPercent();

	// Show while not full; fade out a moment after it fills up
	TimeSinceStaminaFull = Percent >= 1.f ? TimeSinceStaminaFull + DeltaTime : 0.f;
	const float TargetAlpha = TimeSinceStaminaFull < StaminaFadeDelay ? 1.f : 0.f;
	StaminaBarAlpha = FMath::FInterpTo(StaminaBarAlpha, TargetAlpha, DeltaTime, 6.f);
	if (StaminaBarAlpha < 0.01f)
	{
		return;
	}

	const float X = (Canvas->ClipX - StaminaBarSize.X) * 0.5f;
	const float Y = Canvas->ClipY * StaminaBarHeight;

	// Parchment-ish fill; turns reddish while exhausted
	const FLinearColor Fill = Stamina->IsExhausted() ? FLinearColor(0.75f, 0.2f, 0.12f) : FLinearColor(0.92f, 0.82f, 0.55f);

	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.45f * StaminaBarAlpha), X - 2.f, Y - 2.f, StaminaBarSize.X + 4.f, StaminaBarSize.Y + 4.f);
	DrawRect(Fill.CopyWithNewOpacity(0.9f * StaminaBarAlpha), X, Y, StaminaBarSize.X * Percent, StaminaBarSize.Y);
}
