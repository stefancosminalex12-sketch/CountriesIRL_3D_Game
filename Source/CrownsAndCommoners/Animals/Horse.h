// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Animals/MountDefinition.h"
#include "Horse.generated.h"

class UStaminaComponent;
class UHorseSoundComponent;
class UHealthComponent;
class ABallCharacter;

/**
 *  A rideable horse. Moves like a horse: it turns gradually (never slides sideways), builds up and
 *  loses speed smoothly, walks, trots (the travelling pace, free), canters or gallops on its own stamina.
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
	UHealthComponent* GetHealth() const { return Health; }
	bool IsDead() const;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	ABallCharacter* GetRider() const { return Rider; }
	bool CanBeMounted() const { return !Rider && Definition && !IsDead(); }

	/** Called by the rider when getting on (pass nullptr when getting off) */
	void SetRider(ABallCharacter* NewRider);

	/** Rider's reins: where to go (world direction, length 0..1) and at which gait */
	void SetRiderInput(const FVector& Direction, EHorseGait Gait);

	/** Rider presses jump: standing still the horse rears up on its hind legs and neighs, moving it jumps */
	void RiderJump();

	/** Rears up on the hind legs and neighs (the rider stays on). With bThrowRider it throws the rider off at the top.
	 *  Returns true if it started */
	bool Rear(bool bThrowRider = false);

	bool IsRearing() const { return RearTime >= 0.f; }

	/** How far up the front of the horse is right now (degrees); the rider leans with it */
	float GetRearPitch() const;

	/** Where the rider's seat is, in this actor's space (follows the back as the horse moves) */
	FVector GetSaddleOffset() const;

	float GetBodyHalfWidth() const;

	/** The gait the horse is actually in (canter and gallop drop down a gait when out of breath) */
	EHorseGait GetGait() const { return Gait; }
	bool IsGalloping() const { return Gait == EHorseGait::Gallop; }

protected:

	virtual bool CanJumpInternal_Implementation() const override;
	virtual void OnJumped_Implementation() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Horse")
	TObjectPtr<UMountDefinition> Definition;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaminaComponent> Stamina;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UHealthComponent> Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UHorseSoundComponent> Sounds;

	/** Health ran out: the rider is thrown off and the horse falls and stays there */
	UFUNCTION()
	void HandleDeath(UHealthComponent* DepletedHealth);

private:

	/** Sets up mesh, animation, collision and movement from the definition */
	void ApplyDefinition();

	/** Is there something right in front of the horse's head? */
	bool IsBlockedAhead() const;

	UPROPERTY(Transient)
	TObjectPtr<ABallCharacter> Rider;

	FVector DesiredDirection = FVector::ZeroVector;
	EHorseGait RequestedGait = EHorseGait::Walk;
	EHorseGait Gait = EHorseGait::Walk;

	/** Rearing: seconds into it (-1 = not rearing), and whether the rider is still to be thrown at the top */
	float RearTime = -1.f;
	bool bThrowRiderAtTop = false;

	/** Out of breath, the horse falls back to a trot; the rider has to ease off (ask for a walk or trot) before
	 *  asking for more again, and asking again while it's still out of breath gets them thrown */
	bool bRiderEasedOff = false;

	/** The mesh's place when standing (rearing tilts it up around the hind hooves) */
	FVector MeshBaseLocation = FVector::ZeroVector;
	FRotator MeshBaseRotation = FRotator::ZeroRotator;

	/** 0 standing .. 1 fully up, over the rear's duration */
	float RearAmount() const;
	void UpdateRear(float DeltaTime);
	void ThrowRider();

};
