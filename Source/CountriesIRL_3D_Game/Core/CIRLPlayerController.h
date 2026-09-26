// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CIRLPlayerController.generated.h"

class UCIRLInputConfig;

/**
 *  The game's player controller. Owns the input configuration and registers it with Enhanced Input.
 */
UCLASS()
class ACIRLPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	const UCIRLInputConfig* GetInputConfig() const { return InputConfig; }

protected:

	virtual void PostInitializeComponents() override;
	virtual void SetupInputComponent() override;

private:

	UPROPERTY(Transient)
	TObjectPtr<UCIRLInputConfig> InputConfig;
};
