// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BallAnimatorComponent.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class USceneComponent;
class ABallCharacter;

/** A floating hand: a palm, four cylinder fingers and a thumb (each with a round tip), posed by curling the fingers. */
USTRUCT()
struct FBallHandParts
{
	GENERATED_BODY()

	/** Moved and rotated by the animator; everything else hangs from it */
	UPROPERTY() TObjectPtr<USceneComponent> Root;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Palm;

	/** Index, middle, ring, little finger, then the thumb */
	UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Fingers;
	UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Tips;

	/** Finger curl currently applied, to skip re-posing when nothing changed */
	float AppliedCurl = -1.f;
};

/** A floating boot: sole, foot and a short ankle shaft. */
USTRUCT()
struct FBallFootParts
{
	GENERATED_BODY()

	UPROPERTY() TObjectPtr<USceneComponent> Root;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Sole;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Upper;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Shaft;
};

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

	/** Only bones remain: hands and boots lie beside the skeleton instead of the body */
	void SetSkeletonPose(bool bEnable) { bSkeletonPose = bEnable; }

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

	/** Size of hands and feet in cm (HandSize is the space a hand takes, for wall checks) */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	float HandSize = 20.f;

	/** Palm size in cm (length along the fingers, width across the knuckles, thickness) */
	UPROPERTY(EditAnywhere, Category="Ball|Hands")
	FVector PalmSize = FVector(14.f, 17.f, 9.f);

	/** Finger thickness in cm */
	UPROPERTY(EditAnywhere, Category="Ball|Hands")
	float FingerThickness = 4.3f;

	/** Finger bend in degrees: relaxed hands and clenched fists */
	UPROPERTY(EditAnywhere, Category="Ball|Hands")
	float RelaxedCurl = 30.f;

	UPROPERTY(EditAnywhere, Category="Ball|Hands")
	float FistCurl = 155.f;

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

	/** Places fingers and thumb for a curl between relaxed (0) and fist (1) */
	void PoseHand(FBallHandParts& Hand, float FistAmount, float ThumbSide) const;

	/** Pulls a hand back toward the ball if it would go into a wall. Positions are in limb-root space. */
	FVector KeepHandOutOfWalls(int32 Index, const FVector& LocalStart, const FVector& LocalTarget, float DeltaTime);

	UPROPERTY() TObjectPtr<USceneComponent> LimbRoot;

	/** Left, right */
	UPROPERTY() FBallHandParts Hands[2];
	UPROPERTY() FBallFootParts Boots[2];
	UPROPERTY() TObjectPtr<USceneComponent> BodyPivot;

	FFootState Feet[2];
	bool bFeetPlanted = false;

	bool bFirstPersonHands = false;
	bool bSkeletonPose = false;

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

	/** Smoothed hand orientation and fist amount (left, right) */
	FQuat HandRotation[2] = { FQuat::Identity, FQuat::Identity };
	float FistAmount[2] = { 0.f, 0.f };

	/** 0 = hands down, 1 = guard up (smoothed) */
	float GuardBlend = 0.f;

	/** How far each hand may reach before a wall (1 = full reach) */
	float HandReach[2] = { 1.f, 1.f };
};
