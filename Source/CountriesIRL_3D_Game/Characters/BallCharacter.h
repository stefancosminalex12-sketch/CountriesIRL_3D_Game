// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Characters/BallFaceComponent.h"
#include "BallCharacter.generated.h"

class UStaticMeshComponent;
class UBallAnimatorComponent;
class UStaminaComponent;
class UHealthComponent;
class UCorpseComponent;
class UBallSkeletonComponent;
class UBallMeleeComponent;
class AHorse;

/**
 *  Base class for every person in the game: a countryball with eyes, floating hands and feet.
 *  The player, villagers and soldiers all derive from this.
 */
UCLASS()
class ABallCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	ABallCharacter();

	UFUNCTION(BlueprintCallable, Category="Ball")
	void SetEmotion(EBallEmotion NewEmotion);

	UFUNCTION(BlueprintPure, Category="Ball")
	EBallEmotion GetEmotion() const;

	/** Whether the ball wants to run. It only actually runs while it has stamina. */
	UFUNCTION(BlueprintCallable, Category="Ball|Movement")
	void SetSprinting(bool bNewSprinting);

	UFUNCTION(BlueprintPure, Category="Ball|Movement")
	bool IsRunning() const { return bRunning; }

	UStaminaComponent* GetStamina() const { return Stamina; }

	UHealthComponent* GetHealth() const { return Health; }

	UBallMeleeComponent* GetMelee() const { return Melee; }

	UBallAnimatorComponent* GetAnimator() const { return Animator; }

	UFUNCTION(BlueprintPure, Category="Ball")
	bool IsDead() const;

	/** Gets on a free horse. Returns true if now riding it. */
	bool Mount(AHorse* Horse);

	/** Gets off the horse, landing beside it */
	void Dismount();

	AHorse* GetMount() const { return MountedHorse; }
	bool IsMounted() const { return MountedHorse != nullptr; }

	/** Wants to go fast: runs on foot, gallops on horseback */
	bool WantsToRun() const { return bWantsToRun; }

	/** All damage goes through here (combat, falls, fire...) */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	float GetBallRadius() const { return BallRadius; }

	/** Half the ball's height (equals the radius for the sphere; BallHeightScale can stretch it) */
	float GetBallHalfHeight() const { return BallRadius * BallHeightScale; }

	float GetRunSpeed() const { return RunSpeed; }

	/** Height of the ball's center relative to the capsule center */
	float GetBallCenterZ() const;

	/** Distance from the capsule center down to the ground */
	float GetGroundOffset() const;

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void OnJumped_Implementation() override;

	/** Visual root: everything that is drawn hangs from here */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> VisualRoot;

	/** Moves and tilts the ball body and face together (bob, lean) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USceneComponent> BodyPivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBallFaceComponent> Face;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBallAnimatorComponent> Animator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaminaComponent> Stamina;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UHealthComponent> Health;

	/** Decay after death (pale, flies, rotting, bones) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UCorpseComponent> Corpse;

	/** Cartoon bones shown once the corpse has decayed to the skeleton stage */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBallSkeletonComponent> Skeleton;

	/** Unarmed fighting (punches); weapons build on this later */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBallMeleeComponent> Melee;

	/** Solid body once dead: others bump into it and can climb or stand on it (a box around the lying ball, then around the bones) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<class UBoxComponent> CorpseCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<class USphereComponent> SkullCollision;

	/** Turns on corpse collision for the current decay stage (ball lying down, or bones) */
	void UpdateCorpseCollision(bool bBones);

	/** Applies base colors, decay tint and the damage flash to body, hands and feet */
	void RefreshColors();

	/** Health reached zero: x_x eyes, stop moving. (Player reload/capture flow comes later.) */
	UFUNCTION()
	virtual void HandleDeath(UHealthComponent* DepletedHealth);

	/**
	 *  Body proportions (Master file > Characters & Art): ball 58% of the height, empty gap 29%, boots 13%.
	 *  Defaults are the average English man of 1455: 1.71 m = 1.00 m ball + 0.49 m gap + 0.22 m boots.
	 */

	/** Ball radius in cm, side to side */
	UPROPERTY(VisibleAnywhere, Category="Ball")
	float BallRadius = 50.f;

	/** How much taller than wide the ball is (1 = perfect sphere, the chosen look) */
	UPROPERTY(VisibleAnywhere, Category="Ball")
	float BallHeightScale = 1.f;

	/** Distance from the ground to the bottom of the ball: the boots (22 cm) plus the empty gap above them (49 cm) */
	UPROPERTY(VisibleAnywhere, Category="Ball")
	float FeetGap = 71.f;

	/** Placeholder color until flags/liveries are textured on the ball */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ball|Look")
	FLinearColor BodyColor = FLinearColor(0.55f, 0.43f, 0.30f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ball|Look")
	FLinearColor HandColor = FLinearColor(0.85f, 0.82f, 0.75f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ball|Look")
	FLinearColor FootColor = FLinearColor(0.18f, 0.11f, 0.06f);

	/** Emotion shown when the ball spawns */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ball|Look")
	EBallEmotion StartingEmotion = EBallEmotion::Neutral;

	UPROPERTY(EditAnywhere, Category="Ball|Movement")
	float WalkSpeed = 220.f;

	UPROPERTY(EditAnywhere, Category="Ball|Movement")
	float RunSpeed = 400.f;

	UPROPERTY(EditAnywhere, Category="Ball|Movement")
	float JumpVelocity = 340.f;

	/** Stamina per second while running */
	UPROPERTY(EditAnywhere, Category="Ball|Movement")
	float RunStaminaCost = 12.f;

	/** Stamina per second while galloping on a horse: riding hard tires you too, as slowly as holding the guard */
	UPROPERTY(EditAnywhere, Category="Ball|Movement")
	float RidingStaminaCost = 1.5f;

	/** Stamina per jump */
	UPROPERTY(EditAnywhere, Category="Ball|Movement")
	float JumpStaminaCost = 15.f;

	/** Seconds the ball stays tinted red after taking damage */
	UPROPERTY(EditAnywhere, Category="Ball|Look")
	float DamageFlashDuration = 0.35f;

	UPROPERTY(EditAnywhere, Category="Ball|Look")
	FLinearColor DamageFlashColor = FLinearColor(1.f, 0.06f, 0.04f);

	UPROPERTY(EditAnywhere, Category="Ball|Look", meta=(ClampMin=0, ClampMax=1))
	float DamageFlashStrength = 0.65f;

	float DamageFlashTime = 0.f;

	/** Run key held */
	bool bWantsToRun = false;

	UPROPERTY(Transient)
	TObjectPtr<AHorse> MountedHorse;

	/** Keeps the ball sitting in the saddle as the horse's back moves */
	void UpdateSeat();

	/** Actually running this frame (wants to, has stamina, moving on the ground) */
	bool bRunning = false;
};
