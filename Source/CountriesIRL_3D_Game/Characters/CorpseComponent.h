// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CorpseComponent.generated.h"

class UStaticMeshComponent;
class UStaticMesh;

/** One step of decay, reached a number of in-game hours after death. */
USTRUCT(BlueprintType)
struct FCorpseStage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Corpse")
	FName Name;

	/** In-game hours after death when this stage is fully reached */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Corpse", meta=(ClampMin=0))
	float HoursAfterDeath = 0.f;

	/** Color the body shifts toward */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Corpse")
	FLinearColor Tint = FLinearColor::White;

	/** How strongly the tint replaces the living colors (0 = alive colors, 1 = pure tint) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Corpse", meta=(ClampMin=0, ClampMax=1))
	float TintStrength = 0.f;

	/** Body size (bloating, shrinking to bones) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Corpse", meta=(ClampMin=0.1))
	float BodyScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Corpse")
	bool bFlies = false;

	/** Ravens/crows come to pick at it (needs bird art; flag only for now) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Corpse")
	bool bScavengers = false;

	/** Only bones remain */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Corpse")
	bool bSkeleton = false;
};

/**
 *  Decay of a dead body over in-game time. Only the moment of death is stored; the current look is
 *  always worked out from the world clock, so waiting, sleeping, travel or coming back days later all
 *  show the right state (and saving it later is just one timestamp).
 *  The stage table is data; tune it per creature type.
 */
UCLASS(ClassGroup=(Ball), meta=(BlueprintSpawnableComponent))
class UCorpseComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UCorpseComponent();

	/** Starts decaying from the current in-game time */
	void StartDecay();

	bool IsDecaying() const { return bDecaying; }

	/** In-game hours since death */
	float GetHoursDead() const;

	/** Current decay look, blended between stages */
	FLinearColor GetTint() const { return CurrentTint; }
	float GetTintStrength() const { return CurrentTintStrength; }
	float GetBodyScale() const { return CurrentBodyScale; }
	bool IsSkeleton() const { return bSkeleton; }
	bool HasScavengers() const { return bScavengers; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	/** Decay stages in time order */
	UPROPERTY(EditAnywhere, Category="Corpse")
	TArray<FCorpseStage> Stages;

	/** In-game hours after death when the remains disappear (later: burial parties, looting) */
	UPROPERTY(EditAnywhere, Category="Corpse", meta=(ClampMin=1))
	float RemoveAfterHours = 24.f * 7.f;

	UPROPERTY(EditAnywhere, Category="Corpse|Flies", meta=(ClampMin=0, ClampMax=32))
	int32 FlyCount = 9;

	/** Fly size in cm (stylized, bigger than real) */
	UPROPERTY(EditAnywhere, Category="Corpse|Flies")
	float FlySize = 2.2f;

private:

	void UpdateStage();
	void SetFliesActive(bool bActive);
	void AnimateFlies(float DeltaTime);

	UPROPERTY(Transient) TArray<TObjectPtr<UStaticMeshComponent>> Flies;
	UPROPERTY() TObjectPtr<UStaticMesh> FlyMesh;

	/** Per-fly orbit parameters (radius, speed, height, phase) */
	TArray<FVector4f> FlyOrbits;
	float FlyTime = 0.f;

	FDateTime DeathTime;
	bool bDecaying = false;

	FLinearColor CurrentTint = FLinearColor::White;
	float CurrentTintStrength = 0.f;
	float CurrentBodyScale = 1.f;
	bool bSkeleton = false;
	bool bScavengers = false;
	bool bFliesActive = false;

	float TimeSinceStageUpdate = 0.f;
};
