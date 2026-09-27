// CountriesIRL 3D Game

#include "Core/CIRLPlayerController.h"
#include "Core/CIRLInputConfig.h"
#include "Core/CIRLHUD.h"
#include "World/WorldClockSubsystem.h"
#include "World/WeatherSubsystem.h"
#include "Animals/Horse.h"
#include "Characters/BallCharacter.h"
#include "Engine/DamageEvents.h"
#include "EngineUtils.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"

void ACIRLPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	InputConfig = NewObject<UCIRLInputConfig>(this, TEXT("InputConfig"));
	InputConfig->Build();
}

void ACIRLPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!IsLocalPlayerController() || !InputConfig)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InputConfig->DefaultContext, 0);
	}
}

void ACIRLPlayerController::DevTime(float Hours)
{
	if (UWorldClockSubsystem* Clock = GetWorld()->GetSubsystem<UWorldClockSubsystem>())
	{
		Clock->SetTimeOfDay(Hours);
	}
}

void ACIRLPlayerController::DevTimeSpeed(float Multiplier)
{
	if (UWorldClockSubsystem* Clock = GetWorld()->GetSubsystem<UWorldClockSubsystem>())
	{
		Clock->SetSpeedMultiplier(Multiplier);
	}
}

void ACIRLPlayerController::DevClock()
{
	if (ACIRLHUD* GameHUD = GetHUD<ACIRLHUD>())
	{
		GameHUD->SetShowClock(!GameHUD->IsShowingClock());
	}
}

void ACIRLPlayerController::DevAdvance(float Hours)
{
	if (UWorldClockSubsystem* Clock = GetWorld()->GetSubsystem<UWorldClockSubsystem>())
	{
		Clock->AdvanceTime(FTimespan::FromHours(Hours));
	}
}

void ACIRLPlayerController::DevDate(int32 Day, int32 Month)
{
	if (UWorldClockSubsystem* Clock = GetWorld()->GetSubsystem<UWorldClockSubsystem>())
	{
		Clock->SetDate(Clock->GetDateTime().GetYear(), Month, Day);
	}
	// Show the clock so the result is visible
	if (ACIRLHUD* GameHUD = GetHUD<ACIRLHUD>())
	{
		GameHUD->SetShowClock(true);
	}
}

void ACIRLPlayerController::DevYear(int32 Year)
{
	if (UWorldClockSubsystem* Clock = GetWorld()->GetSubsystem<UWorldClockSubsystem>())
	{
		const FDateTime Now = Clock->GetDateTime();
		Clock->SetDate(Year, Now.GetMonth(), FMath::Min(Now.GetDay(), FDateTime::DaysInMonth(Year, Now.GetMonth())));
	}
	// Show the clock so the result is visible
	if (ACIRLHUD* GameHUD = GetHUD<ACIRLHUD>())
	{
		GameHUD->SetShowClock(true);
	}
}

void ACIRLPlayerController::DevWeather(const FString& Weather)
{
	UWeatherSubsystem* WeatherSystem = GetWorld()->GetSubsystem<UWeatherSubsystem>();
	if (!WeatherSystem)
	{
		return;
	}
	const int64 Value = StaticEnum<EForcedWeather>()->GetValueByNameString(Weather);
	WeatherSystem->ForceWeather(Value == INDEX_NONE ? EForcedWeather::None : static_cast<EForcedWeather>(Value));
	if (ACIRLHUD* GameHUD = GetHUD<ACIRLHUD>())
	{
		GameHUD->SetShowClock(true);
	}
}

void ACIRLPlayerController::DevHitHorse(float Amount)
{
	const ABallCharacter* Ball = Cast<ABallCharacter>(GetPawn());
	AHorse* Target = Ball ? Ball->GetMount() : nullptr;
	if (!Target && GetPawn())
	{
		float Best = TNumericLimits<float>::Max();
		for (TActorIterator<AHorse> It(GetWorld()); It; ++It)
		{
			const float Distance = FVector::Dist(It->GetActorLocation(), GetPawn()->GetActorLocation());
			if (!It->IsDead() && Distance < Best)
			{
				Best = Distance;
				Target = *It;
			}
		}
	}
	if (Target)
	{
		Target->TakeDamage(Amount, FDamageEvent(), this, GetPawn());
	}
}

void ACIRLPlayerController::DevHitNearest(float Amount)
{
	const APawn* Self = GetPawn();
	if (!Self)
	{
		return;
	}

	ABallCharacter* Nearest = nullptr;
	double NearestDistance = TNumericLimits<double>::Max();
	for (TActorIterator<ABallCharacter> It(GetWorld()); It; ++It)
	{
		if (*It == Self || It->IsDead())
		{
			continue;
		}
		const double Distance = FVector::DistSquared(It->GetActorLocation(), Self->GetActorLocation());
		if (Distance < NearestDistance)
		{
			NearestDistance = Distance;
			Nearest = *It;
		}
	}

	if (Nearest)
	{
		Nearest->TakeDamage(Amount, FDamageEvent(), this, GetPawn());
	}
}
