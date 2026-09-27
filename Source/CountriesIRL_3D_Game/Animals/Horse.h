// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Horse.generated.h"

class UMountDefinition;
class UStaminaComponent;
class ABallCharacter;

/**
 *  A rideable horse. Moves like a horse: it turns gradually (never slides sideways), builds up and
 *  loses speed smoothly, walks or gallops, and gallops on its own stamina.
 *  Without a rider it just stands (an AI controller is attached for grazing/wandering later).
 *  What kind of horse it is comes from its UMountDefinition.
 */
UCLASS()
class AHorse : public ACharacter
{
	GENERATED_BODY()

public:

	AHorse();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	const UMountDefinition* GetDefinition() const { return Definition; }
	UStaminaComponent* GetStamina() const { return Stamina; }

	ABallCharacter* GetRider() const { return Rider; }
	bool CanBeMounted() const { return !Rider && Definition; }

	/** Called by the rider when getting on (pass nullptr when getting off) */
	void SetRider(ABallCharacter* NewRider);

	/** Rider's reins: where to go (world direction, length 0..1) and whether to gallop */
	void SetRiderInput(const FVector& Direction, bool bGallop);

	/** Rider asks for a jump */
	void RiderJump();

	/** Where the rider's seat is, in this actor's space (follows the back as the horse moves) */
	FVector GetSaddleOffset() const;

	float GetBodyHalfWidth() const;

	bool IsGalloping() const { return bGalloping; }

protected:

	virtual bool CanJumpInternal_Implementation() const override;
	virtual void OnJumped_Implementation() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horse")
	TObjectPtr<UMountDefinition> Definition;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaminaComponent> Stamina;

private:

	/** Sets up mesh, animation, collision and movement from the definition */
	void ApplyDefinition();

	/** Is there something right in front of the horse's head? */
	bool IsBlockedAhead() const;

	UPROPERTY(Transient)
	TObjectPtr<ABallCharacter> Rider;

	FVector DesiredDirection = FVector::ZeroVector;
	bool bWantsGallop = false;
	bool bGalloping = false;
};
