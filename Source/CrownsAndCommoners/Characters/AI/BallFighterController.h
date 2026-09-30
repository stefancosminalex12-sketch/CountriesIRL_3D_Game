// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BallFighterController.generated.h"

class ABallCharacter;

/** What a fighting ball is doing */
UENUM()
enum class EBallFighterState : uint8
{
	/** Standing at its post, watching */
	Idle,
	/** Has an enemy: closes in, strikes, guards */
	Fight,
	/** Lost or beat its enemy: walks back to its post */
	Return
};

/**
 *  The brain of a ball that fights on foot (bandits now; guards and soldiers later). It fights with exactly what the
 *  player has: the ball's melee component, so its weapon's reach, speed and stamina cost, its guard and its armour all
 *  count the same way.
 *
 *  It stands at its post until it sees an enemy (in front of it, or very close on any side, with nothing in the
 *  way) or is hit by one. Then it runs in, keeps just inside striking distance, circles, strikes when it can, and
 *  sometimes raises its guard when the enemy swings. It gives up when the enemy gets too far away or it has been led
 *  too far from its post, and walks back.
 *
 *  It walks straight at things: there is no path finding yet (that comes with the real landscape).
 */
UCLASS()
class ABallFighterController : public AAIController
{
	GENERATED_BODY()

public:

	ABallFighterController();

	virtual void Tick(float DeltaTime) override;

	EBallFighterState GetState() const { return State; }

protected:

	virtual void OnPossess(APawn* InPawn) override;

	/** Who this ball attacks on sight. For now: the player (sides and reputation decide this later) */
	virtual bool IsEnemy(const ABallCharacter* Other) const;

	/** How far it sees an enemy in front of it, in cm */
	UPROPERTY(EditAnywhere, Category="Fighter|Senses")
	float SightRange = 1500.f;

	/** How wide it sees: the cosine of the angle to each side of straight ahead (0.17 is about 80 degrees) */
	UPROPERTY(EditAnywhere, Category="Fighter|Senses")
	float SightCosine = 0.17f;

	/** Closer than this it notices an enemy on any side, even behind it */
	UPROPERTY(EditAnywhere, Category="Fighter|Senses")
	float NoticeRange = 400.f;

	/** It gives up the chase when the enemy is further away than this */
	UPROPERTY(EditAnywhere, Category="Fighter|Senses")
	float GiveUpRange = 3000.f;

	/** ...or when it has been led further than this from its post */
	UPROPERTY(EditAnywhere, Category="Fighter|Senses")
	float LeashRange = 4500.f;

	/** Further from the enemy than this, it runs */
	UPROPERTY(EditAnywhere, Category="Fighter|Fight")
	float RunBeyond = 450.f;

	/** Seconds it waits between its blows (on top of the weapon's own recovery), picked in this range */
	UPROPERTY(EditAnywhere, Category="Fighter|Fight")
	FVector2D StrikePause = FVector2D(0.35f, 1.1f);

	/** How often it answers an enemy's swing by raising its guard (0 never, 1 always) */
	UPROPERTY(EditAnywhere, Category="Fighter|Fight", meta=(ClampMin=0, ClampMax=1))
	float GuardChance = 0.45f;

	/** Seconds it needs to react to a swing */
	UPROPERTY(EditAnywhere, Category="Fighter|Fight")
	float ReactionTime = 0.12f;

	/** Seconds it keeps the guard up, picked in this range */
	UPROPERTY(EditAnywhere, Category="Fighter|Fight")
	FVector2D GuardTime = FVector2D(0.5f, 1.0f);

	/** How hard it circles its enemy between blows (share of full walking input) */
	UPROPERTY(EditAnywhere, Category="Fighter|Fight")
	float CircleInput = 0.45f;

private:

	UFUNCTION()
	void HandleDamaged(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	ABallCharacter* FindEnemy(const ABallCharacter* Ball) const;
	void StartFight(ABallCharacter* Ball, ABallCharacter* Enemy);
	void StopFight(ABallCharacter* Ball, bool bWon);
	void TickFight(ABallCharacter* Ball, float DeltaTime);
	void TickReturn(ABallCharacter* Ball);

	/** Faces the enemy while fighting, and the way it walks otherwise */
	void SetFacesEnemy(ABallCharacter* Ball, bool bFaceEnemy);

	EBallFighterState State = EBallFighterState::Idle;

	UPROPERTY(Transient)
	TObjectPtr<ABallCharacter> Target;

	/** Its post: where it stood when play began, and which way it faced */
	FVector HomeLocation = FVector::ZeroVector;
	FRotator HomeRotation = FRotator::ZeroRotator;

	float LookTimer = 0.f;
	float StrikeTimer = 0.f;
	float GuardDelay = -1.f;
	float GuardLeft = 0.f;
	float CircleLeft = 0.f;
	float CircleSide = 1.f;
	/** The enemy's current swing has been noticed (and answered or not) */
	bool bSawSwing = false;
};
