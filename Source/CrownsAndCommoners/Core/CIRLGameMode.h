// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CIRLGameMode.generated.h"

/**
 *  The main game mode: spawns the player as a ball with the game's player controller.
 */
UCLASS()
class ACIRLGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ACIRLGameMode();
};
