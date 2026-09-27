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

void UBallAnimatorComponent::CreateLimbMeshes(AActor* Owner, USceneComponent* LimbParent, USceneComponent* InBodyPivot, UStaticMesh* SphereMesh,
	float BallRadius, float BallCenterZ, float GroundZ)
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
			auto FingerPart = [&](const TCHAR* Part, UStaticMesh* Mesh)
			{
				return BallParts::Create(Owner, PartName(FString::Printf(TEXT("Finger%d%s"), Finger, Part)), Hand.Root, Mesh);
			};
			Hand.BaseSegments.Add(FingerPart(TEXT("Base"), CylinderMesh));
			Hand.Joints.Add(FingerPart(TEXT("Joint"), SphereMesh));
			Hand.OuterSegments.Add(FingerPart(TEXT("Outer"), CylinderMesh));
			Hand.Tips.Add(FingerPart(TEXT("Tip"), SphereMesh));
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

		// Resting pose: relaxed hands at the sides, boots under the ball
		const float Side = Index == 0 ? -1.f : 1.f;
		PoseHand(Hand, 0.f, -Side);
		HandRotation[Index] = FRotationMatrix::MakeFromXZ(FVector(0.35f, 0.f, -1.f), FVector(0.f, Side, 0.f)).ToQuat();
		Hand.Root->SetRelativeLocationAndRotation(FVector(8.f, Side * (BallRadius + 14.f), BallCenterZ - 4.f), HandRotation[Index]);
		Boot.Root->SetRelativeLocation(FVector(2.f, Side * FootHalfSpacing, GroundZ + FootSize.Z * 0.5f));
	}
}

void UBallAnimatorComponent::BeginPlay()
{
	Super::BeginPlay();
	bFeetPlanted = false;
}

void UBallAnimatorComponent::ApplyColors(const FLinearColor& HandColor, const FLinearColor& FootColor)
{
	TArray<UStaticMeshComponent*> HandParts;
	TArray<UStaticMeshComponent*> BootParts;
	TArray<UStaticMeshComponent*> SoleParts;
	for (int32 Index = 0; Index < 2; ++Index)
	{
		const FBallHandParts& Hand = Hands[Index];
		HandParts.Add(Hand.Palm);
		for (const TArray<TObjectPtr<UStaticMeshComponent>>* Group : { &Hand.BaseSegments, &Hand.Joints, &Hand.OuterSegments, &Hand.Tips })
		{
			for (UStaticMeshComponent* Part : *Group)
			{
				HandParts.Add(Part);
			}
		}
		BootParts.Add(Boots[Index].Upper);
		BootParts.Add(Boots[Index].Shaft);
		SoleParts.Add(Boots[Index].Sole);
	}

	ShareColor(HandParts, HandColor);
	ShareColor(BootParts, FootColor);
	ShareColor(SoleParts, (FootColor * 0.45f).CopyWithNewOpacity(1.f));
}

void UBallAnimatorComponent::ShareColor(const TArray<UStaticMeshComponent*>& Parts, const FLinearColor& Color)
{
	if (Parts.Num() == 0 || !Parts[0])
	{
		return;
	}

	// The first part owns the material; the rest point at the same one
	BallParts::SetColor(Parts[0], Color);
	UMaterialInterface* Shared = Parts[0]->GetMaterial(0);
	for (int32 Index = 1; Index < Parts.Num(); ++Index)
	{
		if (Parts[Index] && Parts[Index]->GetMaterial(0) != Shared)
		{
			Parts[Index]->SetMaterial(0, Shared);
		}
	}
}

