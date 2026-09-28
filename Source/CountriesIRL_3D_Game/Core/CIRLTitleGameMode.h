// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "CIRLTitleGameMode.generated.h"

class SCIRLTitleScreen;
class FNavigationConfig;

/**
 *  Game mode of the title-screen level (L_MainMenu): no character, just the title screen.
 *  Set as the level's GameMode Override in its World Settings.
 */
UCLASS()
class ACIRLTitleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ACIRLTitleGameMode();
};

/**
 *  Shows the title screen (UI/SCIRLTitleScreen) and handles its buttons:
 *  New Game opens the world (Project Settings > CountriesIRL World > New Game Map), Quit leaves the game.
 */
UCLASS()
class ACIRLTitleController : public APlayerController
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	void StartNewGame();
	void Quit();

	TSharedPtr<SCIRLTitleScreen> TitleScreen;
	TSharedPtr<FNavigationConfig> PreviousNavigation;
};
