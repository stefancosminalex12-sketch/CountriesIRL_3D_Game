// CountriesIRL 3D Game

#include "Characters/BallCharacter.h"
#include "Characters/BallAnimatorComponent.h"
#include "Characters/BallParts.h"
#include "Characters/StaminaComponent.h"
#include "Characters/HealthComponent.h"
#include "Characters/CorpseComponent.h"
#include "Characters/BallSkeletonComponent.h"
#include "Characters/BallMeleeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ABallCharacter::ABallCharacter()
{
	// Capsule spans from the ground up to the top of the ball
	const float HalfHeight = (BallRadius * 2.f + FeetGap) * 0.5f;
	GetCapsuleComponent()->InitCapsuleSize(BallRadius * 0.9f, HalfHeight);

	// Balls don't use a skeleton; everything is procedural
	GetMesh()->SetVisibility(false);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 540.f, 0.f);
	Movement->MaxWalkSpeed = WalkSpeed;
	Movement->MinAnalogWalkSpeed = 20.f;
	Movement->BrakingDecelerationWalking = 2000.f;
	Movement->JumpZVelocity = JumpVelocity;
	Movement->AirControl = 0.35f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UStaticMesh* Sphere = BallParts::LoadSphere();

	VisualRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VisualRoot"));
	VisualRoot->SetupAttachment(GetCapsuleComponent());

	BodyPivot = CreateDefaultSubobject<USceneComponent>(TEXT("BodyPivot"));
	BodyPivot->SetupAttachment(VisualRoot);
	BodyPivot->SetRelativeLocation(FVector(0.f, 0.f, GetBallCenterZ()));

	BodyMesh = BallParts::Create(this, TEXT("BodyMesh"), BodyPivot, Sphere);
	BodyMesh->SetRelativeScale3D(FVector(BallRadius / 50.f));

	Face = CreateDefaultSubobject<UBallFaceComponent>(TEXT("Face"));
	Face->CreateFaceMesh(this, BodyPivot, Sphere, BallRadius);

	Animator = CreateDefaultSubobject<UBallAnimatorComponent>(TEXT("Animator"));
	Animator->CreateLimbMeshes(this, VisualRoot, BodyPivot, Sphere);

	Stamina = CreateDefaultSubobject<UStaminaComponent>(TEXT("Stamina"));
	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	Corpse = CreateDefaultSubobject<UCorpseComponent>(TEXT("Corpse"));
	Skeleton = CreateDefaultSubobject<UBallSkeletonComponent>(TEXT("Skeleton"));
	Melee = CreateDefaultSubobject<UBallMeleeComponent>(TEXT("Melee"));

	// Eyes are where the ball looks and punches from
	BaseEyeHeight = GetBallCenterZ() + 10.f;
}

void ABallCharacter::BeginPlay()
{
	Super::BeginPlay();

	RefreshColors();

	SetEmotion(StartingEmotion);
	SetSprinting(false);

	Health->OnDepleted.AddDynamic(this, &ABallCharacter::HandleDeath);
}

bool ABallCharacter::IsDead() const
{
	return Health->IsDepleted();
}

float ABallCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	Damage = Melee->ModifyIncomingDamage(Damage, DamageCauser);
	const float Removed = Health->ApplyDamage(Damage);
	if (Removed > 0.f)
	{
		// Minecraft-style red flash while being hurt
		DamageFlashTime = DamageFlashDuration;
	}
	return Removed;
}

void ABallCharacter::RefreshColors()
{
	const float Flash = DamageFlashDuration > 0.f ? FMath::Clamp(DamageFlashTime / DamageFlashDuration, 0.f, 1.f) : 0.f;
	auto Shade = [this, Flash](const FLinearColor& Base, bool bDecays)
	{
		FLinearColor Color = Base;
		if (bDecays && Corpse->IsDecaying())
		{
			Color = FMath::Lerp(Color, Corpse->GetTint(), Corpse->GetTintStrength());
		}
		return FMath::Lerp(Color, DamageFlashColor, Flash * DamageFlashStrength);
	};

	// Feet are boots: they don't rot
	BallParts::SetColor(BodyMesh, Shade(BodyColor, true));
	Animator->ApplyColors(Shade(HandColor, true), Shade(FootColor, false));
}

void ABallCharacter::HandleDeath(UHealthComponent* DepletedHealth)
{
	SetEmotion(EBallEmotion::Dead);
	SetSprinting(false);
	GetCharacterMovement()->DisableMovement();

	// The living can walk over the dead; traces (looting, interaction) still hit them
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	Corpse->StartDecay();
}

float ABallCharacter::GetBallCenterZ() const
{
	// Capsule center sits halfway between the ground and the top of the ball
	return FeetGap + BallRadius - (BallRadius * 2.f + FeetGap) * 0.5f;
}

float ABallCharacter::GetGroundOffset() const
{
	return GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
}

void ABallCharacter::SetEmotion(EBallEmotion NewEmotion)
{
	Face->SetEmotion(NewEmotion);
}

EBallEmotion ABallCharacter::GetEmotion() const
{
	return Face->GetEmotion();
}

void ABallCharacter::SetSprinting(bool bNewSprinting)
{
	bWantsToRun = bNewSprinting;
}

void ABallCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Colors change while flashing red and while a corpse decays
	const bool bFlashing = DamageFlashTime > 0.f;
	DamageFlashTime = FMath::Max(DamageFlashTime - DeltaTime, 0.f);
	if (bFlashing || Corpse->IsDecaying())
	{
		RefreshColors();
	}

	if (IsDead())
	{
		if (Corpse->IsDecaying())
		{
			BodyPivot->SetRelativeScale3D(FVector(Corpse->GetBodyScale()));

			// Decayed to bones: swap the ball for the cartoon skeleton
			if (Corpse->IsSkeleton() && !Skeleton->IsShown())
			{
				Skeleton->Show(VisualRoot, -GetGroundOffset(), Corpse->GetTint());
				BodyMesh->SetVisibility(false);
				Face->SetFaceVisible(false);
				Animator->SetSkeletonPose(true);
			}
		}
		return;
	}

	// Only drain stamina while actually running on the ground, not while holding the key standing still
	const bool bMovingOnGround = GetVelocity().Size2D() > 10.f && GetCharacterMovement()->IsMovingOnGround();
	// No running with the guard up, and moving is slower
	const bool bGuarding = Melee->IsGuarding();
	bRunning = bWantsToRun && bMovingOnGround && !bGuarding && Stamina->HasStamina();
	if (bRunning)
	{
		Stamina->Drain(RunStaminaCost, DeltaTime);
	}

	GetCharacterMovement()->MaxWalkSpeed = bRunning ? RunSpeed : (bGuarding ? WalkSpeed * Melee->GetGuardMoveSpeedScale() : WalkSpeed);
}

bool ABallCharacter::CanJumpInternal_Implementation() const
{
	return Super::CanJumpInternal_Implementation() && Stamina->HasStamina(JumpStaminaCost);
}

void ABallCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	Stamina->TryConsume(JumpStaminaCost);
}
