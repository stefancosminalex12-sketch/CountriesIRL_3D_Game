// CountriesIRL 3D Game

#include "Animals/Horse.h"
#include "Animals/HorseAnimInstance.h"
#include "Animals/MountDefinition.h"
#include "Characters/BallCharacter.h"
#include "Characters/StaminaComponent.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AHorse::AHorse()
{
	PrimaryActorTick.bCanEverTick = true;

	Stamina = CreateDefaultSubobject<UStaminaComponent>(TEXT("Stamina"));

	// Horses aren't steered by the controller's view; they turn themselves in Tick
	bUseControllerRotationYaw = false;
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = false;
	Movement->GroundFriction = 6.f;
	Movement->bUseSeparateBrakingFriction = true;
	Movement->BrakingFriction = 0.5f;

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// An AI controller lets the horse move without a player possessing it (the rider steers it)
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AHorse::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyDefinition();
}

void AHorse::BeginPlay()
{
	Super::BeginPlay();
	ApplyDefinition();

	// Horses stand upright: only their heading comes from how they were placed
	SetActorRotation(FRotator(0.f, GetActorRotation().Yaw, 0.f));
}

void AHorse::ApplyDefinition()
{
	if (!Definition)
	{
		return;
	}

	GetCapsuleComponent()->SetCapsuleSize(Definition->CapsuleRadius, Definition->CapsuleHalfHeight);

	USkeletalMeshComponent* Body = GetMesh();
	Body->SetSkeletalMesh(Definition->Mesh);
	Body->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -Definition->CapsuleHalfHeight), FRotator(0.f, Definition->MeshYaw, 0.f));
	if (Body->GetAnimInstance() == nullptr || !Body->GetAnimInstance()->IsA<UHorseAnimInstance>())
	{
		Body->SetAnimInstanceClass(UHorseAnimInstance::StaticClass());
	}

	Stamina->SetMaxStamina(Definition->MaxStamina);

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = Definition->WalkSpeed;
	Movement->MaxAcceleration = Definition->Acceleration;
	Movement->BrakingDecelerationWalking = Definition->Acceleration * 1.3f;
	Movement->JumpZVelocity = Definition->JumpVelocity;
}

void AHorse::SetRider(ABallCharacter* NewRider)
{
	if (Rider)
	{
		GetCapsuleComponent()->IgnoreActorWhenMoving(Rider, false);
	}
	Rider = NewRider;
	DesiredDirection = FVector::ZeroVector;
	bWantsGallop = false;
	if (Rider)
	{
		// The rider sits on top of us: don't let our own movement bump into them
		GetCapsuleComponent()->IgnoreActorWhenMoving(Rider, true);
	}
}

void AHorse::SetRiderInput(const FVector& Direction, bool bGallop)
{
	DesiredDirection = Direction.GetClampedToMaxSize(1.f);
	DesiredDirection.Z = 0.f;
	bWantsGallop = bGallop;
}

void AHorse::RiderJump()
{
	Jump();
}

bool AHorse::CanJumpInternal_Implementation() const
{
	return Definition && Super::CanJumpInternal_Implementation() && Stamina->HasStamina(Definition->JumpStaminaCost);
}

void AHorse::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	if (!Definition)
	{
		return;
	}
	Stamina->TryConsume(Definition->JumpStaminaCost);
	if (UHorseAnimInstance* Anim = Cast<UHorseAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->PlayOneShot(Definition->JumpAnim);
	}
}

void AHorse::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!Definition)
	{
		return;
	}

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	const float Throttle = DesiredDirection.Size();
	const float Speed = GetVelocity().Size2D();

	if (Throttle > 0.05f)
	{
		// Turn toward where the rider wants to go, slower the faster we run (wide turns at a gallop)
		const float SpeedAlpha = FMath::GetMappedRangeValueClamped(FVector2D(Definition->WalkSpeed, Definition->GallopSpeed), FVector2D(0.f, 1.f), Speed);
		const float TurnRate = FMath::Lerp(Definition->WalkTurnRate, Definition->GallopTurnRate, SpeedAlpha);
		const float NewYaw = FMath::FixedTurn(GetActorRotation().Yaw, DesiredDirection.Rotation().Yaw, TurnRate * DeltaTime);
		SetActorRotation(FRotator(0.f, NewYaw, 0.f));

		// Horses only go forward: asked to go the other way, they turn around at a slow walk first
		const float Facing = FVector::DotProduct(GetActorForwardVector(), DesiredDirection.GetSafeNormal());
		const float Drive = Throttle * FMath::GetMappedRangeValueClamped(FVector2D(-0.5f, 0.7f), FVector2D(0.25f, 1.f), Facing);
		if (!IsBlockedAhead())
		{
			AddMovementInput(GetActorForwardVector(), Drive);
		}
	}

	// Only the chest has collision, so look ahead and stop before the head goes into a wall
	if (IsBlockedAhead() && FVector::DotProduct(GetVelocity(), GetActorForwardVector()) > 0.f)
	{
		Movement->StopMovementImmediately();
	}

	// Gallop while asked, moving forward on the ground, and neither the horse nor its rider is out of breath
	const bool bRiderFresh = !Rider || Rider->GetStamina()->HasStamina();
	bGalloping = bWantsGallop && Throttle > 0.5f && Movement->IsMovingOnGround() && Stamina->HasStamina() && bRiderFresh;
	if (bGalloping)
	{
		Stamina->Drain(Definition->GallopStaminaPerSecond, DeltaTime);
	}
	Movement->MaxWalkSpeed = bGalloping ? Definition->GallopSpeed : Definition->WalkSpeed;
}

FVector AHorse::GetSaddleOffset() const
{
	if (!Definition)
	{
		return FVector::ZeroVector;
	}
	const FVector Bone = GetMesh()->GetSocketLocation(Definition->SaddleBone);
	return GetActorTransform().InverseTransformPosition(Bone) + FVector(0.f, 0.f, Definition->SaddleHeight);
}

bool AHorse::IsBlockedAhead() const
{
	const FVector Chest = GetActorLocation() + FVector(0.f, 0.f, Definition->CapsuleHalfHeight * 0.3f);
	const FVector Head = Chest + GetActorForwardVector() * Definition->HeadReach;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(HorseHeadCheck), false, this);
	if (Rider)
	{
		Params.AddIgnoredActor(Rider);
	}
	FHitResult Hit;
	return GetWorld()->SweepSingleByChannel(Hit, Chest, Head, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(22.f), Params);
}

float AHorse::GetBodyHalfWidth() const
{
	return Definition ? Definition->BodyHalfWidth : 25.f;
}
