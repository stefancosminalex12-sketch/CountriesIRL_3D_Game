// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BallFootstepsComponent.generated.h"

class UAudioComponent;
class USoundBase;

/**
 *  Footsteps for a ball, by the ground under it (Project Settings > Crowns & Commoners Audio > Footsteps: one set of
 *  recordings per kind of ground, found through the ground's physical material; the first set is for any other
 *  ground). Each set has variations of the same ground: one is picked at random whenever the ball starts walking or
 *  the ground changes, and long recordings start at a random point. Louder and quicker when running, soft when
 *  sneaking, silent in the air or in the saddle. A thump on landing from a jump or a fall.
 *  Heard as your own steps (not placed in the world); NPCs will get placed ones.
 */
UCLASS(ClassGroup=(Ball))
class UBallFootstepsComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UBallFootstepsComponent();

	/** The ball hit the ground falling this fast (cm/s, positive down): a landing sound if it was a real drop */
	void OnLanded(float FallSpeed);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	virtual void BeginPlay() override;

	/** Falling slower than this (cm/s) makes no landing sound; this fast or more is a full thump */
	UPROPERTY(EditAnywhere, Category="Footsteps")
	FVector2D LandingSpeed = FVector2D(380.f, 1000.f);

private:

	/** Name of the ground under the ball (its physical surface), or NAME_None */
	FName TraceGround() const;

	/** Starts a random variation of the given set, cross-fading from whatever played before */
	void StartSteps(int32 Set);

	/** Two players, so a new ground or variation can fade in while the old one fades out */
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> Players[2];
	int32 ActivePlayer = 0;

	int32 CurrentSet = INDEX_NONE;
	int32 LastVariation = INDEX_NONE;
	bool bStepping = false;
	float Volume = 0.f;
	float NextGroundCheck = 0.f;
	FName Ground;
};
