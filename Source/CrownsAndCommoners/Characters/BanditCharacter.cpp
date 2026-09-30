// Crowns & Commoners

#include "Characters/BanditCharacter.h"
#include "Characters/AI/BallFighterController.h"

ABanditCharacter::ABanditCharacter()
{
	AIControllerClass = ABallFighterController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	StartingEmotion = EBallEmotion::Suspicious;
	StartingGear = { TEXT("tunic_plain_wool"), TEXT("gambeson_padded_jack"), TEXT("hood_wool"), TEXT("boots_ankle"), TEXT("weapon_cudgel") };
}
