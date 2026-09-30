// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CIRLPlayerController.generated.h"

class UCIRLInputConfig;
class SCIRLGameMenu;
class SCIRLDeathScreen;
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

	/** The player's ball has died: after a moment the death screen offers to respawn */
	void OnPlayerDied();

	/** A new ball at the player start, with the arms and view of the one that died; the old body is removed */
	UFUNCTION(Exec)
	void Respawn();

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

	void ShowDeathScreen();
	void HideDeathScreen();

	TSharedPtr<SCIRLDeathScreen> DeathScreen;

	/** The compass bar at the top of the screen */
	TSharedPtr<SWidget> Compass;
	FTimerHandle DeathScreenTimer;

	/** Seconds between dying and the death screen starting to fade in (the body falls while it does) */
	float DeathScreenDelay = 0.2f;

	/** How the player was looking when they died, for the new ball */
	bool bDiedInFirstPerson = true;

	/** Killed by someone (SLAIN) rather than just dying (PERISHED) */
	bool bWasSlain = false;

	/** The studio that films the 3D character for the Equipment tab (made the first time the menu opens) */
	UPROPERTY(Transient)
	TObjectPtr<ACIRLPaperDollStage> PaperDollStage;

	/** Music playing in the world */
	UPROPERTY(Transient)
	TObjectPtr<class UCIRLPlaylistComponent> GameMusic;

	/** The death sound effect while it plays (faded out on respawn) */
	UPROPERTY(Transient)
	TObjectPtr<class UAudioComponent> DeathSound;

	/** Music while dead (the world's music stops) */
	UPROPERTY(Transient)
	TObjectPtr<class UCIRLPlaylistComponent> DeathMusic;

	/** Starts the world's music from its first track */
	void StartGameMusic();

	/** Slate's navigation rules from before the menu opened (the menu adds WASD) */
	TSharedPtr<FNavigationConfig> PreviousNavigation;
};