void UBallAnimatorComponent::PoseHand(FBallHandParts& Hand, float Fist, float ThumbSide) const
{
	if (FMath::Abs(Fist - Hand.AppliedFist) < 0.005f || Hand.BaseSegments.Num() < 5)
	{
		return;
	}
	Hand.AppliedFist = Fist;

	// A bone is a cylinder stretched between two points; joints and tips are small spheres
	auto PlaceBone = [](UStaticMeshComponent* Bone, const FVector& From, const FVector& To, float Thickness)
	{
		const FVector Delta = To - From;
		const float Length = FMath::Max(Delta.Size(), 0.01f);
		Bone->SetRelativeTransform(FTransform(FRotationMatrix::MakeFromZ(Delta / Length).Rotator(), (From + To) * 0.5f, FVector(Thickness, Thickness, Length) / 100.f));
	};
	auto PlaceBall = [](UStaticMeshComponent* Ball, const FVector& Location, float Size)
	{
		Ball->SetRelativeTransform(FTransform(FRotator::ZeroRotator, Location, FVector(Size / 100.f)));
	};
	auto PlaceFinger = [&](int32 Finger, const FVector& Knuckle, const FVector& Joint, const FVector& Tip, float Thickness)
	{
		PlaceBone(Hand.BaseSegments[Finger], Knuckle, Joint, Thickness);
		PlaceBall(Hand.Joints[Finger], Joint, Thickness * 1.02f);
		PlaceBone(Hand.OuterSegments[Finger], Joint, Tip, Thickness);
		PlaceBall(Hand.Tips[Finger], Tip, Thickness * 1.05f);
	};

	// Hand space: X = along the fingers, Y = across the knuckles, Z = back of the hand. Fingers bend toward the palm (-Z).
	// In a fist the knuckle bends ~85 degrees and the middle joint ~100, so the fingers roll down and back under the palm.
	const float KnuckleBend = FMath::DegreesToRadians(FMath::Lerp(RelaxedJointBend, FistJointBend.X, Fist));
	const float TotalBend = KnuckleBend + FMath::DegreesToRadians(FMath::Lerp(RelaxedJointBend, FistJointBend.Y, Fist));
	const FVector BaseDirection(FMath::Cos(KnuckleBend), 0.f, -FMath::Sin(KnuckleBend));
	const FVector OuterDirection(FMath::Cos(TotalBend), 0.f, -FMath::Sin(TotalBend));

	const float Lengths[4] = { 9.f, 10.f, 9.5f, 7.5f };   // index, middle, ring, little
	for (int32 Finger = 0; Finger < 4; ++Finger)
	{
		// The index finger sits next to the thumb; fingers pack snugly like a glove
		const FVector Knuckle(PalmSize.X * 0.35f, ThumbSide * (5.2f - 3.47f * Finger), 0.5f);
		const FVector Joint = Knuckle + BaseDirection * (Lengths[Finger] * 0.55f);
		PlaceFinger(Finger, Knuckle, Joint, Joint + OuterDirection * (Lengths[Finger] * 0.45f), FingerThickness);
	}

	// Thumb: rooted on the side of the palm. Relaxed it points forward and out; in a fist it runs forward
	// along the side of the fist, then bends across the front of the curled fingers.
	const float ThumbThickness = FingerThickness * 1.1f;
	const FVector ThumbRoot(1.f, ThumbSide * 9.7f, -1.5f);
	const FVector RelaxedJoint = ThumbRoot + FVector(0.7f, ThumbSide * 0.55f, -0.45f).GetSafeNormal() * 4.5f;
	const FVector RelaxedTip = RelaxedJoint + FVector(0.8f, ThumbSide * 0.45f, -0.4f).GetSafeNormal() * 3.5f;
	const FVector FistJoint(9.6f, ThumbSide * 9.7f, -3.5f);
	const FVector FistTip(9.6f, ThumbSide * 5.f, -3.5f);
	PlaceFinger(4, ThumbRoot, FMath::Lerp(RelaxedJoint, FistJoint, Fist), FMath::Lerp(RelaxedTip, FistTip, Fist), ThumbThickness);
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
		const FVector Rest = FootRestWorld(Index);
		Landing[Index] = KeepOnOwnSide(Index, Rest + Velocity2D * (StepTime * 0.5f));
		// Each foot finds the ground exactly where it will land (steps, slopes, bodies, edges)
		Landing[Index].Z = GroundZAt(Landing[Index], Rest.Z);
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
	// Riding: the feet leave the ground and hang in the stirrups
	RideBlend = FMath::FInterpTo(RideBlend, bRiding && !bDead ? 1.f : 0.f, DeltaTime, 8.f);
	if (!bDead && !bRiding)
	{
		UpdateFeet(DeltaTime, SpeedAlpha, bFalling);
	}
	const float BallBottomZ = CenterZ - Ball->GetBallHalfHeight();

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

		const FVector RidingFoot(2.f, Side(Index) * (RidingHalfWidth + FootSize.Y * 0.5f + 3.f), BallBottomZ - 26.f);
		Local = FMath::Lerp(Local, RidingFoot, RideBlend);

		// Bones stage: the boots lie past the pelvis, at the opposite end from the skull
		const FVector DeadFoot = bSkeletonPose
			? FVector(80.f, Side(Index) * 14.f, GroundZ + FootSize.Z * 0.5f)
			: FVector(Radius + 8.f, Side(Index) * 24.f, GroundZ + FootSize.Z * 0.5f);
		Local = FMath::Lerp(Local, DeadFoot, DeadBlend);
		Boots[Index].Root->SetRelativeLocation(Local);
	}

	// Guard: fists up in front of the face
	const UBallMeleeComponent* Melee = Ball->GetMelee();
	GuardBlend = FMath::FInterpTo(GuardBlend, (Melee && Melee->IsGuarding() && !bDead) ? 1.f : 0.f, DeltaTime, 12.f);
	int32 PunchHand = 0;
	const float PunchExtension = Melee ? Melee->GetPunchExtension(PunchHand) : 0.f;
	const bool bPunching = Melee && Melee->IsPunching() && !bDead;
	const float PunchEnvelope = bPunching ? Melee->GetPunchEnvelope() : 0.f;
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
		// Riding: both fists low in front, holding the reins
		const FVector Reins = FMath::Lerp(FVector(Radius + 14.f, 14.f, CenterZ - 22.f), FVector(Radius * 0.55f + 62.f, 22.f, CenterZ - 42.f), FirstPersonBlend);
		Hand = FMath::Lerp(Hand, FVector(Reins.X, Side(Index) * Reins.Y, Reins.Z), RideBlend);
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
		if (RideBlend > 0.5f && GuardBlend < 0.5f)
		{
			TargetRotation = FRotationMatrix::MakeFromXZ(FVector(1.f, 0.f, -0.3f), FVector(0.f, Side(Index), 0.2f)).ToQuat();
			bFist = true;
		}

		// The other fist comes up to protect the face while punching
		const FVector GuardSpot(GuardRest.X, Side(Index) * GuardRest.Y, GuardRest.Z);
		if (bPunching && PunchHand != Index)
		{
			Hand = FMath::Lerp(Hand, GuardSpot, PunchEnvelope);
			bFist = true;
		}

		// Punch: a short wind-up (pull back and in), a snap forward knuckles-first that twists from a
		// vertical fist to palm-down (corkscrew), then the recoil
		bool bDirectRotation = false;
		if (bPunching && PunchHand == Index)
		{
			if (PunchExtension < 0.f)
			{
				const FVector WindUp = Hand + FVector(-12.f, -Side(Index) * 5.f, -3.f);
				Hand = FMath::Lerp(Hand, WindUp, FMath::Min(-PunchExtension / Melee->GetWindUpPull(), 1.f));
			}
			else
			{
				const float PunchReach = FMath::Lerp(Radius + 55.f, FirstPersonRest.X + 25.f, FirstPersonBlend);
				const FVector PunchTarget = FVector(0.f, 0.f, CenterZ) + AimLocal * PunchReach + FVector(0.f, Side(Index) * 10.f, 0.f);
				Hand = FMath::Lerp(Hand, PunchTarget, PunchExtension);
			}

			const FQuat Vertical = FRotationMatrix::MakeFromXZ(AimLocal, FVector(0.f, Side(Index), 0.f)).ToQuat();
			const FQuat PalmDown = FRotationMatrix::MakeFromXZ(AimLocal, FVector::UpVector).ToQuat();
			TargetRotation = FQuat::Slerp(Vertical, PalmDown, FMath::Clamp(PunchExtension, 0.f, 1.f));
			bDirectRotation = true;
			bFist = true;
		}

		Hand = KeepHandOutOfWalls(Index, FVector(0.f, 0.f, CenterZ), Hand, DeltaTime);

		// Lying flat on the ground: beside the ball, or beside the ribs once only bones are left
		const float LyingHandZ = GroundZ + PalmSize.Z * 0.5f + 1.f;
		const FVector DeadHand = bSkeletonPose
			? FVector(30.f, Side(Index) * 42.f, LyingHandZ)
			: FVector(-5.f, Side(Index) * (Radius + 10.f), LyingHandZ);
		Hand = FMath::Lerp(Hand, DeadHand, DeadBlend);
		if (DeadBlend > 0.5f)
		{
			TargetRotation = FRotationMatrix::MakeFromXZ(FVector::ForwardVector, FVector::UpVector).ToQuat();
			bFist = false;
		}

		// Punches are too fast for smoothing; everything else eases
		HandRotation[Index] = bDirectRotation ? TargetRotation : FQuat::Slerp(HandRotation[Index], TargetRotation, 1.f - FMath::Exp(-DeltaTime * 18.f));
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
	// Punching: the body winds up, then twists into the punch (the punching side goes forward), leans and lunges
	const float Drive = bPunching ? PunchExtension : 0.f;
	const FRotator PunchTurn(-PunchLean * FMath::Max(Drive, 0.f), -Side(PunchHand) * PunchTwist * Drive, 0.f);
	const FVector AliveLocation(PunchLunge * FMath::Max(Drive, 0.f), 0.f, CenterZ + Bob);
	const FVector DeadLocation(0.f, 0.f, GroundZ + Radius * BodyScale);
	const FQuat Rotation = FQuat::Slerp((Lean + PunchTurn).Quaternion(), FRotator(70.f, 0.f, 12.f).Quaternion(), DeadBlend);

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

void UBallAnimatorComponent::SetRiding(bool bEnable, float MountHalfWidth)
{
	bRiding = bEnable;
	RidingHalfWidth = MountHalfWidth;
	// Back on the ground, the feet find new footing instead of stepping back to where they were
	bFeetPlanted = false;
}
