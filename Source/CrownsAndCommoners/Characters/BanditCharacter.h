// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Characters/BallCharacter.h"
#include "BanditCharacter.generated.h"

/**
 *  A bandit: a ball that attacks the player on sight (its brain is ABallFighterController) with whatever it carries.
 *  The default is a poor outlaw: padded jack, hood and a cudgel. Set StartingGear on a placed bandit to arm it
 *  differently. (Kinds of people become data when villagers and soldiers arrive; this is the first of them.)
 */
UCLASS()
class ABanditCharacter : public ABallCharacter
{
	GENERATED_BODY()

public:

	ABanditCharacter();

protected:

	/** Its gear is its own, not one of the test sets */
	virtual bool GetsTestGear() const override { return false; }
};
