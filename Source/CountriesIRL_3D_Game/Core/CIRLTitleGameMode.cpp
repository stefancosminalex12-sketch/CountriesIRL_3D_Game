// CountriesIRL 3D Game

#include "Core/CIRLTitleGameMode.h"
#include "UI/SCIRLTitleScreen.h"
#include "UI/CIRLMenuNavigation.h"
#include "World/WorldSimulationSettings.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

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
	// The world's own player controller takes over input and hides the cursor
	SetInputMode(FInputModeGameOnly());
	SetShowMouseCursor(false);
	UGameplayStatics::OpenLevel(this, FName(*Map.GetLongPackageName()));
}

void ACIRLTitleController::Quit()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}
