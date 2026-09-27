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

	UFUNCTION(BlueprintPure, Category="Ball")
	bool IsDead() const;

	/** All damage goes through here (combat, falls, fire...) */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	float GetBallRadius() const { return BallRadius; }

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

	/** Applies base colors, decay tint and the damage flash to body, hands and feet */
	void RefreshColors();

	/** Health reached zero: x_x eyes, stop moving. (Player reload/capture flow comes later.) */
	UFUNCTION()
	virtual void HandleDeath(UHealthComponent* DepletedHealth);

	/** Ball radius in cm */
	UPROPERTY(VisibleAnywhere, Category="Ball")
	float BallRadius = 50.f;

	/** Gap between the bottom of the ball and the ground, where the feet float */
	UPROPERTY(VisibleAnywhere, Category="Ball")
	float FeetGap = 25.f;

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

	/** Actually running this frame (wants to, has stamina, moving on the ground) */
	bool bRunning = false;
};
