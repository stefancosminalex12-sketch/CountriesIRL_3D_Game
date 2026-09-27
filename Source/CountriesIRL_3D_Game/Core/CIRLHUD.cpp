// CountriesIRL 3D Game

#include "Core/CIRLHUD.h"
#include "Characters/BallCharacter.h"
#include "Characters/HealthComponent.h"
#include "Characters/StaminaComponent.h"
#include "Characters/PlayerBallCharacter.h"
#include "Animals/Horse.h"
#include "World/WorldClockSubsystem.h"
#include "World/SeasonSubsystem.h"
#include "World/MedievalCalendar.h"
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

	// In the saddle: the horse's stamina above our health bar (brown), always shown
	if (const AHorse* Horse = Ball->GetMount())
	{
		const UStaminaComponent* HorseStamina = Horse->GetStamina();
		const float HorseY = HealthY - (BarSpacing + StaminaBarHeight) * Scale;
		const FLinearColor Fill = HorseStamina->IsExhausted() ? FLinearColor(0.75f, 0.2f, 0.12f) : FLinearColor(0.55f, 0.33f, 0.14f);
		DrawBar(X, HorseY, Width, StaminaBarHeight * Scale, HorseStamina->GetStaminaPercent(), Fill, 1.f);
	}

	// What the interact key would do (e.g. "E  Get on the horse"), bottom center
	if (const APlayerBallCharacter* Player = Cast<APlayerBallCharacter>(Ball))
	{
		const FString Prompt = Player->GetInteractPrompt();
		if (!Prompt.IsEmpty())
		{
			float TextWidth = 0.f;
			float TextHeight = 0.f;
			const float TextScale = Scale * 1.3f;
			GetTextSize(Prompt, TextWidth, TextHeight, GEngine->GetMediumFont(), TextScale);
			const float PromptX = (Canvas->ClipX - TextWidth) * 0.5f;
			const float PromptY = Canvas->ClipY - 150.f * Scale;
			DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.45f), PromptX - 12.f * Scale, PromptY - 6.f * Scale, TextWidth + 24.f * Scale, TextHeight + 12.f * Scale);
			DrawText(Prompt, FLinearColor(0.95f, 0.9f, 0.75f), PromptX, PromptY, GEngine->GetMediumFont(), TextScale);
		}
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

	const FLinearColor TextColor(0.95f, 0.9f, 0.75f);
	const float X = Margin.X * Scale;
	float Y = Margin.X * Scale;
	const float LineHeight = 22.f * Scale;

	const FString Text = FString::Printf(TEXT("%s   %s"), *Clock->FormatDate(), *Clock->FormatTime());
	DrawText(Text, TextColor, X, Y, GEngine->GetMediumFont(), Scale * 1.2f);

	// Season, weather-ish details and the medieval calendar
	FString Details;
	if (const USeasonSubsystem* Seasons = GetWorld()->GetSubsystem<USeasonSubsystem>())
	{
		const FSeasonState& State = Seasons->GetState();
		Details = FString::Printf(TEXT("%s   %.0f%cC"), *USeasonSubsystem::SeasonName(State.Season), State.Temperature, TCHAR(0x00B0));
		if (State.Frost > 0.3f) { Details += TEXT("   frost"); }
		if (State.Mist > 0.3f) { Details += TEXT("   mist"); }
	}
	const FDateTime Date = Clock->GetDateTime();
	const FString ChurchSeason = MedievalCalendar::ChurchSeason(Date);
	if (!ChurchSeason.IsEmpty()) { Details += TEXT("   ") + ChurchSeason; }

	Y += LineHeight * 1.3f;
	DrawText(Details, TextColor, X, Y, GEngine->GetSmallFont(), Scale * 1.2f);

	const FString Feast = MedievalCalendar::FeastDay(Date);
	if (!Feast.IsEmpty())
	{
		Y += LineHeight;
		DrawText(Feast, FLinearColor(1.f, 0.8f, 0.4f), X, Y, GEngine->GetSmallFont(), Scale * 1.2f);
	}
	Y += LineHeight;
	DrawText(TEXT("In the fields: ") + MedievalCalendar::FarmWork(Date), TextColor, X, Y, GEngine->GetSmallFont(), Scale * 1.2f);
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
