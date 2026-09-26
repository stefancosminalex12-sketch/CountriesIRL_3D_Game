// CountriesIRL 3D Game

#include "Core/CIRLGameMode.h"
#include "Core/CIRLPlayerController.h"
#include "Characters/PlayerBallCharacter.h"

ACIRLGameMode::ACIRLGameMode()
{
	DefaultPawnClass = APlayerBallCharacter::StaticClass();
	PlayerControllerClass = ACIRLPlayerController::StaticClass();
}
