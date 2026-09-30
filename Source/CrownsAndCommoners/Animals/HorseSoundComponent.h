// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HorseSoundComponent.generated.h"

class AHorse;
class UAudioComponent;
class USoundAttenuation;
class USoundBase;

/**
 *  A horse's sounds, from its UMountDefinition: hoofbeats that loop while it moves (one loop per gait, cross-faded
 *  as it changes gait, louder and quicker the faster it goes), a snort now and then (often when it is out of
 *  breath), and neighs when it rears. Heard in the world: louder close by, gone past ~35 m.
 */
UCLASS(ClassGroup=(Horse))
class UHorseSoundComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UHorseSoundComponent();

	/** A random neigh from the horse's set; returns how long it lasts (0 if it has none) */
	float PlayNeigh();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	virtual void BeginPlay() override;

	/** Seconds between snorts when rested, and when out of breath, picked in these ranges */
	UPROPERTY(EditAnywhere, Category="Horse Sound")
	FVector2D RestedSnortGap = FVector2D(15.f, 35.f);

	UPROPERTY(EditAnywhere, Category="Horse Sound")
	FVector2D TiredSnortGap = FVector2D(2.5f, 5.f);

	/** Below this share of its stamina the horse counts as out of breath */
	UPROPERTY(EditAnywhere, Category="Horse Sound")
	float TiredBelow = 0.25f;

	/** How far away it can be heard (cm) */
	UPROPERTY(EditAnywhere, Category="Horse Sound")
	float HearingDistance = 3500.f;

private:

	void PlayOneShot(const TArray<TObjectPtr<USoundBase>>& Sounds);

	/** One looping hoofbeat per gait (walk, trot, canter, gallop) and how loud each is now */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UAudioComponent>> GaitLoops;
	float GaitVolume[4] = { 0.f, 0.f, 0.f, 0.f };

	UPROPERTY(Transient)
	TObjectPtr<USoundAttenuation> Attenuation;

	float NextSnort = 0.f;
};
