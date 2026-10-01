// Crowns & Commoners

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
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameViewportClient.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/SCIRLGameMenu.h"
#include "UI/SCIRLDeathScreen.h"
#include "UI/SCIRLCompassBar.h"
#include "Widgets/Layout/SBox.h"
#include "Characters/PlayerBallCharacter.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"
#include "UI/CIRLMenuNavigation.h"
#include "Audio/CIRLAudioSubsystem.h"
#include "Items/CIRLInventoryComponent.h"
#include "Audio/CIRLPlaylistComponent.h"
#include "UI/CIRLPaperDollStage.h"
#include "Characters/Heraldry.h"
#include "World/WorldSimulationSettings.h"

void ACIRLPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	InputConfig = NewObject<UCIRLInputConfig>(this, TEXT("InputConfig"));
	InputConfig->Build();
}

void ACIRLPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// The player's volumes (Settings) apply from the first frame of the world
	UCIRLAudioSubsystem* Audio = UCIRLAudioSubsystem::Get(this);
	if (Audio)
	{
		Audio->ApplyVolumes(GetWorld());
	}

	// In-game music (Project Settings > Crowns & Commoners Audio > Game Playlist): the first track, then random,
	// under the In-Game Music slider
	if (IsLocalPlayerController())
	{
		GameMusic = NewObject<UCIRLPlaylistComponent>(this, TEXT("GameMusic"));
		GameMusic->RegisterComponent();
		DeathMusic = NewObject<UCIRLPlaylistComponent>(this, TEXT("DeathMusic"));
		DeathMusic->RegisterComponent();
		StartGameMusic();

		// The compass bar, top centre, under every menu
		if (UGameViewportClient* Viewport = GetWorld()->GetGameViewport())
		{
			Compass = SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				.Padding(FMargin(0.f, 22.f, 0.f, 0.f))
				.Visibility(EVisibility::HitTestInvisible)
				[
					SNew(SCIRLCompassBar).Owner(this)
				];
			Viewport->AddViewportWidgetForPlayer(GetLocalPlayer(), Compass.ToSharedRef(), 5);
		}
	}
}

void ACIRLPlayerController::StartGameMusic()
{
	if (GameMusic)
	{
		const UCIRLAudioSubsystem* Audio = UCIRLAudioSubsystem::Get(this);
		GameMusic->Play(GetDefault<UCIRLMusicSettings>()->GamePlaylist, Audio ? Audio->GetGameMusicClass() : nullptr);
	}
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

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(InputConfig->GameMenu, ETriggerEvent::Started, this, &ACIRLPlayerController::OnGameMenuPressed);
		EIC->BindAction(InputConfig->Equipment, ETriggerEvent::Started, this, &ACIRLPlayerController::OnEquipmentPressed);
		EIC->BindAction(InputConfig->Map, ETriggerEvent::Started, this, &ACIRLPlayerController::OnMapPressed);
		EIC->BindAction(InputConfig->NextSong, ETriggerEvent::Started, this, &ACIRLPlayerController::OnNextSongPressed);
	}
}

void ACIRLPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Leaving the game with the menu open (e.g. stopping Play in the editor): put Slate back the way it was
	if (GameMenu.IsValid())
	{
		CloseGameMenu();
	}
	HideDeathScreen();
	if (Compass.IsValid())
	{
		if (UGameViewportClient* Viewport = GetWorld() ? GetWorld()->GetGameViewport() : nullptr)
		{
			Viewport->RemoveViewportWidgetForPlayer(GetLocalPlayer(), Compass.ToSharedRef());
		}
		Compass.Reset();
	}
	if (PaperDollStage)
	{
		PaperDollStage->Destroy();
		PaperDollStage = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void ACIRLPlayerController::OnGameMenuPressed()
{
	OpenGameMenu(ECIRLMenuTab::Game);
}

void ACIRLPlayerController::OnEquipmentPressed()
{
	OpenGameMenu(ECIRLMenuTab::Equipment);
}

void ACIRLPlayerController::OnMapPressed()
{
	OpenGameMenu(ECIRLMenuTab::Map);
}

void ACIRLPlayerController::OnNextSongPressed()
{
	if (GameMusic)
	{
		GameMusic->Next();
	}
}

void ACIRLPlayerController::OpenGameMenu(ECIRLMenuTab Tab)
{
	if (GameMenu.IsValid())
	{
		GameMenu->SetTab(Tab);
		GameMenu->FocusCurrentTab();
		return;
	}

	UGameViewportClient* Viewport = GetWorld()->GetGameViewport();
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!Viewport || !LocalPlayer)
	{
		return;
	}

	if (!PaperDollStage)
	{
		// Out of sight, 2 km below the world
		FActorSpawnParameters Params;
		Params.Owner = this;
		PaperDollStage = GetWorld()->SpawnActor<ACIRLPaperDollStage>(FVector(0.f, 0.f, -200000.f), FRotator::ZeroRotator, Params);
	}

	// The doll wears the player's arms
	const ABallCharacter* Ball = Cast<ABallCharacter>(GetPawn());
	UTexture2D* PlayerArms = Ball ? Ball->GetFlag() : nullptr;
	const int32 ArmsIndex = CIRLHeraldry::IndexOf(PlayerArms);
	if (PaperDollStage)
	{
		PaperDollStage->SetDollFlag(PlayerArms);
		PaperDollStage->MirrorGear(Ball ? Ball->GetInventory() : nullptr);
	}

	SAssignNew(GameMenu, SCIRLGameMenu)
		.InitialTab(Tab)
		.PaperDollStage(PaperDollStage)
		.CurrentArms(ArmsIndex == INDEX_NONE ? 0 : ArmsIndex)
		.OnArmsChosen(SCIRLGameMenu::FOnArmsChosen::CreateUObject(this, &ACIRLPlayerController::WearArms))
		.OnCloseRequested(SCIRLGameMenu::FOnCloseRequested::CreateUObject(this, &ACIRLPlayerController::CloseGameMenu))
		.OnMainMenuRequested(SCIRLGameMenu::FOnCloseRequested::CreateUObject(this, &ACIRLPlayerController::ReturnToTitle))
		.OnQuitRequested(SCIRLGameMenu::FOnCloseRequested::CreateUObject(this, &ACIRLPlayerController::QuitGame))
		.Audio(UCIRLAudioSubsystem::Get(this))
		.Player(GetPawn())
		.Inventory(Ball ? Ball->GetInventory() : nullptr);
	Viewport->AddViewportWidgetForPlayer(LocalPlayer, GameMenu.ToSharedRef(), 50);

	// Menus can be walked with WASD as well as the arrow keys and the controller
	PreviousNavigation = CIRLMenuNavigation::Push();

	// Keys held while the menu opens would never see their release (Slate gets it), so the ball
	// would keep walking after the menu closes: forget every held key now
	FlushPressedKeys();
	SetPause(true);
	SetShowMouseCursor(true);
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(GameMenu);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	GameMenu->FocusCurrentTab();
}

void ACIRLPlayerController::CloseGameMenu()
{
	if (!GameMenu.IsValid())
	{
		return;
	}

	UGameViewportClient* Viewport = GetWorld() ? GetWorld()->GetGameViewport() : nullptr;
	if (Viewport && GetLocalPlayer())
	{
		Viewport->RemoveViewportWidgetForPlayer(GetLocalPlayer(), GameMenu.ToSharedRef());
	}
	GameMenu.Reset();

	CIRLMenuNavigation::Pop(PreviousNavigation);

	FlushPressedKeys();
	SetPause(false);
	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());
}

void ACIRLPlayerController::OnPlayerDied()
{
	const APlayerBallCharacter* PlayerBall = Cast<APlayerBallCharacter>(GetPawn());
	bDiedInFirstPerson = !PlayerBall || PlayerBall->IsFirstPerson();
	bWasSlain = PlayerBall && PlayerBall->GetLastAttacker() != nullptr;

	// The world's music is cut off; death has its own (if there is any)
	if (GameMusic)
	{
		GameMusic->Stop(0.f);
	}
	if (USoundBase* Sound = GetDefault<UCIRLMusicSettings>()->DeathSound.LoadSynchronous())
	{
		// Its own sound class (Effects) comes from the asset; not auto-destroyed, so respawning can fade it out
		DeathSound = UGameplayStatics::SpawnSound2D(this, Sound, 1.f, 1.f, 0.f, nullptr, false, false);
	}
	const TArray<FCIRLMusicTrack>& Tracks = GetDefault<UCIRLMusicSettings>()->DeathMusic;
	if (DeathMusic && Tracks.Num() > 0)
	{
		const UCIRLAudioSubsystem* Audio = UCIRLAudioSubsystem::Get(this);
		DeathMusic->Play(Tracks, Audio ? Audio->GetGameMusicClass() : nullptr);
	}
	GetWorldTimerManager().SetTimer(DeathScreenTimer, this, &ACIRLPlayerController::ShowDeathScreen, DeathScreenDelay, false);
}

