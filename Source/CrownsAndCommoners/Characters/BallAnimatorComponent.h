// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BallAnimatorComponent.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class USceneComponent;
class ABallCharacter;

/**
 *  A floating hand: a palm, four fingers and a thumb. Every finger has two cylinder segments with a round
 *  joint and tip, so fingers roll into a real fist and the thumb wraps over the front of it.
 */
USTRUCT()
struct FBallHandParts
{
	GENERATED_BODY()

	/** Moved and rotated by the animator; everything else hangs from it */
	UPROPERTY() TObjectPtr<USceneComponent> Root;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Palm;

	/** Per finger (index, middle, ring, little, thumb): base segment, joint, outer segment, tip */
	UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> BaseSegments;
	UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Joints;
	UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> OuterSegments;
	UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Tips;

	/** Fist amount currently applied, to skip re-posing when nothing changed */
	float AppliedFist = -1.f;
};

/** A boot (sole, foot, ankle shaft) and the short leg that joins it to the ball. */
USTRUCT()
struct FBallFootParts
{
	GENERATED_BODY()

	UPROPERTY() TObjectPtr<USceneComponent> Root;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Sole;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Upper;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Shaft;

	/** Leg (wool hose) from a hip point inside the ball down to the boot: thigh, knee and shin with fixed lengths.
	 *  Placed every frame by two-bone IK, so the knee bends forward when the foot comes up and nothing stretches */
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Thigh;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Knee;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Shin;
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

	/**
	 *  Creates hand and foot meshes in a resting pose (so the editor preview looks right before play).
	 *  Must be called from the owning actor's constructor. Heights are relative to the capsule center.
	 */
	void CreateLimbMeshes(AActor* Owner, USceneComponent* LimbParent, USceneComponent* InBodyPivot, UStaticMesh* SphereMesh,
		float BallRadius, float BallCenterZ, float GroundZ);

	void ApplyColors(const FLinearColor& HandColor, const FLinearColor& FootColor);

	/** In first-person the hands are held in front of the camera so the player can see them */
	void SetFirstPersonHands(bool bEnable) { bFirstPersonHands = bEnable; }

	/** Only bones remain: hands and boots lie beside the skeleton instead of the body */
	void SetSkeletonPose(bool bEnable) { bSkeletonPose = bEnable; }

	/** In the saddle: boots hang at the mount's sides (MountHalfWidth = half its body width), hands hold the reins */
	void SetRiding(bool bEnable, float MountHalfWidth = 0.f);

	/** Puts both feet straight onto their resting spots under the body, with no step (e.g. when the body is turned
	 *  like a statue on a turntable, as the Equipment screen's doll is) */
	void SnapFeetToRest();

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

	/** How far below the ball's centre the hands rest, as a share of the ball's radius. They hang on the lower
	 *  part of the ball, like arms at the sides, so they sit between the ball and the floating boots */
	UPROPERTY(EditAnywhere, Category="Ball|Hands", meta=(ClampMin=0, ClampMax=0.9))
	float HandRestDrop = 0.52f;

	/** Gap between the ball's surface and a resting hand (cm) */
	UPROPERTY(EditAnywhere, Category="Ball|Hands")
	float HandRestGap = 20.f;

	/** Hands are drawn this much bigger than their modelled size (big cartoon hands, like the concept art) */
	UPROPERTY(EditAnywhere, Category="Ball|Hands")
	float HandScale = 1.6f;

	/** Leg thickness (cm); the boot shaft around the ankle is a little thicker */
	UPROPERTY(EditAnywhere, Category="Ball|Legs")
	float LegThickness = 14.f;

	/** Thigh (hip to knee) and shin (knee to ankle) lengths in cm. The hip sits inside the ball, so only part
	 *  of the thigh shows. Standing, the leg is almost straight; as a foot lifts the knee bends forward */
	UPROPERTY(EditAnywhere, Category="Ball|Legs")
	float ThighLength = 21.f;

	UPROPERTY(EditAnywhere, Category="Ball|Legs")
	float ShinLength = 15.5f;

	/** How far below the ball's centre the hips are, as a share of the ball's radius */
	UPROPERTY(EditAnywhere, Category="Ball|Legs")
	float HipDrop = 0.6f;

	/** Colour of the legs (wool hose) */
	UPROPERTY(EditAnywhere, Category="Ball|Legs")
	FLinearColor LegColor = FLinearColor(0.16f, 0.07f, 0.045f);

	/** Where a resting hand floats (right hand; the left mirrors it), in limb-root space */
	FVector HandRestLocation(float Radius, float CenterZ) const;

	/** Size of hands and feet in cm (HandSize is the space a hand takes, for wall checks) */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	float HandSize = 20.f;

	/** Palm size in cm (length along the fingers, width across the knuckles, thickness) */
	UPROPERTY(EditAnywhere, Category="Ball|Hands")
	FVector PalmSize = FVector(14.f, 17.f, 9.f);

	/** Finger thickness in cm */
	UPROPERTY(EditAnywhere, Category="Ball|Hands")
	float FingerThickness = 4.3f;

	/** Bend at each finger joint for a relaxed hand, in degrees */
	UPROPERTY(EditAnywhere, Category="Ball|Hands")
	float RelaxedJointBend = 15.f;

	/** Bend at the knuckle and the middle joint in a clenched fist, in degrees */
	UPROPERTY(EditAnywhere, Category="Ball|Hands")
	FVector2D FistJointBend = FVector2D(85.f, 100.f);

	/** How far the whole body twists into a punch (degrees), leans forward and lunges (cm) */
	UPROPERTY(EditAnywhere, Category="Ball|Punch")
	float PunchTwist = 14.f;

	UPROPERTY(EditAnywhere, Category="Ball|Punch")
	float PunchLean = 7.f;

	UPROPERTY(EditAnywhere, Category="Ball|Punch")
	float PunchLunge = 6.f;

	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	FVector FootSize = FVector(34.f, 19.f, 13.f);

	/** Height of the boot's ankle shaft above the middle of the foot (cm); the whole boot is ~30 cm tall */
	static constexpr float BootShaftTop = 24.f;

	/** Distance from the center line to each foot's resting spot */
	UPROPERTY(EditAnywhere, Category="Ball|Animation")
	float FootHalfSpacing = 22.f;

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

	/** Places thigh, knee and shin between each hip and ankle (two-bone IK); hides them on a body lying dead */
	void UpdateLegs();

	/** Keeps a target on the foot's own side of the body so the feet never cross or overlap */
	FVector KeepOnOwnSide(int32 Index, const FVector& WorldTarget) const;

	/** Ground height at a world XY near ReferenceZ (returns ReferenceZ if nothing in reach) */
	float GroundZAt(const FVector& WorldPoint, float ReferenceZ) const;

	/** Places fingers and thumb between a relaxed hand (0) and a fist (1) */
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

	bool bRiding = false;
	float RidingHalfWidth = 0.f;
	float RideBlend = 0.f;
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

	/** Parts sharing one color share one material, so the renderer can draw them together */
	void ShareColor(const TArray<UStaticMeshComponent*>& Parts, const FLinearColor& Color);

	/** Smoothed hand orientation and fist amount (left, right) */
	FQuat HandRotation[2] = { FQuat::Identity, FQuat::Identity };
	float FistAmount[2] = { 0.f, 0.f };

	/** 0 = hands down, 1 = guard up (smoothed) */
	float GuardBlend = 0.f;

	/** How far each hand may reach before a wall (1 = full reach) */
	float HandReach[2] = { 1.f, 1.f };
};
