// CountriesIRL 3D Game

#include "Characters/BallAnimatorComponent.h"
#include "Characters/BallCharacter.h"
#include "Characters/BallParts.h"
#include "Characters/BallMeleeComponent.h"
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

	UStaticMesh* CubeMesh = BallParts::LoadCube();
	UStaticMesh* CylinderMesh = BallParts::LoadCylinder();

	for (int32 Index = 0; Index < 2; ++Index)
	{
		const TCHAR* Prefix = Index == 0 ? TEXT("Left") : TEXT("Right");
		auto PartName = [Prefix](const FString& Part) { return FName(*FString::Printf(TEXT("%s%s"), Prefix, *Part)); };

		// Hand: palm plus four fingers and a thumb, each a cylinder with a round tip
		FBallHandParts& Hand = Hands[Index];
		Hand.Root = Owner->CreateDefaultSubobject<USceneComponent>(PartName(TEXT("Hand")));
		Hand.Root->SetupAttachment(LimbParent);
		Hand.Palm = BallParts::Create(Owner, PartName(TEXT("Palm")), Hand.Root, SphereMesh);
		Hand.Palm->SetRelativeScale3D(PalmSize / 100.f);
		for (int32 Finger = 0; Finger < 5; ++Finger)
		{
			Hand.Fingers.Add(BallParts::Create(Owner, PartName(FString::Printf(TEXT("Finger%d"), Finger)), Hand.Root, CylinderMesh));
			Hand.Tips.Add(BallParts::Create(Owner, PartName(FString::Printf(TEXT("FingerTip%d"), Finger)), Hand.Root, SphereMesh));
		}

		// Boot: a flat sole, the foot and a short ankle shaft (origin at the middle of the foot)
		FBallFootParts& Boot = Boots[Index];
		Boot.Root = Owner->CreateDefaultSubobject<USceneComponent>(PartName(TEXT("Boot")));
		Boot.Root->SetupAttachment(LimbParent);
		Boot.Sole = BallParts::Create(Owner, PartName(TEXT("BootSole")), Boot.Root, CubeMesh);
		Boot.Sole->SetRelativeTransform(FTransform(FRotator::ZeroRotator, FVector(0.f, 0.f, -FootSize.Z * 0.5f + 1.5f), FVector(FootSize.X + 1.f, FootSize.Y - 1.f, 3.f) / 100.f));
		Boot.Upper = BallParts::Create(Owner, PartName(TEXT("BootUpper")), Boot.Root, SphereMesh);
		Boot.Upper->SetRelativeTransform(FTransform(FRotator::ZeroRotator, FVector(1.5f, 0.f, 1.f), FVector(FootSize.X, FootSize.Y, FootSize.Z - 1.f) / 100.f));
		Boot.Shaft = BallParts::Create(Owner, PartName(TEXT("BootShaft")), Boot.Root, CylinderMesh);
		Boot.Shaft->SetRelativeTransform(FTransform(FRotator::ZeroRotator, FVector(-5.f, 0.f, 6.f), FVector(12.f, 12.f, 12.f) / 100.f));
	}
}

void UBallAnimatorComponent::BeginPlay()
{
	Super::BeginPlay();
	bFeetPlanted = false;
}

void UBallAnimatorComponent::ApplyColors(const FLinearColor& HandColor, const FLinearColor& FootColor)
{
	const FLinearColor SoleColor = FootColor * 0.45f;
	for (int32 Index = 0; Index < 2; ++Index)
	{
		BallParts::SetColor(Hands[Index].Palm, HandColor);
		for (UStaticMeshComponent* Part : Hands[Index].Fingers)
		{
			BallParts::SetColor(Part, HandColor);
		}
		for (UStaticMeshComponent* Part : Hands[Index].Tips)
		{
			BallParts::SetColor(Part, HandColor);
		}

		BallParts::SetColor(Boots[Index].Sole, SoleColor.CopyWithNewOpacity(1.f));
		BallParts::SetColor(Boots[Index].Upper, FootColor);
		BallParts::SetColor(Boots[Index].Shaft, FootColor);
	}
}

