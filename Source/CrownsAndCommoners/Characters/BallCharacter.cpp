// Crowns & Commoners

#include "Characters/BallCharacter.h"
#include "Characters/BallAnimatorComponent.h"
#include "Characters/BallParts.h"
#include "Characters/StaminaComponent.h"
#include "Characters/HealthComponent.h"
#include "Characters/CorpseComponent.h"
#include "Characters/BallSkeletonComponent.h"
#include "Characters/BallMeleeComponent.h"
#include "Characters/Heraldry.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Animals/Horse.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ABallCharacter::ABallCharacter()
{
	// Capsule spans from the ground up to the top of the ball
	const float HalfHeight = (GetBallHalfHeight() * 2.f + FeetGap) * 0.5f;
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
	// Coat of arms across the ball (M_BallArms projects the texture from the front)
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> FlagMaterialAsset(TEXT("/Game/CrownsAndCommoners/Characters/Materials/M_BallArms.M_BallArms"));
	if (FlagMaterialAsset.Succeeded())
	{
		BodyMesh->SetMaterial(0, FlagMaterialAsset.Object);
	}
	BodyMesh->SetRelativeScale3D(FVector(BallRadius, BallRadius, GetBallHalfHeight()) / 50.f);

	Face = CreateDefaultSubobject<UBallFaceComponent>(TEXT("Face"));
	Face->CreateFaceMesh(this, BodyPivot, Sphere, BallRadius, BallHeightScale);

	Animator = CreateDefaultSubobject<UBallAnimatorComponent>(TEXT("Animator"));
	Animator->CreateLimbMeshes(this, VisualRoot, BodyPivot, Sphere, BallRadius, GetBallCenterZ(), -HalfHeight);

	Stamina = CreateDefaultSubobject<UStaminaComponent>(TEXT("Stamina"));
	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	Corpse = CreateDefaultSubobject<UCorpseComponent>(TEXT("Corpse"));
	Skeleton = CreateDefaultSubobject<UBallSkeletonComponent>(TEXT("Skeleton"));
	Melee = CreateDefaultSubobject<UBallMeleeComponent>(TEXT("Melee"));

	// Other balls can land and stand on top of us (by default the engine bounces characters off each other)
	GetCapsuleComponent()->CanCharacterStepUpOn = ECB_Yes;

	// Corpse collision: off while alive, sized to the lying body or the bones after death
	CorpseCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CorpseCollision"));
	SkullCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SkullCollision"));
	for (UShapeComponent* Shape : TArray<UShapeComponent*>{ CorpseCollision, SkullCollision })
	{
		Shape->SetupAttachment(GetCapsuleComponent());
		Shape->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		Shape->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		Shape->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Shape->CanCharacterStepUpOn = ECB_Yes;
		Shape->SetCanEverAffectNavigation(false);
	}

	// Eyes are where the ball looks and punches from
	BaseEyeHeight = GetBallCenterZ() + 10.f;
}

void ABallCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	// Shows the arms in the editor too
	ApplyFlag();
	RefreshColors();
}

void ABallCharacter::SetFlag(UTexture2D* NewFlag)
{
	Flag = NewFlag;
	ApplyFlag();
}

void ABallCharacter::ApplyFlag()
{
	if (!FlagMaterial)
	{
		UMaterialInterface* Base = BodyMesh->GetMaterial(0);
		if (!Base || !Base->GetName().Contains(TEXT("M_BallArms")))
		{
			return;
		}
		FlagMaterial = BodyMesh->CreateDynamicMaterialInstance(0, Base);
	}
	UTexture2D* Arms = Flag;
	if (!Arms && CIRLHeraldry::All().Num() > 0)
	{
		Arms = CIRLHeraldry::LoadTexture(CIRLHeraldry::All()[0]);
	}
	if (FlagMaterial && Arms)
	{
		FlagMaterial->SetTextureParameterValue(TEXT("Flag"), Arms);
	}
}

void ABallCharacter::BeginPlay()
{
	Super::BeginPlay();
	ApplyFlag();

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

	// The ball wears its coat of arms: rotting and the damage flash tint it
	if (FlagMaterial)
	{
		FLinearColor Tint = DamageFlashColor;
		float Amount = Flash * DamageFlashStrength;
		if (Corpse->IsDecaying())
		{
			const float Decay = Corpse->GetTintStrength();
			Tint = FMath::Lerp(Corpse->GetTint(), DamageFlashColor, Amount);
			Amount = Decay + (1.f - Decay) * Amount;
		}
		FlagMaterial->SetVectorParameterValue(TEXT("Tint"), Tint);
		FlagMaterial->SetScalarParameterValue(TEXT("TintAmount"), Amount);
	}
	else
	{
		BallParts::SetColor(BodyMesh, Shade(BodyColor, true));
	}
	// Feet are boots: they don't rot
	Animator->ApplyColors(Shade(HandColor, true), Shade(FootColor, false));
}

