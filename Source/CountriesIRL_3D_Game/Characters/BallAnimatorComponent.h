// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BallAnimatorComponent.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class USceneComponent;
class ABallCharacter;

/**
 *  Procedural animation for a ball: floating Rayman-style hands and feet that step with movement,
 *  plus a body bob and lean. No skeleton needed; everything is driven by the owner's velocity.
 */
UCLASS(ClassGroup=(Ball), meta=(BlueprintSpawnableComponent))
class UBallAnimatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UBallAnimatorComponent();

	/** Creates hand and foot meshes. Must be called from the owning actor's constructor. */
	void CreateLimbMeshes(AActor* Owner, USceneComponent* LimbParent, USceneComponent* InBodyPivot, UStaticMesh* SphereMesh);

	void ApplyColors(const FLinearColor& HandColor, const FLinearColor& FootColor);

	/** In first-person the hands are held in front of the camera so the player can see them */
	void SetFirstPersonHands(bool bEnable) { bFirstPersonHands = bEnable; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	/** Distance covered by one full walk cycle (two steps), at walking and running speed */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	FVector2D StrideLength = FVector2D(80.f, 140.f);

	/** How far feet swing forward/back, walking and running */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	FVector2D FootSwing = FVector2D(14.f, 26.f);

	/** How high feet lift, walking and running */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	FVector2D FootLift = FVector2D(8.f, 16.f);

	/** How far hands swing, walking and running */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	FVector2D HandSwing = FVector2D(10.f, 24.f);

	/** How much the body bounces each step, walking and running */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	FVector2D BodyBob = FVector2D(3.f, 7.f);

	/** Forward lean at full run (degrees) */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	float MaxLean = 10.f;

	/** Speed treated as a full run for blending */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	float RunSpeedReference = 600.f;

	/** Size of hands and feet in cm */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	float HandSize = 20.f;

	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	FVector FootSize = FVector(26.f, 15.f, 11.f);

private:

	UPROPERTY() TObjectPtr<UStaticMeshComponent> LeftHand;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> RightHand;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> LeftFoot;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> RightFoot;
	UPROPERTY() TObjectPtr<USceneComponent> BodyPivot;

	bool bFirstPersonHands = false;

	/** Walk cycle phase in radians */
	float Phase = 0.f;

	/** 0 = standing, 1 = moving (smoothed) */
	float MoveBlend = 0.f;

	/** 0 = grounded, 1 = in the air (smoothed) */
	float AirBlend = 0.f;

	/** 0 = third-person hand pose, 1 = first-person hand pose (smoothed) */
	float FirstPersonBlend = 0.f;

	/** Local-space lean direction scaled by lean strength (smoothed) */
	FVector SmoothedLean = FVector::ZeroVector;
	float IdleTime = 0.f;
};
