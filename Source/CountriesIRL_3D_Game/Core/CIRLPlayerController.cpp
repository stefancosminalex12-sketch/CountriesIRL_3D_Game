// CountriesIRL 3D Game

#include "Core/CIRLPlayerController.h"
#include "Core/CIRLInputConfig.h"
#include "Core/CIRLHUD.h"
#include "World/WorldClockSubsystem.h"
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
