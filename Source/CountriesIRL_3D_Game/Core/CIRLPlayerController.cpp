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
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/NavigationConfig.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/SCIRLGameMenu.h"

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

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(InputConfig->GameMenu, ETriggerEvent::Started, this, &ACIRLPlayerController::OnGameMenuPressed);
		EIC->BindAction(InputConfig->Equipment, ETriggerEvent::Started, this, &ACIRLPlayerController::OnEquipmentPressed);
	}
}

void ACIRLPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Leaving the game with the menu open (e.g. stopping Play in the editor): put Slate back the way it was
	if (GameMenu.IsValid())
	{
		CloseGameMenu();
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

	SAssignNew(GameMenu, SCIRLGameMenu)
		.InitialTab(Tab)
		.OnCloseRequested(SCIRLGameMenu::FOnCloseRequested::CreateUObject(this, &ACIRLPlayerController::CloseGameMenu))
		.OnQuitRequested(SCIRLGameMenu::FOnCloseRequested::CreateUObject(this, &ACIRLPlayerController::QuitGame));
	Viewport->AddViewportWidgetForPlayer(LocalPlayer, GameMenu.ToSharedRef(), 50);

	// Menus can be walked with WASD as well as the arrow keys and the controller
	FSlateApplication& Slate = FSlateApplication::Get();
	PreviousNavigation = Slate.GetNavigationConfig();
	TSharedRef<FNavigationConfig> Navigation = MakeShared<FNavigationConfig>();
	Navigation->KeyEventRules.Emplace(EKeys::W, EUINavigation::Up);
	Navigation->KeyEventRules.Emplace(EKeys::S, EUINavigation::Down);
	Navigation->KeyEventRules.Emplace(EKeys::A, EUINavigation::Left);
	Navigation->KeyEventRules.Emplace(EKeys::D, EUINavigation::Right);
	Slate.SetNavigationConfig(Navigation);

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

	if (PreviousNavigation.IsValid() && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().SetNavigationConfig(PreviousNavigation.ToSharedRef());
		PreviousNavigation.Reset();
	}

	SetPause(false);
	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());
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
