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
	FVector2D StrideLength = FVector2D(90.f, 130.f);

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

	/** Size of hands and feet in cm */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	float HandSize = 20.f;

	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	FVector FootSize = FVector(26.f, 15.f, 11.f);

	/** Distance from the center line to each foot */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	float FootHalfSpacing = 16.f;

	/** Feet never get closer than this to each other when side-stepping */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	float MinFootGap = 8.f;

	/** How far up/down a foot may reach to find the ground (slopes, steps) */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	float MaxFootAdjust = 35.f;

private:

	/** Height of the ground under a foot relative to the capsule bottom (0 if nothing in reach) */
	float GroundHeightUnder(const FVector& LocalFoot, float GroundZ) const;

	/** Pulls a hand back toward the ball if it would go into a wall. Positions are in limb-root space. */
	FVector KeepHandOutOfWalls(int32 Index, const FVector& LocalStart, const FVector& LocalTarget, float DeltaTime);

	UPROPERTY() TObjectPtr<USceneComponent> LimbRoot;

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

	/** Smoothed ground height under each foot (left, right) */
	float FootGroundOffset[2] = { 0.f, 0.f };

	/** How far each hand may reach before a wall (1 = full reach) */
	float HandReach[2] = { 1.f, 1.f };
};