void UBallAnimatorComponent::PoseHand(FBallHandParts& Hand, float Fist, float ThumbSide) const
{
	const float Curl = FMath::Lerp(RelaxedCurl, FistCurl, Fist);
	if (FMath::Abs(Curl - Hand.AppliedCurl) < 0.5f || Hand.Fingers.Num() < 5)
	{
		return;
	}
	Hand.AppliedCurl = Curl;

	// Hand space: X = along the fingers, Y = across the knuckles, Z = back of the hand. Fingers curl toward the palm (-Z).
	auto PlaceFinger = [this](UStaticMeshComponent* Bone, UStaticMeshComponent* Tip, const FVector& Knuckle, const FVector& Direction, float Length, float Thickness)
	{
		const FRotator AlongFinger = FRotationMatrix::MakeFromZ(Direction).Rotator();
		Bone->SetRelativeTransform(FTransform(AlongFinger, Knuckle + Direction * (Length * 0.5f), FVector(Thickness, Thickness, Length) / 100.f));
		Tip->SetRelativeTransform(FTransform(FRotator::ZeroRotator, Knuckle + Direction * Length, FVector(Thickness * 1.05f) / 100.f));
	};

	const float CurlRadians = FMath::DegreesToRadians(Curl);
	const FVector FingerDirection(FMath::Cos(CurlRadians), 0.f, -FMath::Sin(CurlRadians));
	const float Lengths[4] = { 9.f, 10.f, 9.5f, 7.5f };   // index, middle, ring, little
	for (int32 Finger = 0; Finger < 4; ++Finger)
	{
		// The index finger sits next to the thumb
		const FVector Knuckle(PalmSize.X * 0.35f, ThumbSide * (6.f - 4.f * Finger), 0.5f);
		PlaceFinger(Hand.Fingers[Finger], Hand.Tips[Finger], Knuckle, FingerDirection, Lengths[Finger], FingerThickness);
	}

	// Thumb: sticks out forward when relaxed, folds across the curled fingers in a fist
	const FVector OpenThumb(0.7f, ThumbSide * 0.6f, -0.3f);
	const FVector FistThumb(0.45f, -ThumbSide * 0.55f, -0.7f);
	const FVector ThumbDirection = FMath::Lerp(OpenThumb, FistThumb, Fist).GetSafeNormal();
	PlaceFinger(Hand.Fingers[4], Hand.Tips[4], FVector(0.5f, ThumbSide * 7.f, -2.f), ThumbDirection, 7.f, FingerThickness * 1.1f);
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
	if (!Ball || !Hands[0].Root || !BodyPivot || !LimbRoot)
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

		const FVector DeadFoot = bSkeletonPose
			? FVector(-96.f, Side(Index) * 16.f, GroundZ + FootSize.Z * 0.5f)
			: FVector(Radius + 8.f, Side(Index) * 24.f, GroundZ + FootSize.Z * 0.5f);
		Local = FMath::Lerp(Local, DeadFoot, DeadBlend);
		Boots[Index].Root->SetRelativeLocation(Local);
	}

	// Guard: fists up in front of the face
	const UBallMeleeComponent* Melee = Ball->GetMelee();
	GuardBlend = FMath::FInterpTo(GuardBlend, (Melee && Melee->IsGuarding() && !bDead) ? 1.f : 0.f, DeltaTime, 12.f);
	int32 PunchHand = 0;
	const float PunchExtension = Melee ? Melee->GetPunchExtension(PunchHand) : 0.f;
	const FVector AimLocal = Root.InverseTransformVectorNoScale(Ball->GetBaseAimRotation().Vector());

	// Hands: swing with the opposite foot (left hand forward when the right foot is forward)
	const FVector ThirdPersonRest(8.f, Radius + 14.f, CenterZ - 4.f);
	const FVector FirstPersonRest(Radius * 0.55f + 95.f, 45.f, CenterZ - 32.f);
	const FVector HandRest = FMath::Lerp(ThirdPersonRest, FirstPersonRest, FirstPersonBlend);
	const FVector GuardRest = FMath::Lerp(FVector(Radius + 20.f, 17.f, CenterZ + 14.f), FVector(Radius * 0.55f + 58.f, 19.f, CenterZ + 2.f), FirstPersonBlend);
	const float SwingScale = HandSwingPerFootOffset * FMath::Lerp(1.f, 0.3f, FirstPersonBlend) * (1.f - AirBlend) * (1.f - GuardBlend);
	for (int32 Index = 0; Index < 2; ++Index)
	{
		FVector Hand(HandRest.X, Side(Index) * HandRest.Y, HandRest.Z);
		Hand.X += FMath::Clamp(FootOffsetX[1 - Index] * SwingScale, -28.f, 28.f);
		Hand.Z += FMath::Sin(IdleTime * 2.2f + Side(Index)) * 1.5f;
		Hand += FVector(0.f, Side(Index) * 8.f, 22.f) * AirBlend;
		Hand = FMath::Lerp(Hand, FVector(GuardRest.X, Side(Index) * GuardRest.Y, GuardRest.Z), GuardBlend);

		// Which way the hand faces (hand X = along the fingers, Z = back of the hand)
		const FQuat RelaxedRotation = FQuat::Slerp(
			FRotationMatrix::MakeFromXZ(FVector(0.35f, 0.f, -1.f), FVector(0.f, Side(Index), 0.f)).ToQuat(),
			FRotationMatrix::MakeFromXZ(FVector(1.f, 0.f, -0.6f), FVector(0.f, Side(Index) * 0.5f, 1.f)).ToQuat(),
			FirstPersonBlend);
		FQuat TargetRotation = GuardBlend > 0.5f
			? FRotationMatrix::MakeFromXZ(FVector(0.9f, 0.f, 0.45f), FVector(0.f, Side(Index) * 0.8f, 0.6f)).ToQuat()
			: RelaxedRotation;
		bool bFist = GuardBlend > 0.5f;

		// Punch: the fist shoots out toward where the ball is aiming, knuckles first, then pulls back
		if (PunchExtension > 0.f && PunchHand == Index)
		{
			const float PunchReach = FMath::Lerp(Radius + 55.f, FirstPersonRest.X + 25.f, FirstPersonBlend);
			const FVector PunchTarget = FVector(0.f, 0.f, CenterZ) + AimLocal * PunchReach + FVector(0.f, Side(Index) * 10.f, 0.f);
			Hand = FMath::Lerp(Hand, PunchTarget, PunchExtension);
			TargetRotation = FRotationMatrix::MakeFromXZ(AimLocal, FVector::UpVector).ToQuat();
			bFist = true;
		}

		Hand = KeepHandOutOfWalls(Index, FVector(0.f, 0.f, CenterZ), Hand, DeltaTime);

		const FVector DeadHand = bSkeletonPose
			? FVector(-30.f, Side(Index) * 54.f, GroundZ + HandSize * 0.5f)
			: FVector(-5.f, Side(Index) * (Radius + 10.f), GroundZ + HandSize * 0.5f);
		Hand = FMath::Lerp(Hand, DeadHand, DeadBlend);
		if (DeadBlend > 0.5f)
		{
			TargetRotation = FRotationMatrix::MakeFromXZ(FVector::ForwardVector, FVector::UpVector).ToQuat();
			bFist = false;
		}

		HandRotation[Index] = FQuat::Slerp(HandRotation[Index], TargetRotation, 1.f - FMath::Exp(-DeltaTime * 18.f));
		FistAmount[Index] = FMath::FInterpTo(FistAmount[Index], bFist ? 1.f : 0.f, DeltaTime, 20.f);

		// The thumb is on the side of the hand facing the body
		PoseHand(Hands[Index], FistAmount[Index], -Side(Index));
		Hands[Index].Root->SetRelativeLocationAndRotation(Hand, HandRotation[Index]);
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
