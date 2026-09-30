// CountriesIRL 3D Game

#include "Core/CIRLTitleGameMode.h"
#include "UI/SCIRLTitleScreen.h"
#include "Audio/CIRLAudioSubsystem.h"
#include "Audio/CIRLMusicSettings.h"
#include "Audio/CIRLPlaylistComponent.h"
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
	/** Title screen ambience (Art/Audio/SOURCES.md); the music is a playlist in the project settings */
	const TCHAR* TitleAmbiencePath = TEXT("/Game/CrownsAndCommoners/Audio/Ambience/amb_title_river.amb_title_river");
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

	UCIRLAudioSubsystem* Audio = UCIRLAudioSubsystem::Get(this);
	if (Audio)
	{
		Audio->ApplyVolumes(GetWorld());
	}

	// Main menu music (Project Settings > CountriesIRL Audio > Title Playlist): the first track, then random
	Playlist = NewObject<UCIRLPlaylistComponent>(this, TEXT("TitleMusic"));
	Playlist->RegisterComponent();
	Playlist->Play(GetDefault<UCIRLMusicSettings>()->TitlePlaylist);

	TWeakObjectPtr<UCIRLPlaylistComponent> WeakPlaylist = Playlist;
	SAssignNew(TitleScreen, SCIRLTitleScreen)
		.Audio(Audio)
		.NowPlaying_Lambda([WeakPlaylist]() { return FText::FromString(WeakPlaylist.IsValid() ? WeakPlaylist->GetCurrentTitle() : FString()); })
		.OnNextSong_Lambda([WeakPlaylist]() { if (WeakPlaylist.IsValid()) { WeakPlaylist->Next(); } })
		.OnNewGame(SCIRLTitleScreen::FOnAction::CreateUObject(this, &ACIRLTitleController::StartNewGame))
		.OnQuit(SCIRLTitleScreen::FOnAction::CreateUObject(this, &ACIRLTitleController::Quit));
	Viewport->AddViewportWidgetForPlayer(GetLocalPlayer(), TitleScreen.ToSharedRef(), 10);

	// The river stays underneath, quieter, so the music leads
	if (USoundBase* Sound = LoadObject<USoundBase>(nullptr, TitleAmbiencePath))
	{
		Ambience = UGameplayStatics::CreateSound2D(this, Sound, 0.45f);
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
	if (Playlist)
	{
		Playlist->Stop(0.3f);
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
