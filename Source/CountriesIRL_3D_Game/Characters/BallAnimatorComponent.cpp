// CountriesIRL 3D Game

#include "Characters/BallAnimatorComponent.h"
#include "Characters/BallCharacter.h"
#include "Characters/BallParts.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

namespace
{
	/** Feet further than this from where they belong (teleport, respawn) snap instead of stepping */
	constexpr float FootSnapDistance = 150.f;

	float Side(int32 Index) { return Index == 0 ? -1.f : 1.f; }
}

UBallAnimatorComponent::UBallAnimatorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// Animate after movement so limbs match this frame's position
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

void UBallAnimatorComponent::BeginPlay()
{
	Super::BeginPlay();
	bFeetPlanted = false;
}

void UBallAnimatorComponent::ApplyColors(const FLinearColor& HandColor, const FLinearColor& FootColor)
{
	BallParts::SetColor(LeftHand, HandColor);
	BallParts::SetColor(RightHand, HandColor);
	BallParts::SetColor(LeftFoot, FootColor);
	BallParts::SetColor(RightFoot, FootColor);
}

FVector UBallAnimatorComponent::FootRestWorld(int32 Index) const
{
	const ABallCharacter* Ball = CastChecked<ABallCharacter>(GetOwner());
	const FVector Local(2.f, Side(Index) * FootHalfSpacing, -Ball->GetGroundOffset());
	FVector World = LimbRoot->GetComponentTransform().TransformPosition(Local);
	World.Z = GroundZAt(World, World.Z);
	return World;
}

FVector UBallAnimatorComponent::FootWorld(int32 Index) const
{
	const FFootState& Foot = Feet[Index];
	if (!Foot.IsStepping())
	{
		return Foot.Planted;
	}

	const float Alpha = FMath::Clamp(Foot.StepAlpha, 0.f, 1.f);
	FVector Position = FMath::Lerp(Foot.StepFrom, Foot.StepTo, FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.f));
	Position.Z += Foot.StepLift * FMath::Sin(Alpha * UE_PI);
	return Position;
}

FVector UBallAnimatorComponent::KeepOnOwnSide(int32 Index, const FVector& WorldTarget) const
{
	const FTransform& Root = LimbRoot->GetComponentTransform();
	FVector Local = Root.InverseTransformPosition(WorldTarget);

	// Each foot stays at least half the (foot width + gap) away from the center line
	const float MinOffset = (FootSize.Y + MinFootGap) * 0.5f;
	Local.Y = Side(Index) * FMath::Max(Side(Index) * Local.Y, MinOffset);

	FVector Result = Root.TransformPosition(Local);
	Result.Z = WorldTarget.Z;
	return Result;
}