void ABallCharacter::HandleDeath(UHealthComponent* DepletedHealth)
{
	// Killed in the saddle: fall off first
	Dismount();

	SetEmotion(EBallEmotion::Dead);
	SetSprinting(false);
	GetCharacterMovement()->DisableMovement();

	// The standing capsule no longer fits a body lying on the ground: a box shaped like the lying ball
	// takes over, so the living bump into it and can climb on it; traces (looting) hit it too
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	UpdateCorpseCollision(false);

	Corpse->StartDecay();
}

float ABallCharacter::GetBallCenterZ() const
{
	// Capsule center sits halfway between the ground and the top of the ball
	return FeetGap + GetBallHalfHeight() - (GetBallHalfHeight() * 2.f + FeetGap) * 0.5f;
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
				UpdateCorpseCollision(true);
			}
		}
		return;
	}

	if (MountedHorse)
	{
		// The horse does the running; we sit in the saddle, and a hard gallop slowly tires us too
		UpdateSeat();
		if (MountedHorse->IsGalloping())
		{
			Stamina->Drain(RidingStaminaCost, DeltaTime);
		}
		bRunning = false;
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

bool ABallCharacter::Mount(AHorse* Horse)
{
	if (!Horse || !Horse->CanBeMounted() || MountedHorse || IsDead())
	{
		return false;
	}

	MountedHorse = Horse;
	Horse->SetRider(this);

	// Our own legs stop moving us; we ride along attached to the horse
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->StopMovementImmediately();
	Movement->DisableMovement();
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetCapsuleComponent()->IgnoreActorWhenMoving(Horse, true);

	AttachToActor(Horse, FAttachmentTransformRules::KeepWorldTransform);
	UpdateSeat();
	Animator->SetRiding(true, Horse->GetBodyHalfWidth());
	return true;
}

void ABallCharacter::Dismount()
{
	AHorse* Horse = MountedHorse;
	if (!Horse)
	{
		return;
	}
	MountedHorse = nullptr;
	Horse->SetRider(nullptr);

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Animator->SetRiding(false);
	GetCapsuleComponent()->IgnoreActorWhenMoving(Horse, false);
	if (!IsDead())
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}

	// Land beside the horse: the left side like a real rider, the right side if that's blocked
	const float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float HorseHalfHeight = Horse->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float SideDistance = Horse->GetBodyHalfWidth() + GetCapsuleComponent()->GetScaledCapsuleRadius() + 25.f;
	const FRotator Facing(0.f, Horse->GetActorRotation().Yaw, 0.f);
	for (const float Side : { -1.f, 1.f })
	{
		FVector Spot = Horse->GetActorLocation() + Horse->GetActorRightVector() * Side * SideDistance;
		Spot.Z = Horse->GetActorLocation().Z - HorseHalfHeight + HalfHeight + 5.f;
		if (TeleportTo(Spot, Facing))
		{
			break;
		}
	}

	if (!IsDead())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	}
}

void ABallCharacter::UpdateSeat()
{
	// The bottom of the ball rests on the saddle
	const float BallBottomZ = GetBallCenterZ() - GetBallHalfHeight();
	SetActorRelativeLocation(MountedHorse->GetSaddleOffset() - FVector(0.f, 0.f, BallBottomZ));
	SetActorRelativeRotation(FRotator::ZeroRotator);
}

void ABallCharacter::UpdateCorpseCollision(bool bBones)
{
	const float GroundZ = -GetGroundOffset();
	if (!bBones)
	{
		// The ball lies on its back: about as long as it is tall, as wide and high as it is wide
		const FVector Extent(GetBallHalfHeight() * 0.9f, BallRadius * 0.95f, BallRadius * 0.95f);
		CorpseCollision->SetBoxExtent(Extent);
		CorpseCollision->SetRelativeLocation(FVector(0.f, 0.f, GroundZ + Extent.Z));
		CorpseCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SkullCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return;
	}

	// Bones: a low box over the ribs and pelvis (easy to step onto) and a round skull
	const FVector Extent(40.f, 34.f, 12.f);
	CorpseCollision->SetBoxExtent(Extent);
	CorpseCollision->SetRelativeLocation(FVector(30.f, 0.f, GroundZ + Extent.Z));
	SkullCollision->SetSphereRadius(Skeleton->GetSkullSize() * 0.5f);
	SkullCollision->SetRelativeLocation(FVector(-30.f, 0.f, GroundZ + Skeleton->GetSkullSize() * 0.5f));
	SkullCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}
