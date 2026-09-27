// CountriesIRL 3D Game

#include "Core/CIRLPlayerController.h"
#include "Core/CIRLInputConfig.h"
#include "Core/CIRLHUD.h"
#include "World/WorldClockSubsystem.h"
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

void ACIRLPlayerController::DevBodyAll()
{
	// Follow the player's current style, flipped, so everyone matches
	const ABallCharacter* Mine = Cast<ABallCharacter>(GetPawn());
	const EBallBodyStyle Target = (Mine && Mine->GetBodyStyle() == EBallBodyStyle::Countryball) ? EBallBodyStyle::Humanoid : EBallBodyStyle::Countryball;
	for (TActorIterator<ABallCharacter> It(GetWorld()); It; ++It)
	{
		It->SetBodyStyle(Target);
	}
}