void UBallAnimatorComponent::UpdateFeet(float DeltaTime, float SpeedAlpha, bool bFalling)
{
	if (bFalling)
	{
		// Replant wherever we land
		bFeetPlanted = false;
		return;
	}

	if (!bFeetPlanted)
	{
		for (int32 Index = 0; Index < 2; ++Index)
		{
			Feet[Index] = FFootState();
			Feet[Index].Planted = FootRestWorld(Index);
		}
		bFeetPlanted = true;
	}

	const FVector Velocity2D = GetOwner()->GetVelocity() * FVector(1.f, 1.f, 0.f);
	const float Speed = Velocity2D.Size();
	const bool bMoving = Speed > 10.f;
	const float StepTime = FMath::Lerp(StepDuration.X, StepDuration.Y, SpeedAlpha);

	// Where each foot wants to land: its resting spot, half a step ahead in the direction of travel
	FVector Landing[2];
	for (int32 Index = 0; Index < 2; ++Index)
	{
		Landing[Index] = KeepOnOwnSide(Index, FootRestWorld(Index) + Velocity2D * (StepTime * 0.5f));
	}

	// Advance steps in progress; keep steering them toward the moving landing spot
	for (int32 Index = 0; Index < 2; ++Index)
	{
		FFootState& Foot = Feet[Index];
		if (!Foot.IsStepping())
		{
			continue;
		}

		Foot.StepTo = Landing[Index];
		Foot.StepAlpha += DeltaTime / Foot.StepTime;
		if (Foot.StepAlpha >= 1.f)
		{
			Foot.Planted = Foot.StepTo;
			Foot.StepAlpha = -1.f;
		}
	}

	// Start new steps: the foot that is furthest behind goes first, one foot at a time
	const float Distance[2] = {
		FVector::Dist2D(Feet[0].Planted, Landing[0]),
		FVector::Dist2D(Feet[1].Planted, Landing[1])
	};
	const int32 Order[2] = { Distance[0] >= Distance[1] ? 0 : 1, Distance[0] >= Distance[1] ? 1 : 0 };
	const float Trigger = bMoving ? FMath::Max(SettleDistance, Speed * StepTime * 0.95f) : SettleDistance;

	for (const int32 Index : Order)
	{
		FFootState& Foot = Feet[Index];
		const FFootState& Other = Feet[1 - Index];
		if (Foot.IsStepping())
		{
			continue;
		}

		if (Distance[Index] > FootSnapDistance)
		{
			Foot.Planted = Landing[Index];
			continue;
		}

		// A planted foot that ended up on the wrong side of the body must move right away
		const FVector PlantedLocal = LimbRoot->GetComponentTransform().InverseTransformPosition(Foot.Planted);
		const bool bCrossing = Side(Index) * PlantedLocal.Y < (FootSize.Y + MinFootGap) * 0.5f;

		const bool bOtherBusy = Other.IsStepping() && Other.StepAlpha < 0.75f;
		if ((Distance[Index] > Trigger && !bOtherBusy) || bCrossing)
		{
			Foot.StepFrom = Foot.Planted;
			Foot.StepTo = Landing[Index];
			Foot.StepAlpha = 0.f;
			Foot.StepTime = bMoving ? StepTime : StepDuration.X;
			// Settling steps are small shuffles; moving steps lift properly
			Foot.StepLift = bMoving
				? FMath::Lerp(FootLift.X, FootLift.Y, SpeedAlpha)
				: FMath::Clamp(Distance[Index] * 0.5f, 2.f, FootLift.X);
		}
	}
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
	const FTransform& Root = LimbRoot->GetComponentTransform();

	// Movement state
	const FVector Velocity2D = Ball->GetVelocity() * FVector(1.f, 1.f, 0.f);
	const float Speed = Velocity2D.Size();
	// 0 at a standstill, 1 at the owner's full running speed
	const float SpeedAlpha = FMath::Clamp(Speed / FMath::Max(Ball->GetRunSpeed(), 1.f), 0.f, 1.f);
	const bool bFalling = Ball->GetCharacterMovement()->IsFalling();

	MoveBlend = FMath::FInterpTo(MoveBlend, Speed > 10.f && !bFalling ? 1.f : 0.f, DeltaTime, 6.f);
	AirBlend = FMath::FInterpTo(AirBlend, bFalling ? 1.f : 0.f, DeltaTime, 10.f);
	FirstPersonBlend = FMath::FInterpTo(FirstPersonBlend, bFirstPersonHands ? 1.f : 0.f, DeltaTime, 8.f);
	IdleTime += DeltaTime;

	// Dead balls stop stepping and topple over onto their back
	const bool bDead = Ball->IsDead();
	DeadBlend = FMath::FInterpTo(DeadBlend, bDead ? 1.f : 0.f, DeltaTime, 4.f);
	if (!bDead)
	{
		UpdateFeet(DeltaTime, SpeedAlpha, bFalling);
	}

	// Feet: planted/stepping positions, blended toward a tucked pose while in the air
	float FootOffsetX[2] = { 0.f, 0.f };
	float MaxLift = 0.f;
	for (int32 Index = 0; Index < 2; ++Index)
	{
		const FVector Tucked(-1.f, Side(Index) * (FootHalfSpacing + 3.f), GroundZ + FootSize.Z * 0.5f + 14.f);
		FVector Local = Tucked;
		if (bFeetPlanted)
		{
			Local = Root.InverseTransformPosition(FootWorld(Index)) + FVector(0.f, 0.f, FootSize.Z * 0.5f);
			Local = FMath::Lerp(Local, Tucked, AirBlend);

			const FFootState& Foot = Feet[Index];
			if (Foot.IsStepping())
			{
				MaxLift = FMath::Max(MaxLift, Foot.StepLift * FMath::Sin(FMath::Clamp(Foot.StepAlpha, 0.f, 1.f) * UE_PI));
			}
		}

		FootOffsetX[Index] = Local.X - 2.f;

		const FVector DeadFoot(Radius + 8.f, Side(Index) * 24.f, GroundZ + FootSize.Z * 0.5f);
		Local = FMath::Lerp(Local, DeadFoot, DeadBlend);
		(Index == 0 ? LeftFoot : RightFoot)->SetRelativeLocation(Local);
	}

	// Hands: swing with the opposite foot (left hand forward when the right foot is forward)
	const FVector ThirdPersonRest(8.f, Radius + 14.f, CenterZ - 4.f);
	const FVector FirstPersonRest(Radius * 0.55f + 95.f, 45.f, CenterZ - 32.f);
	const FVector HandRest = FMath::Lerp(ThirdPersonRest, FirstPersonRest, FirstPersonBlend);
	const float SwingScale = HandSwingPerFootOffset * FMath::Lerp(1.f, 0.3f, FirstPersonBlend) * (1.f - AirBlend);
	for (int32 Index = 0; Index < 2; ++Index)
	{
		FVector Hand(HandRest.X, Side(Index) * HandRest.Y, HandRest.Z);
		Hand.X += FMath::Clamp(FootOffsetX[1 - Index] * SwingScale, -28.f, 28.f);
		Hand.Z += FMath::Sin(IdleTime * 2.2f + Side(Index)) * 1.5f;
		Hand += FVector(0.f, Side(Index) * 8.f, 22.f) * AirBlend;

		Hand = KeepHandOutOfWalls(Index, FVector(0.f, 0.f, CenterZ), Hand, DeltaTime);

		const FVector DeadHand(-5.f, Side(Index) * (Radius + 10.f), GroundZ + HandSize * 0.5f);
		Hand = FMath::Lerp(Hand, DeadHand, DeadBlend);
		(Index == 0 ? LeftHand : RightHand)->SetRelativeLocation(Hand);
	}

	// Body: rises a little with each step, breathes when idle, leans into movement
	const float Bob = MaxLift * BodyBobPerLift + FMath::Sin(IdleTime * 2.f) * 0.8f * (1.f - MoveBlend);

	FVector MoveDirection = Ball->GetActorRotation().UnrotateVector(Velocity2D).GetSafeNormal2D();
	SmoothedLean = FMath::VInterpTo(SmoothedLean, MoveDirection * SpeedAlpha, DeltaTime, 6.f);
	const FRotator Lean(-MaxLean * SmoothedLean.X, 0.f, MaxLean * 0.6f * SmoothedLean.Y);

	// Lying on the ground, tipped back so the eyes face the sky (scale shrinks the body as it decays)
	const float BodyScale = BodyPivot->GetRelativeScale3D().Z;
	const FVector AliveLocation(0.f, 0.f, CenterZ + Bob);
	const FVector DeadLocation(0.f, 0.f, GroundZ + Radius * BodyScale);
	const FQuat Rotation = FQuat::Slerp(Lean.Quaternion(), FRotator(70.f, 0.f, 12.f).Quaternion(), DeadBlend);

	BodyPivot->SetRelativeLocationAndRotation(FMath::Lerp(AliveLocation, DeadLocation, DeadBlend), Rotation);
}

float UBallAnimatorComponent::GroundZAt(const FVector& WorldPoint, float ReferenceZ) const
{
	const FVector Base(WorldPoint.X, WorldPoint.Y, ReferenceZ);
	const FVector Reach(0.f, 0.f, MaxFootAdjust);

	FHitResult Hit;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(BallFootTrace), false, GetOwner());
	if (GetWorld()->LineTraceSingleByChannel(Hit, Base + Reach, Base - Reach, ECC_Visibility, Params) && !Hit.bStartPenetrating)
	{
		return Hit.ImpactPoint.Z;
	}
	return ReferenceZ;
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
