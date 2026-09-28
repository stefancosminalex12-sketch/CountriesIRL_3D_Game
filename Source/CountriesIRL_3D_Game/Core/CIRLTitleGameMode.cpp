// CountriesIRL 3D Game

#include "Core/CIRLTitleGameMode.h"
#include "UI/SCIRLTitleScreen.h"
#include "UI/CIRLMenuNavigation.h"
#include "World/WorldSimulationSettings.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

namespace
{
	/** Title screen ambience (Art/Audio/SOURCES.md) */
	const TCHAR* TitleAmbiencePath = TEXT("/Game/CountriesIRL/Audio/Ambience/amb_title_river.amb_title_river");
}

ACIRLTitleGameMode::ACIRLTitleGameMode()
{
	// Nobody walks around on the title screen
	DefaultPawnClass = nullptr;
	PlayerControllerClass = ACIRLTitleController::StaticClass();
}

void ACIRLTitleController::BeginPlay()
{
	Super::BeginPlay();

	UGameViewportClient* Viewport = GetWorld()->GetGameViewport();
	if (!IsLocalPlayerController() || !Viewport)
	{
		return;
	}

	SAssignNew(TitleScreen, SCIRLTitleScreen)
		.OnNewGame(SCIRLTitleScreen::FOnAction::CreateUObject(this, &ACIRLTitleController::StartNewGame))
		.OnQuit(SCIRLTitleScreen::FOnAction::CreateUObject(this, &ACIRLTitleController::Quit));
	Viewport->AddViewportWidgetForPlayer(GetLocalPlayer(), TitleScreen.ToSharedRef(), 10);

	if (USoundBase* Sound = LoadObject<USoundBase>(nullptr, TitleAmbiencePath))
	{
		Ambience = UGameplayStatics::CreateSound2D(this, Sound, 0.8f);
		if (Ambience)
		{
			Ambience->FadeIn(2.5f);
		}
	}

	PreviousNavigation = CIRLMenuNavigation::Push();
	SetShowMouseCursor(true);
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(TitleScreen);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void ACIRLTitleController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (TitleScreen.IsValid())
	{
		if (UGameViewportClient* Viewport = GetWorld()->GetGameViewport())
		{
			Viewport->RemoveViewportWidgetForPlayer(GetLocalPlayer(), TitleScreen.ToSharedRef());
		}
		TitleScreen.Reset();
	}
	CIRLMenuNavigation::Pop(PreviousNavigation);
	Super::EndPlay(EndPlayReason);
}

void ACIRLTitleController::StartNewGame()
{
	const FSoftObjectPath& Map = GetDefault<UWorldSimulationSettings>()->NewGameMap;
	if (Ambience)
	{
		Ambience->FadeOut(0.3f, 0.f);
	}
	// The world's own player controller takes over input and hides the cursor
	SetInputMode(FInputModeGameOnly());
	SetShowMouseCursor(false);
	UGameplayStatics::OpenLevel(this, FName(*Map.GetLongPackageName()));
}

void ACIRLTitleController::Quit()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}
