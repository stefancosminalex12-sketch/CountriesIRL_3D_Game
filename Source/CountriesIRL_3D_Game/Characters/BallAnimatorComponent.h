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
 *  Procedural animation for a ball: floating Rayman-style hands and feet, plus a body bob and lean.
 *  No skeleton needed.
 *
 *  Feet really step: each foot stays planted on the ground until the body has moved too far from it,
 *  then takes a step (an arc) to where it should be. So starting, stopping, strafing and turning on the
 *  spot all produce natural footwork, and stopping ends with a settling step instead of a slide.
 *  Hands swing with the opposite foot, and the body bobs with the steps.
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

	virtual void BeginPlay() override;

	/** How long one step takes, walking and running (seconds) */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	FVector2D StepDuration = FVector2D(0.22f, 0.16f);

	/** How high feet lift during a step, walking and running */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	FVector2D FootLift = FVector2D(8.f, 15.f);

	/** When standing still, a foot this far from its resting spot takes a small settling step */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	float SettleDistance = 4.f;

	/** Hand swing per cm of foot offset (hands swing with the opposite foot) */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	float HandSwingPerFootOffset = 0.8f;

	/** How much the body rises with each step (fraction of the step's lift) */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	float BodyBobPerLift = 0.4f;

	/** Forward lean at full run (degrees) */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	float MaxLean = 10.f;

	/** Size of hands and feet in cm */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	float HandSize = 20.f;

	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	FVector FootSize = FVector(26.f, 15.f, 11.f);

	/** Distance from the center line to each foot's resting spot */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	float FootHalfSpacing = 17.f;

	/** Gap kept between the feet's edges, so they never overlap */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	float MinFootGap = 4.f;

	/** How far up/down a foot may reach to find the ground (slopes, steps) */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	float MaxFootAdjust = 35.f;

private:

	/** Per-foot stepping state. Positions are world space, at the bottom center of the foot. */
	struct FFootState
	{
		FVector Planted = FVector::ZeroVector;
		FVector StepFrom = FVector::ZeroVector;
		FVector StepTo = FVector::ZeroVector;
		float StepAlpha = -1.f;
		float StepTime = 0.2f;
		float StepLift = 8.f;

		bool IsStepping() const { return StepAlpha >= 0.f; }
	};

	void UpdateFeet(float DeltaTime, float SpeedAlpha, bool bFalling);

	/** Where a foot should rest right now (world, on the ground) */
	FVector FootRestWorld(int32 Index) const;

	/** Current foot position (world, bottom center) including the step arc */
	FVector FootWorld(int32 Index) const;

	/** Keeps a target on the foot's own side of the body so the feet never cross or overlap */
	FVector KeepOnOwnSide(int32 Index, const FVector& WorldTarget) const;

	/** Ground height at a world XY near ReferenceZ (returns ReferenceZ if nothing in reach) */
	float GroundZAt(const FVector& WorldPoint, float ReferenceZ) const;

	/** Pulls a hand back toward the ball if it would go into a wall. Positions are in limb-root space. */
	FVector KeepHandOutOfWalls(int32 Index, const FVector& LocalStart, const FVector& LocalTarget, float DeltaTime);

	UPROPERTY() TObjectPtr<USceneComponent> LimbRoot;

	UPROPERTY() TObjectPtr<UStaticMeshComponent> LeftHand;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> RightHand;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> LeftFoot;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> RightFoot;
	UPROPERTY() TObjectPtr<USceneComponent> BodyPivot;

	FFootState Feet[2];
	bool bFeetPlanted = false;

	bool bFirstPersonHands = false;

	/** 0 = grounded, 1 = in the air (smoothed) */
	float AirBlend = 0.f;

	/** 0 = third-person hand pose, 1 = first-person hand pose (smoothed) */
	float FirstPersonBlend = 0.f;

	/** 0 = standing, 1 = moving (smoothed); used for idle breathing */
	float MoveBlend = 0.f;

	/** Local-space lean direction scaled by lean strength (smoothed) */
	FVector SmoothedLean = FVector::ZeroVector;
	float IdleTime = 0.f;

	/** 0 = alive pose, 1 = lying dead (smoothed) */
	float DeadBlend = 0.f;

	/** How far each hand may reach before a wall (1 = full reach) */
	float HandReach[2] = { 1.f, 1.f };
};
