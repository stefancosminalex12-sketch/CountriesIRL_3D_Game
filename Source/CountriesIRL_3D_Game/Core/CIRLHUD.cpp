// CountriesIRL 3D Game

#include "Core/CIRLHUD.h"
#include "Characters/BallCharacter.h"
#include "Characters/HealthComponent.h"
#include "Characters/StaminaComponent.h"
#include "World/WorldClockSubsystem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

void ACIRLHUD::DrawHUD()
{
	Super::DrawHUD();

	const ABallCharacter* Ball = Cast<ABallCharacter>(GetOwningPawn());
	if (!Ball || !Canvas)
	{
		return;
	}

	const float Scale = Canvas->ClipY / 1080.f;
	const float X = Margin.X * Scale;
	const float Width = BarWidth * Scale;

	// Stack from the bottom up: stamina at the bottom, health above it
	const float StaminaY = Canvas->ClipY - (Margin.Y + StaminaBarHeight) * Scale;
	const float HealthY = StaminaY - (BarSpacing + HealthBarHeight) * Scale;

	if (const UHealthComponent* Health = Ball->GetHealth())
	{
		DrawBar(X, HealthY, Width, HealthBarHeight * Scale, Health->GetHealthPercent(), FLinearColor(0.62f, 0.07f, 0.05f), 1.f);
	}

	if (const UStaminaComponent* Stamina = Ball->GetStamina())
	{
		const float DeltaTime = GetWorld()->GetDeltaSeconds();
		const float Percent = Stamina->GetStaminaPercent();

		// Show while not full; fade out a moment after it fills up
		TimeSinceStaminaFull = Percent >= 1.f ? TimeSinceStaminaFull + DeltaTime : 0.f;
		const float TargetAlpha = TimeSinceStaminaFull < StaminaFadeDelay ? 1.f : 0.f;
		StaminaBarAlpha = FMath::FInterpTo(StaminaBarAlpha, TargetAlpha, DeltaTime, 6.f);

		// Parchment-ish fill; turns reddish while exhausted
		const FLinearColor Fill = Stamina->IsExhausted() ? FLinearColor(0.75f, 0.2f, 0.12f) : FLinearColor(0.92f, 0.82f, 0.55f);
		DrawBar(X, StaminaY, Width, StaminaBarHeight * Scale, Percent, Fill, StaminaBarAlpha);
	}

	if (bShowClock)
	{
		DrawClock(Scale);
	}
}

void ACIRLHUD::DrawClock(float Scale)
{
	const UWorldClockSubsystem* Clock = GetWorld()->GetSubsystem<UWorldClockSubsystem>();
	if (!Clock)
	{
		return;
	}

	const FString Text = FString::Printf(TEXT("%s   %s"), *Clock->FormatDate(), *Clock->FormatTime());
	DrawText(Text, FLinearColor(0.95f, 0.9f, 0.75f), Margin.X * Scale, Margin.X * Scale, GEngine->GetMediumFont(), Scale * 1.2f);
}

void ACIRLHUD::DrawBar(float X, float Y, float Width, float Height, float Percent, const FLinearColor& Fill, float Alpha)
{
	if (Alpha < 0.01f)
	{
		return;
	}

	const float Border = FMath::Max(2.f, Height * 0.2f);
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.5f * Alpha), X - Border, Y - Border, Width + Border * 2.f, Height + Border * 2.f);
	DrawRect(Fill.CopyWithNewOpacity(0.92f * Alpha), X, Y, Width * FMath::Clamp(Percent, 0.f, 1.f), Height);
}