void ACIRLPlayerController::ShowDeathScreen()
{
	UGameViewportClient* Viewport = GetWorld()->GetGameViewport();
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (DeathScreen.IsValid() || !Viewport || !LocalPlayer)
	{
		return;
	}
	if (GameMenu.IsValid())
	{
		CloseGameMenu();
	}

	// The word says how you died (later also FALLEN, for dying in a battle)
	SAssignNew(DeathScreen, SCIRLDeathScreen)
		.Title(bWasSlain ? NSLOCTEXT("CIRLDeath", "Slain", "SLAIN") : NSLOCTEXT("CIRLDeath", "Perished", "PERISHED"))
		.OnRespawn(SCIRLDeathScreen::FOnChosen::CreateUObject(this, &ACIRLPlayerController::Respawn))
		.OnMainMenu(SCIRLDeathScreen::FOnChosen::CreateUObject(this, &ACIRLPlayerController::ReturnToTitle));
	Viewport->AddViewportWidgetForPlayer(LocalPlayer, DeathScreen.ToSharedRef(), 40);

	PreviousNavigation = CIRLMenuNavigation::Push();
	FlushPressedKeys();
	SetShowMouseCursor(true);
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(DeathScreen->GetFocusTarget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void ACIRLPlayerController::HideDeathScreen()
{
	GetWorldTimerManager().ClearTimer(DeathScreenTimer);
	if (!DeathScreen.IsValid())
	{
		return;
	}
	UGameViewportClient* Viewport = GetWorld() ? GetWorld()->GetGameViewport() : nullptr;
	if (Viewport && GetLocalPlayer())
	{
		Viewport->RemoveViewportWidgetForPlayer(GetLocalPlayer(), DeathScreen.ToSharedRef());
	}
	DeathScreen.Reset();
	CIRLMenuNavigation::Pop(PreviousNavigation);

	FlushPressedKeys();
	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());
}

void ACIRLPlayerController::Respawn()
{
	HideDeathScreen();
	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
	if (!GameMode)
	{
		return;
	}

	// The old body is removed while the screen is dark, so nobody sees it vanish (the player loses nothing by dying)
	ABallCharacter* OldBall = Cast<ABallCharacter>(GetPawn());
	UTexture2D* Arms = OldBall ? OldBall->GetFlag() : nullptr;
	UnPossess();
	if (OldBall)
	{
		OldBall->Destroy();
	}

	// For now always at the player start (later: the nearest village, a shrine, or being found by someone)
	const AActor* Start = GameMode->FindPlayerStart(this);
	const FTransform Where = Start ? Start->GetActorTransform() : FTransform(FVector(0.f, 0.f, 200.f));
	FActorSpawnParameters Params;
	// Someone (or your own body) standing on the spot must not stop you coming back
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	APawn* NewPawn = GetWorld()->SpawnActor<APawn>(GameMode->GetDefaultPawnClassForController(this), Where, Params);
	if (!NewPawn)
	{
		return;
	}
	Possess(NewPawn);
	SetControlRotation(FRotator(0.f, Where.Rotator().Yaw, 0.f));

	// Back to life, back to the world's music
	if (DeathMusic)
	{
		DeathMusic->Stop(1.f);
	}
	if (DeathSound)
	{
		// Cut at once: back to life
		DeathSound->Stop();
		DeathSound = nullptr;
	}
	StartGameMusic();

	if (APlayerBallCharacter* PlayerBall = Cast<APlayerBallCharacter>(NewPawn))
	{
		if (Arms)
		{
			PlayerBall->SetFlag(Arms);
		}
		PlayerBall->SetFirstPerson(bDiedInFirstPerson);
	}
}

void ACIRLPlayerController::DevMenu(const FString& Tab)
{
	static const TCHAR* Names[] = { TEXT("Map"), TEXT("Quests"), TEXT("Equipment"), TEXT("Character"), TEXT("Game") };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Names); ++Index)
	{
		if (Tab.Equals(Names[Index], ESearchCase::IgnoreCase))
		{
			OpenGameMenu(static_cast<ECIRLMenuTab>(Index));
			return;
		}
	}
	CloseGameMenu();
}

void ACIRLPlayerController::ReturnToTitle()
{
	CloseGameMenu();
	HideDeathScreen();
	UGameplayStatics::OpenLevel(this, FName(*GetDefault<UWorldSimulationSettings>()->TitleMap.GetLongPackageName()));
}

void ACIRLPlayerController::WearArms(int32 Index)
{
	if (!CIRLHeraldry::All().IsValidIndex(Index))
	{
		return;
	}
	UTexture2D* Arms = CIRLHeraldry::LoadTexture(CIRLHeraldry::All()[Index]);
	if (ABallCharacter* Ball = Cast<ABallCharacter>(GetPawn()))
	{
		Ball->SetFlag(Arms);
	}
	if (PaperDollStage)
	{
		PaperDollStage->SetDollFlag(Arms);
	}
}

void ACIRLPlayerController::QuitGame()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
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
