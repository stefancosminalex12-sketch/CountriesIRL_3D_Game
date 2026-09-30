// Crowns & Commoners

#include "Animals/Horse.h"
#include "Animals/HorseAnimInstance.h"
#include "Characters/BallCharacter.h"
#include "Characters/StaminaComponent.h"
#include "Characters/HealthComponent.h"
#include "Engine/DamageEvents.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AHorse::AHorse()
{
	PrimaryActorTick.bCanEverTick = true;

	Stamina = CreateDefaultSubobject<UStaminaComponent>(TEXT("Stamina"));
	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));

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

	if (Definition)
	{
		Health->SetMaxHealth(Definition->MaxHealth);
	}
	Health->OnDepleted.AddDynamic(this, &AHorse::HandleDeath);
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

bool AHorse::IsDead() const
{
	return Health->IsDepleted();
}

float AHorse::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	return Health->ApplyDamage(Damage);
}

void AHorse::HandleDeath(UHealthComponent* DepletedHealth)
{
	if (Rider)
	{
		Rider->Dismount();
	}
	DesiredDirection = FVector::ZeroVector;
	Gait = EHorseGait::Walk;
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	if (UHorseAnimInstance* Anim = Cast<UHorseAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->PlayOneShot(Definition ? Definition->DeathAnim.Get() : nullptr, true);
	}
}

void AHorse::SetRider(ABallCharacter* NewRider)
{
	if (Rider)
	{
		GetCapsuleComponent()->IgnoreActorWhenMoving(Rider, false);
	}
	Rider = NewRider;
	DesiredDirection = FVector::ZeroVector;
	RequestedGait = EHorseGait::Walk;
	if (Rider)
	{
		// The rider sits on top of us: don't let our own movement bump into them
		GetCapsuleComponent()->IgnoreActorWhenMoving(Rider, true);
	}
}

void AHorse::SetRiderInput(const FVector& Direction, EHorseGait NewGait)
{
	DesiredDirection = Direction.GetClampedToMaxSize(1.f);
	DesiredDirection.Z = 0.f;
	RequestedGait = NewGait;
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

	if (IsDead())
	{
		return;
	}

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	const float Throttle = DesiredDirection.Size();
	const float Speed = GetVelocity().Size2D();

	if (Throttle > 0.05f)
	{
		// Turn toward where the rider wants to go, slower the faster we run (wide turns at a gallop)
		const float TurnRate = Definition->GetTurnRateAtSpeed(Speed);
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

	// Walk and trot cost nothing. Canter and gallop need the horse moving forward on the ground with
	// breath left (the gallop also a rider who isn't exhausted); otherwise it drops a gait:
	// a tired rider can still canter, a tired horse falls back to the travelling trot
	Gait = RequestedGait;
	const bool bCanRunFast = Throttle > 0.5f && Movement->IsMovingOnGround() && Stamina->HasStamina();
	const bool bRiderFresh = !Rider || Rider->GetStamina()->HasStamina();
	if (Gait == EHorseGait::Gallop && !(bCanRunFast && bRiderFresh))
	{
		Gait = EHorseGait::Canter;
	}
	if (Gait == EHorseGait::Canter && !bCanRunFast)
	{
		Gait = EHorseGait::Trot;
	}
	if (Gait == EHorseGait::Gallop)
	{
		Stamina->Drain(Definition->GallopStaminaPerSecond, DeltaTime);
	}
	else if (Gait == EHorseGait::Canter)
	{
		Stamina->Drain(Definition->CanterStaminaPerSecond, DeltaTime);
	}
	Movement->MaxWalkSpeed = Definition->GetGaitSpeed(Gait);

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
