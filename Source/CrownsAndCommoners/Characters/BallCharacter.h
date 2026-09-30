// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Characters/BallFaceComponent.h"
#include "BallCharacter.generated.h"

class UStaticMeshComponent;
class UBallAnimatorComponent;
class UStaminaComponent;
class UCIRLInventoryComponent;
class UBallHeldItemsComponent;
class UBallWornGearComponent;
enum class ECIRLDamageType : uint8;
enum class EBallHitZone : uint8;
class UHealthComponent;
class UCorpseComponent;
class UBallSkeletonComponent;
class UBallMeleeComponent;
class AHorse;
class UTexture2D;
class UMaterialInstanceDynamic;
class UMaterialInterface;

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

	/** Whether the ball wants to sneak: crouched low, slow and quiet (hold). It can't run meanwhile */
	UFUNCTION(BlueprintCallable, Category="Ball|Movement")
	void SetSneaking(bool bNewSneaking) { bWantsToSneak = bNewSneaking; }

	/** Crouched and sneaking right now (wants to, alive, on its own feet) */
	UFUNCTION(BlueprintPure, Category="Ball|Movement")
	bool IsSneaking() const;

	/** Knows Other is there and is dealing with them (fighting them). Someone unaware can be stabbed from behind */
	bool IsAwareOf(const ABallCharacter* Other) const;

	/** A dagger in the neck from behind: dead at once, whatever the armour */
	void Assassinated(ABallCharacter* By);

	UStaminaComponent* GetStamina() const { return Stamina; }
	UBallHeldItemsComponent* GetHeldItems() const { return HeldItems; }
	UCIRLInventoryComponent* GetInventory() const { return Inventory; }
	UBallWornGearComponent* GetWornGear() const { return WornGear; }

	/** The ball's centre: the body, face and worn gear hang from it and lean, bob and fall with it */
	USceneComponent* GetBodyPivot() const { return BodyPivot; }

	/** Gloves and boots recolour the hands and boots (unset = bare hands, the ball's own boots) */
	void SetGearTints(const TOptional<FLinearColor>& Gloves, const TOptional<FLinearColor>& Boots);

	/**
	 *  A blow from a fist or a weapon lands on this ball: the armour worn over that part takes its share, then the
	 *  rest goes through the guard and onto health. OutArmour = how much the armour stopped (0..1). Returns the damage dealt.
	 */
	float TakeStrike(float Damage, ECIRLDamageType DamageType, EBallHitZone Zone, const struct FHitResult& Hit, const FVector& Direction,
		AController* EventInstigator, AActor* DamageCauser, float* OutArmour = nullptr);

	/** How much a heavy load slows this ball (1 = not at all) and how much more stamina everything costs (1 = normal) */
	float GetLoadSpeedScale() const;
	float GetLoadStaminaScale() const;

	UHealthComponent* GetHealth() const { return Health; }

	UBallMeleeComponent* GetMelee() const { return Melee; }

	UBallAnimatorComponent* GetAnimator() const { return Animator; }

	/** Coat of arms painted across the ball (a texture from /Game/CrownsAndCommoners/Characters/Flags) */
	UFUNCTION(BlueprintCallable, Category="Ball|Look")
	void SetFlag(UTexture2D* NewFlag);

	UTexture2D* GetFlag() const { return Flag; }

	/** The look on its face when nothing is going on */
	EBallEmotion GetStartingEmotion() const { return StartingEmotion; }

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
	virtual void OnConstruction(const FTransform& Transform) override;
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

	/** What this ball owns and wears */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UCIRLInventoryComponent> Inventory;

	/** Stand-in shapes for what is in the hands */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBallHeldItemsComponent> HeldItems;

	/** Stand-in shapes for what is worn */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBallWornGearComponent> WornGear;

	TOptional<FLinearColor> GloveTint;
	TOptional<FLinearColor> BootTint;

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
	 *  Body proportions (Master file > Characters & Art), matched to the user's concept art: a big ball sitting low
	 *  on short legs; ball 80% of the height, legs and boots 20%.
	 *  Defaults are the average English man of 1455: 1.71 m = 0.35 m legs and boots + 1.36 m ball.
	 */

	/** Ball radius in cm, side to side */
	UPROPERTY(VisibleAnywhere, Category="Ball")
	float BallRadius = 68.f;

	/** How much taller than wide the ball is (1 = perfect sphere, the chosen look) */
	UPROPERTY(VisibleAnywhere, Category="Ball")
	float BallHeightScale = 1.f;

	/** Distance from the ground to the bottom of the ball: the boots (30 cm) and a short stretch of leg */
	UPROPERTY(VisibleAnywhere, Category="Ball")
	float FeetGap = 35.f;

	/** Coat of arms painted across the ball. Empty = England's St George */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ball|Look")
	TObjectPtr<UTexture2D> Flag;

	/** M_BallArms, the material that paints a coat of arms across the ball */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> ArmsMaterial;

	/** This ball's own copy of it, with its coat of arms (made on first use) */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FlagMaterial;

	/** Makes sure the ball wears its coat of arms (creates the material the first time) */
	void ApplyFlag();

	/** Only used where no coat of arms material is available */
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

	/** Crouched and sneaking: half a walk */
	UPROPERTY(EditAnywhere, Category="Ball|Movement")
	float SneakSpeed = 110.f;

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

	/** A woman: drawn with eyelashes (later: her own height range and clothes) */
	UPROPERTY(EditAnywhere, Category="Ball|Look")
	bool bFemale = false;

	/** What this ball owns from the start (item ids); each is put on if a slot is free. Villagers, soldiers and
	 *  bandits get their gear here. The player's comes from the item settings instead */
	UPROPERTY(EditAnywhere, Category="Ball|Gear")
	TArray<FName> StartingGear;

	/** Test builds dress the balls that have no gear of their own in one of a few sets of armour, to fight against */
	virtual bool GetsTestGear() const { return true; }

	/** This much armour halves a blow (twice as much cuts it to a third, and so on) */
	UPROPERTY(EditAnywhere, Category="Ball|Combat")
	float ArmourHalfPoint = 60.f;

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

	/** Sneak key held */
	bool bWantsToSneak = false;

	UPROPERTY(Transient)
	TObjectPtr<AHorse> MountedHorse;

	/** Keeps the ball sitting in the saddle as the horse's back moves */
	void UpdateSeat();

	/** Actually running this frame (wants to, has stamina, moving on the ground) */
	bool bRunning = false;
};
