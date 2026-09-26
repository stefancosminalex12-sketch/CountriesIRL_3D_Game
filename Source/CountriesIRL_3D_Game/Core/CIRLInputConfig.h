// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CIRLInputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

/**
 *  Builds the game's input actions and default key bindings in code.
 *  Owned by the player controller; pawns read the actions from it when binding input.
 *  Keeping bindings in one place makes it easy to add new actions as systems are added.
 */
UCLASS()
class UCIRLInputConfig : public UObject
{
	GENERATED_BODY()

public:

	/** Creates all actions and the default mapping context. */
	void Build();

	UPROPERTY() TObjectPtr<UInputMappingContext> DefaultContext;

	UPROPERTY() TObjectPtr<UInputAction> Move;
	UPROPERTY() TObjectPtr<UInputAction> Look;
	UPROPERTY() TObjectPtr<UInputAction> Jump;
	UPROPERTY() TObjectPtr<UInputAction> Sprint;
	UPROPERTY() TObjectPtr<UInputAction> ToggleView;
	UPROPERTY() TObjectPtr<UInputAction> Attack;
	UPROPERTY() TObjectPtr<UInputAction> Guard;

	/** Debug: cycles through the ball's eye emotions */
	UPROPERTY() TObjectPtr<UInputAction> CycleEmotion;
};
