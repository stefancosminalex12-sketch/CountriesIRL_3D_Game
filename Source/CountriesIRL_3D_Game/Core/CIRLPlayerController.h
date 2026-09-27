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

	/** Console (testing): damage the nearest other ball. DevHitNearest 1000 kills it */
	UFUNCTION(Exec)
	void DevHitNearest(float Amount);

	/** Console (testing): switch every ball between countryball and ball-head-on-a-body */
	UFUNCTION(Exec)
	void DevBodyAll();

protected:

	virtual void PostInitializeComponents() override;
	virtual void SetupInputComponent() override;

private:

	UPROPERTY(Transient)
	TObjectPtr<UCIRLInputConfig> InputConfig;
};
