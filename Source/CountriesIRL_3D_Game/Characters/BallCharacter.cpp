// CountriesIRL 3D Game

#include "Characters/BallCharacter.h"
#include "Characters/BallAnimatorComponent.h"
#include "Characters/BallParts.h"
#include "Characters/StaminaComponent.h"
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
}

void ABallCharacter::BeginPlay()
{
	Super::BeginPlay();

	BallParts::SetColor(BodyMesh, BodyColor);
	Animator->ApplyColors(HandColor, FootColor);

	SetEmotion(StartingEmotion);
	SetSprinting(false);
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

	// Only drain stamina while actually running on the ground, not while holding the key standing still
	const bool bMovingOnGround = GetVelocity().Size2D() > 10.f && GetCharacterMovement()->IsMovingOnGround();
	bRunning = bWantsToRun && bMovingOnGround && Stamina->HasStamina();
	if (bRunning)
	{
		Stamina->Drain(RunStaminaCost, DeltaTime);
	}

	GetCharacterMovement()->MaxWalkSpeed = bRunning ? RunSpeed : WalkSpeed;
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
