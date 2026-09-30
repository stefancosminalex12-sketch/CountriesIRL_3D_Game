// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CIRLPlayerController.generated.h"

class UCIRLInputConfig;
class SCIRLGameMenu;
class ACIRLPaperDollStage;
class FNavigationConfig;
enum class ECIRLMenuTab : uint8;

/**
 *  The game's player controller. Owns the input configuration and registers it with Enhanced Input.
 */
UCLASS()
class ACIRLPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	const UCIRLInputConfig* GetInputConfig() const { return InputConfig; }

	/** Opens the game menu on a tab (or switches tab if it's already open); pauses the game */
	void OpenGameMenu(ECIRLMenuTab Tab);

	/** Closes the menu and goes back to the game */
	void CloseGameMenu();

	bool IsGameMenuOpen() const { return GameMenu.IsValid(); }

	/** Console (testing): jump to a time of day. DevTime 21.5 = 21:30 */
	UFUNCTION(Exec)
	void DevTime(float Hours);

	/** Console (testing): speed up time. DevTimeSpeed 60 = a day passes in under a minute; 1 = normal */
	UFUNCTION(Exec)
	void DevTimeSpeed(float Multiplier);

	/** Console (testing): show/hide the date and time on screen */
	UFUNCTION(Exec)
	void DevClock();

	/** Console (testing): skip time forward. DevAdvance 24 = one day later */
	UFUNCTION(Exec)
	void DevAdvance(float Hours);

	/** Console (testing): jump to a day of the current year, keeping the time. DevDate 25 12 = Christmas */
	UFUNCTION(Exec)
	void DevDate(int32 Day, int32 Month);

	/** Console (testing): jump to the same day in another year. DevYear 1461 */
	UFUNCTION(Exec)
	void DevYear(int32 Year);

	/** Console (testing): force weather: Clear, Fair, Cloudy, Overcast, Showers, Rain, HeavyRain, Storm, Snow, Fog, or Auto */
	UFUNCTION(Exec)
	void DevWeather(const FString& Weather);

	/** Console (testing): damage the horse you ride, or the nearest horse. DevHitHorse 1000 kills it */
	UFUNCTION(Exec)
	void DevHitHorse(float Amount);

	/** Console (testing): open the menu on a tab (Map, Quests, Equipment, Character, Game), or close it with no tab */
	UFUNCTION(Exec)
	void DevMenu(const FString& Tab);

	/** Console (testing): damage the nearest other ball. DevHitNearest 1000 kills it */
	UFUNCTION(Exec)
	void DevHitNearest(float Amount);

protected:

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	UPROPERTY(Transient)
	TObjectPtr<UCIRLInputConfig> InputConfig;

	void OnGameMenuPressed();
	void OnEquipmentPressed();
	void OnMapPressed();
	void OnNextSongPressed();
	void QuitGame();
	void ReturnToTitle();

	/** The player picked a coat of arms (index into CIRLHeraldry::All()): the ball and the menu's doll wear it */
	void WearArms(int32 Index);

	TSharedPtr<SCIRLGameMenu> GameMenu;

	/** The studio that films the 3D character for the Equipment tab (made the first time the menu opens) */
	UPROPERTY(Transient)
	TObjectPtr<ACIRLPaperDollStage> PaperDollStage;

	/** Music playing in the world */
	UPROPERTY(Transient)
	TObjectPtr<class UCIRLPlaylistComponent> GameMusic;

	/** Slate's navigation rules from before the menu opened (the menu adds WASD) */
	TSharedPtr<FNavigationConfig> PreviousNavigation;
};
