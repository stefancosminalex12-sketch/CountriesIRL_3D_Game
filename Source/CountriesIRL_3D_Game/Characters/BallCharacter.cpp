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
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequenceBase.h"
#include "Engine/SkeletalMesh.h"

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
	Animator->CreateLimbMeshes(this, VisualRoot, BodyPivot, Sphere, BallRadius, GetBallCenterZ(), -HalfHeight);

	Stamina = CreateDefaultSubobject<UStaminaComponent>(TEXT("Stamina"));
	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	Corpse = CreateDefaultSubobject<UCorpseComponent>(TEXT("Corpse"));
	Skeleton = CreateDefaultSubobject<UBallSkeletonComponent>(TEXT("Skeleton"));
	Melee = CreateDefaultSubobject<UBallMeleeComponent>(TEXT("Melee"));

	// Eyes are where the ball looks and punches from
	BaseEyeHeight = GetBallCenterZ() + 10.f;

	// Tick after animation and physics so a humanoid head follows this frame's pose, not last frame's
	PrimaryActorTick.TickGroup = TG_PostPhysics;

	// Humanoid prototype body: Unreal's free template mannequin and animations (already in the project)
	HumanoidMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple")));
	HumanoidAnimClass = TSoftClassPtr<UAnimInstance>(FSoftObjectPath(TEXT("/Game/Variant_Combat/Anims/ABP_Manny_Combat.ABP_Manny_Combat_C")));
	HumanoidPunchAnims.Add(TSoftObjectPtr<UAnimSequenceBase>(FSoftObjectPath(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_01.MM_Attack_01"))));
	HumanoidPunchAnims.Add(TSoftObjectPtr<UAnimSequenceBase>(FSoftObjectPath(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_02.MM_Attack_02"))));
}

void ABallCharacter::BeginPlay()
{
	Super::BeginPlay();

	RefreshColors();

	SetEmotion(StartingEmotion);
	SetSprinting(false);
	ApplyBodyStyle();

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

	// A humanoid body goes limp (ragdoll); the head ball rides along on the neck
	if (BodyStyle == EBallBodyStyle::Humanoid)
	{
		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->WakeAllRigidBodies();
	}

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
			if (BodyStyle == EBallBodyStyle::Countryball)
			{
				BodyPivot->SetRelativeScale3D(FVector(Corpse->GetBodyScale()));
			}

			// Decayed to bones: swap the ball for the cartoon skeleton (countryball body only for now)
			if (BodyStyle == EBallBodyStyle::Countryball && Corpse->IsSkeleton() && !Skeleton->IsShown())
			{
				Skeleton->Show(VisualRoot, -GetGroundOffset(), Corpse->GetTint());
				BodyMesh->SetVisibility(false);
				Face->SetFaceVisible(false);
				Animator->SetSkeletonPose(true);
			}
		}
		if (BodyStyle == EBallBodyStyle::Humanoid)
		{
			UpdateHumanoidHead(DeltaTime);
		}
		return;
	}

	if (BodyStyle == EBallBodyStyle::Humanoid)
	{
		UpdateHumanoidHead(DeltaTime);
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

void ABallCharacter::SetBodyStyle(EBallBodyStyle NewStyle)
{
	if (NewStyle != BodyStyle && !IsDead())
	{
		BodyStyle = NewStyle;
		ApplyBodyStyle();
	}
}

void ABallCharacter::ApplyBodyStyle()
{
	USkeletalMeshComponent* Body = GetMesh();
	const bool bHumanoid = BodyStyle == EBallBodyStyle::Humanoid;

	// Floating hands/feet and their procedural animation only belong to the countryball body
	Animator->SetLimbsVisible(!bHumanoid);
	Animator->SetComponentTickEnabled(!bHumanoid);

	if (bHumanoid)
	{
		USkeletalMesh* BodyAsset = HumanoidMesh.LoadSynchronous();
		UClass* AnimClass = HumanoidAnimClass.LoadSynchronous();
		if (!BodyAsset)
		{
			BodyStyle = EBallBodyStyle::Countryball;
			ApplyBodyStyle();
			return;
		}

		GetCapsuleComponent()->SetCapsuleSize(HumanoidCapsule.X, HumanoidCapsule.Y);
		Body->SetSkeletalMesh(BodyAsset);
		Body->SetAnimInstanceClass(AnimClass);
		Body->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -HumanoidCapsule.Y), FRotator(0.f, -90.f, 0.f));
		Body->SetRelativeScale3D(HumanoidBodyScale);
		Body->SetVisibility(true);
		// The mannequin's own head is replaced by the countryball
		Body->HideBoneByName(HumanoidHeadBone, EPhysBodyOp::PBO_None);

		// The head is not glued to the bone (animation jitters it); it follows the neck smoothly instead
		BodyPivot->AttachToComponent(VisualRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		bHeadPlaced = false;
		UpdateHumanoidHead(0.f);
	}
	else
	{
		const float HalfHeight = (BallRadius * 2.f + FeetGap) * 0.5f;
		GetCapsuleComponent()->SetCapsuleSize(BallRadius * 0.9f, HalfHeight);
		Body->SetVisibility(false);
		Body->SetAnimInstanceClass(nullptr);

		BodyPivot->AttachToComponent(VisualRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		BodyPivot->SetRelativeTransform(FTransform(FRotator::ZeroRotator, FVector(0.f, 0.f, GetBallCenterZ()), FVector::OneVector));
	}

	BaseEyeHeight = (GetHeadCenter() - GetActorLocation()).Z + 10.f;
}

void ABallCharacter::UpdateHumanoidHead(float DeltaTime)
{
	USkeletalMeshComponent* Body = GetMesh();
	const FVector Neck = Body->GetSocketLocation(HumanoidHeadBone);

	// Alive: the head sits straight up on the neck. Dead (ragdoll): it continues the line from the hips
	// through the neck, so it lies with the fallen body.
	FVector Up = FVector::UpVector;
	if (IsDead())
	{
		Up = (Neck - Body->GetSocketLocation(TEXT("pelvis"))).GetSafeNormal(UE_SMALL_NUMBER, FVector::UpVector);
	}

	const float HeadRadius = HumanoidHeadSize * 0.5f;
	const FTransform& Root = VisualRoot->GetComponentTransform();
	const FVector TargetLocal = Root.InverseTransformPosition(Neck + Up * (HeadRadius * HumanoidHeadLift));

	// Smooth in the character's own space: filters jitter without lagging behind when walking
	SmoothedHeadLocal = (bHeadPlaced && DeltaTime > 0.f) ? FMath::VInterpTo(SmoothedHeadLocal, TargetLocal, DeltaTime, HumanoidHeadSmoothing) : TargetLocal;
	bHeadPlaced = true;

	const FRotator Facing = IsDead() ? FRotator(70.f, 0.f, 12.f) : FRotator::ZeroRotator;
	BodyPivot->SetRelativeTransform(FTransform(Facing, SmoothedHeadLocal, FVector(HumanoidHeadSize / (BallRadius * 2.f))));
}

FVector ABallCharacter::GetHeadCenter() const
{
	return BodyPivot->GetComponentLocation();
}

float ABallCharacter::GetHeadRadius() const
{
	return BallRadius * BodyPivot->GetComponentScale().Z;
}

uint8 ABallCharacter::GetHitZoneAtHeight(float WorldZ) const
{
	// 0 = head/face, 1 = chest, 2 = lower body
	const float Height = (WorldZ - GetHeadCenter().Z) / FMath::Max(GetHeadRadius(), 1.f);
	if (BodyStyle == EBallBodyStyle::Countryball)
	{
		// The face (eyes) sits just above the ball's center, so everything from there up is the head
		return Height > 0.05f ? 0 : (Height < -0.45f ? 2 : 1);
	}

	// Humanoid: the ball is the head, the upper body is the chest, below the capsule's middle is lower
	if (Height > -0.9f)
	{
		return 0;
	}
	return WorldZ > GetActorLocation().Z ? 1 : 2;
}

void ABallCharacter::OnPunchStarted(int32 Hand)
{
	if (BodyStyle != EBallBodyStyle::Humanoid || HumanoidPunchAnims.Num() == 0)
	{
		return;
	}

	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	UAnimSequenceBase* Punch = HumanoidPunchAnims[NextPunchAnim % HumanoidPunchAnims.Num()].LoadSynchronous();
	NextPunchAnim++;
	if (Anim && Punch)
	{
		Anim->PlaySlotAnimationAsDynamicMontage(Punch, TEXT("DefaultSlot"), 0.05f, 0.15f, 1.4f);
	}
}
