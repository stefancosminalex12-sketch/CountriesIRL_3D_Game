// CountriesIRL 3D Game

#include "Characters/BallAnimatorComponent.h"
#include "Characters/BallCharacter.h"
#include "Characters/BallParts.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

UBallAnimatorComponent::UBallAnimatorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// Animate after movement so limbs match this frame's velocity
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UBallAnimatorComponent::CreateLimbMeshes(AActor* Owner, USceneComponent* LimbParent, USceneComponent* InBodyPivot, UStaticMesh* SphereMesh)
{
	BodyPivot = InBodyPivot;
	LimbRoot = LimbParent;

	LeftHand = BallParts::Create(Owner, TEXT("LeftHand"), LimbParent, SphereMesh);
	RightHand = BallParts::Create(Owner, TEXT("RightHand"), LimbParent, SphereMesh);
	LeftFoot = BallParts::Create(Owner, TEXT("LeftFoot"), LimbParent, SphereMesh);
	RightFoot = BallParts::Create(Owner, TEXT("RightFoot"), LimbParent, SphereMesh);

	for (UStaticMeshComponent* Hand : { LeftHand.Get(), RightHand.Get() })
	{
		Hand->SetRelativeScale3D(FVector(HandSize / 100.f));
	}
	for (UStaticMeshComponent* Foot : { LeftFoot.Get(), RightFoot.Get() })
	{
		Foot->SetRelativeScale3D(FootSize / 100.f);
	}
}

void UBallAnimatorComponent::ApplyColors(const FLinearColor& HandColor, const FLinearColor& FootColor)
{
	BallParts::SetColor(LeftHand, HandColor);
	BallParts::SetColor(RightHand, HandColor);
	BallParts::SetColor(LeftFoot, FootColor);
	BallParts::SetColor(RightFoot, FootColor);
}

void UBallAnimatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const ABallCharacter* Ball = Cast<ABallCharacter>(GetOwner());
	if (!Ball || !LeftHand || !BodyPivot || !LimbRoot)
	{
		return;
	}

	const float Radius = Ball->GetBallRadius();
	const float CenterZ = Ball->GetBallCenterZ();
	const float GroundZ = -Ball->GetGroundOffset();

	// Movement state
	const FVector Velocity2D = Ball->GetVelocity() * FVector(1.f, 1.f, 0.f);
	const float Speed = Velocity2D.Size();
	// 0 at a standstill, 1 at the owner's full running speed
	const float SpeedAlpha = FMath::Clamp(Speed / FMath::Max(Ball->GetRunSpeed(), 1.f), 0.f, 1.f);
	const bool bFalling = Ball->GetCharacterMovement()->IsFalling();
	const bool bMoving = Speed > 10.f && !bFalling;

	MoveBlend = FMath::FInterpTo(MoveBlend, bMoving ? 1.f : 0.f, DeltaTime, 8.f);
	AirBlend = FMath::FInterpTo(AirBlend, bFalling ? 1.f : 0.f, DeltaTime, 10.f);
	FirstPersonBlend = FMath::FInterpTo(FirstPersonBlend, bFirstPersonHands ? 1.f : 0.f, DeltaTime, 8.f);
	IdleTime += DeltaTime;

	if (bMoving)
	{
		const float Stride = FMath::Lerp(StrideLength.X, StrideLength.Y, SpeedAlpha);
		Phase = FMath::Fmod(Phase + UE_TWO_PI * Speed / Stride * DeltaTime, UE_TWO_PI);
	}

	FVector MoveDirection = Ball->GetActorRotation().UnrotateVector(Velocity2D).GetSafeNormal2D();
	if (MoveDirection.IsNearlyZero())
	{
		MoveDirection = FVector::ForwardVector;
	}

	// Feet: swing forward while lifted, push back while planted. Left and right are half a cycle apart.
	// Sideways swing is limited so that when strafing the feet side-step instead of crossing each other.
	const float FootSwingNow = FMath::Lerp(FootSwing.X, FootSwing.Y, SpeedAlpha) * MoveBlend;
	const float SideSwingNow = FMath::Min(FootSwingNow, FMath::Max(FootHalfSpacing - MinFootGap * 0.5f, 0.f));
	const FVector FootSwingVector(MoveDirection.X * FootSwingNow, MoveDirection.Y * SideSwingNow, 0.f);
	const float FootLiftNow = FMath::Lerp(FootLift.X, FootLift.Y, SpeedAlpha) * MoveBlend;
	for (int32 Index = 0; Index < 2; ++Index)
	{
		const float Side = Index == 0 ? -1.f : 1.f;
		const float Theta = Phase + (Side > 0.f ? UE_PI : 0.f);
		FVector Foot(2.f, Side * FootHalfSpacing, GroundZ + FootSize.Z * 0.5f);
		Foot += FootSwingVector * FMath::Sin(Theta);

		// Plant on the real ground (slopes, steps) rather than the flat bottom of the capsule
		const float TargetGround = bFalling ? 0.f : GroundHeightUnder(Foot, GroundZ);
		FootGroundOffset[Index] = FMath::FInterpTo(FootGroundOffset[Index], TargetGround, DeltaTime, 18.f);
		Foot.Z += FootGroundOffset[Index];

		Foot.Z += FootLiftNow * FMath::Max(0.f, FMath::Cos(Theta));
		Foot += FVector(-3.f, Side * 3.f, 14.f) * AirBlend;

		(Index == 0 ? LeftFoot : RightFoot)->SetRelativeLocation(Foot);
	}

	// Hands: swing opposite to the foot on the same side
	const FVector ThirdPersonRest(8.f, Radius + 14.f, CenterZ - 4.f);
	const FVector FirstPersonRest(Radius * 0.55f + 95.f, 45.f, CenterZ - 32.f);
	const FVector HandRest = FMath::Lerp(ThirdPersonRest, FirstPersonRest, FirstPersonBlend);
	const float HandSwingNow = FMath::Lerp(HandSwing.X, HandSwing.Y, SpeedAlpha) * MoveBlend * FMath::Lerp(1.f, 0.3f, FirstPersonBlend);
	for (int32 Index = 0; Index < 2; ++Index)
	{
		const float Side = Index == 0 ? -1.f : 1.f;
		const float Theta = Phase + (Side > 0.f ? 0.f : UE_PI);
		FVector Hand(HandRest.X, Side * HandRest.Y, HandRest.Z);
		Hand.X += HandSwingNow * FMath::Sin(Theta);
		Hand.Z += 2.f * FMath::Abs(FMath::Cos(Theta)) * MoveBlend;
		Hand.Z += FMath::Sin(IdleTime * 2.2f + Side) * 1.5f;
		Hand += FVector(0.f, Side * 8.f, 22.f) * AirBlend;

		Hand = KeepHandOutOfWalls(Index, FVector(0.f, 0.f, CenterZ), Hand, DeltaTime);
		(Index == 0 ? LeftHand : RightHand)->SetRelativeLocation(Hand);
	}

	// Body: bounce on each step, breathe when idle, lean into movement
	const float Bob = FMath::Lerp(BodyBob.X, BodyBob.Y, SpeedAlpha) * MoveBlend * FMath::Abs(FMath::Cos(Phase))
		+ FMath::Sin(IdleTime * 2.f) * 0.8f * (1.f - MoveBlend);

	SmoothedLean = FMath::VInterpTo(SmoothedLean, MoveDirection * SpeedAlpha * MoveBlend, DeltaTime, 6.f);
	const FRotator Lean(-MaxLean * SmoothedLean.X, 0.f, MaxLean * 0.6f * SmoothedLean.Y);

	BodyPivot->SetRelativeLocationAndRotation(FVector(0.f, 0.f, CenterZ + Bob), Lean);
}

float UBallAnimatorComponent::GroundHeightUnder(const FVector& LocalFoot, float GroundZ) const
{
	// The limb root only yaws with the actor, so a local Z offset equals a world Z offset
	const FVector Base = LimbRoot->GetComponentTransform().TransformPosition(FVector(LocalFoot.X, LocalFoot.Y, GroundZ));
	const FVector Reach(0.f, 0.f, MaxFootAdjust);

	FHitResult Hit;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(BallFootTrace), false, GetOwner());
	if (GetWorld()->LineTraceSingleByChannel(Hit, Base + Reach, Base - Reach, ECC_Visibility, Params) && !Hit.bStartPenetrating)
	{
		return Hit.ImpactPoint.Z - Base.Z;
	}
	return 0.f;
}

FVector UBallAnimatorComponent::KeepHandOutOfWalls(int32 Index, const FVector& LocalStart, const FVector& LocalTarget, float DeltaTime)
{
	// Sweep a hand-sized sphere from the ball's center out to where the hand wants to be
	const FTransform& Root = LimbRoot->GetComponentTransform();
	const FVector Start = Root.TransformPosition(LocalStart);
	const FVector End = Root.TransformPosition(LocalTarget);

	float Reach = 1.f;
	FHitResult Hit;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(BallHandSweep), false, GetOwner());
	if (GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(HandSize * 0.5f), Params))
	{
		Reach = Hit.bStartPenetrating ? 0.f : Hit.Time;
	}

	// Pull in instantly so hands never poke through; ease back out once the way is clear
	HandReach[Index] = Reach < HandReach[Index] ? Reach : FMath::FInterpTo(HandReach[Index], Reach, DeltaTime, 10.f);
	return FMath::Lerp(LocalStart, LocalTarget, HandReach[Index]);
}
