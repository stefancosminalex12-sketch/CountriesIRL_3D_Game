// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BallMeleeComponent.generated.h"

class ABallCharacter;

/** Where a strike landed on a ball. Strikes to the head/face hurt most, the chest a lot, lower less. */
UENUM(BlueprintType)
enum class EBallHitZone : uint8
{
	Head,
	Chest,
	Lower
};

/**
 *  Unarmed fighting for any ball (player now, villagers and bandits later): jabs that alternate hands,
 *  hit what the ball is aiming at within reach, and deal damage by hit zone. Weapons will build on this.
 */
UCLASS(ClassGroup=(Ball), meta=(BlueprintSpawnableComponent))
class UBallMeleeComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UBallMeleeComponent();

	/** Starts a punch if not dead, not on cooldown and enough stamina. Returns true if it started. */
	UFUNCTION(BlueprintCallable, Category="Melee")
	bool TryPunch();

	bool IsPunching() const { return PunchTime >= 0.f; }

	/** Hold to keep the guard up (fists in front of the face) */
	UFUNCTION(BlueprintCallable, Category="Melee")
	void SetGuarding(bool bNewWantsGuard) { bWantsGuard = bNewWantsGuard; }

	bool WantsGuard() const { return bWantsGuard; }

	/** Guard is up: wanted, alive and with stamina left to hold it */
	UFUNCTION(BlueprintPure, Category="Melee")
	bool IsGuarding() const;

	/** Blocks hits from the front while guarding: less damage, costs stamina. Returns the damage that gets through. */
	float ModifyIncomingDamage(float Damage, const AActor* DamageCauser);

	/** Movement speed multiplier while guarding */
	float GetGuardMoveSpeedScale() const { return GuardMoveSpeedScale; }

	/**
	 *  For the animator: which hand (0 left, 1 right) and where the fist is in the punch:
	 *  negative during the wind-up (down to -WindUpPull), rising fast to 1 at impact, then back to 0.
	 */
	float GetPunchExtension(int32& OutHand) const;

	/** 0..1..0 over the whole punch, for secondary motion (the other fist guarding, camera nudge) */
	float GetPunchEnvelope() const;

	float GetWindUpPull() const { return WindUpPull; }

	/** Which zone a point on a target ball falls in */
	static EBallHitZone ZoneForPoint(const ABallCharacter* Target, const FVector& WorldPoint);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	/** Base unarmed damage, picked randomly in this range */
	UPROPERTY(EditAnywhere, Category="Melee|Damage")
	FVector2D PunchDamage = FVector2D(5.f, 8.f);

	UPROPERTY(EditAnywhere, Category="Melee|Damage")
	float HeadMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, Category="Melee|Damage")
	float ChestMultiplier = 1.f;

	UPROPERTY(EditAnywhere, Category="Melee|Damage")
	float LowerMultiplier = 0.7f;

	/** Reach from the eyes, in cm */
	UPROPERTY(EditAnywhere, Category="Melee")
	float Reach = 130.f;

	/** Width of the fist sweep */
	UPROPERTY(EditAnywhere, Category="Melee")
	float HitRadius = 14.f;

	/** Seconds of wind-up (the fist pulls back before striking) */
	UPROPERTY(EditAnywhere, Category="Melee")
	float WindUpTime = 0.07f;

	/** How far back the wind-up goes, as a fraction of the strike */
	UPROPERTY(EditAnywhere, Category="Melee")
	float WindUpPull = 0.3f;

	/** Seconds from starting the punch to the fist landing */
	UPROPERTY(EditAnywhere, Category="Melee")
	float ImpactTime = 0.15f;

	/** Seconds for a whole punch (wind-up, strike and recoil) */
	UPROPERTY(EditAnywhere, Category="Melee")
	float PunchDuration = 0.38f;

	/** Seconds between punches */
	UPROPERTY(EditAnywhere, Category="Melee")
	float Cooldown = 0.45f;

	UPROPERTY(EditAnywhere, Category="Melee")
	float StaminaCost = 8.f;

	/** How hard a punch shoves the target (cm/s) */
	UPROPERTY(EditAnywhere, Category="Melee")
	float Knockback = 220.f;

	/** Stamina per second while holding the guard up (stamina does not refill meanwhile) */
	UPROPERTY(EditAnywhere, Category="Melee|Guard")
	float GuardStaminaPerSecond = 1.5f;

	/** Share of damage that gets through a guard */
	UPROPERTY(EditAnywhere, Category="Melee|Guard", meta=(ClampMin=0, ClampMax=1))
	float BlockDamageMultiplier = 0.35f;

	/** Stamina per blocked hit; without enough, the hit gets through unblocked */
	UPROPERTY(EditAnywhere, Category="Melee|Guard")
	float BlockStaminaCost = 6.f;

	/** Hits within this angle of straight ahead can be blocked (cosine; 0.3 is about 70 degrees to each side) */
	UPROPERTY(EditAnywhere, Category="Melee|Guard")
	float BlockCosine = 0.3f;

	UPROPERTY(EditAnywhere, Category="Melee|Guard")
	float GuardMoveSpeedScale = 0.65f;

private:

	void ResolveHit();
	float MultiplierFor(EBallHitZone Zone) const;

	float PunchTime = -1.f;
	float CooldownLeft = 0.f;
	int32 PunchHand = 1;
	bool bHitResolved = false;
	bool bWantsGuard = false;
};
