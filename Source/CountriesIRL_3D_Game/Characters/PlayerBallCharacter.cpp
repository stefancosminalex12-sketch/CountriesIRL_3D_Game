// CountriesIRL 3D Game

#include "Characters/PlayerBallCharacter.h"
#include "Characters/BallAnimatorComponent.h"
#include "Characters/HealthComponent.h"
#include "Characters/BallMeleeComponent.h"
#include "Engine/DamageEvents.h"
#include "Core/CIRLInputConfig.h"
#include "Core/CIRLPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"

APlayerBallCharacter::APlayerBallCharacter()
{
	// First-person camera sits just behind the eyes, inside the (hidden) ball
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraOffset = FVector(BallRadius * 0.55f, 0.f, GetBallCenterZ() + 10.f);
	FirstPersonCamera->SetRelativeLocation(FirstPersonCameraOffset);
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->SetFieldOfView(90.f);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetCapsuleComponent());
	CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, GetBallCenterZ()));
	CameraBoom->TargetArmLength = 380.f;
	CameraBoom->TargetOffset = FVector(0.f, 0.f, 50.f);
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 12.f;

	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	ThirdPersonCamera->bUsePawnControlRotation = false;
}

void APlayerBallCharacter::BeginPlay()
{
	Super::BeginPlay();
	SetFirstPerson(bStartInFirstPerson);
}

void APlayerBallCharacter::SetFirstPerson(bool bEnable)
{
	bFirstPerson = bEnable;

	FirstPersonCamera->SetActive(bEnable);
	ThirdPersonCamera->SetActive(!bEnable);

	UpdateRotationMode();

	// Don't draw the ball from the inside; hands and feet stay visible
	BodyMesh->SetOwnerNoSee(bEnable);
	Face->SetOwnerNoSee(bEnable);
	Animator->SetFirstPersonHands(bEnable);
}

void APlayerBallCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	const ACIRLPlayerController* PC = Cast<ACIRLPlayerController>(GetController());
	const UCIRLInputConfig* Input = PC ? PC->GetInputConfig() : nullptr;
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input || !EIC)
	{
		return;
	}

	EIC->BindAction(Input->Move, ETriggerEvent::Triggered, this, &APlayerBallCharacter::Move);
	EIC->BindAction(Input->Look, ETriggerEvent::Triggered, this, &APlayerBallCharacter::Look);
	EIC->BindAction(Input->Jump, ETriggerEvent::Started, this, &ACharacter::Jump);
	EIC->BindAction(Input->Jump, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	EIC->BindAction(Input->Sprint, ETriggerEvent::Started, this, &APlayerBallCharacter::StartSprint);
	EIC->BindAction(Input->Sprint, ETriggerEvent::Completed, this, &APlayerBallCharacter::StopSprint);
	EIC->BindAction(Input->ToggleView, ETriggerEvent::Started, this, &APlayerBallCharacter::ToggleView);
	EIC->BindAction(Input->CycleEmotion, ETriggerEvent::Started, this, &APlayerBallCharacter::CycleEmotion);
	EIC->BindAction(Input->Attack, ETriggerEvent::Started, this, &APlayerBallCharacter::Attack);
	EIC->BindAction(Input->Guard, ETriggerEvent::Started, this, &APlayerBallCharacter::StartGuard);
	EIC->BindAction(Input->Guard, ETriggerEvent::Completed, this, &APlayerBallCharacter::StopGuard);
}

void APlayerBallCharacter::Move(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();
	if (!Controller)
	{
		return;
	}

	// Side-steps are slower. Only in first-person: in third-person the ball turns to face where it walks.
	if (bFirstPerson)
	{
		Input.X *= StrafeSpeedScale;
	}

	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FRotationMatrix Matrix(YawRotation);
	AddMovementInput(Matrix.GetUnitAxis(EAxis::X), Input.Y);
	AddMovementInput(Matrix.GetUnitAxis(EAxis::Y), Input.X);
}

void APlayerBallCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	AddControllerYawInput(Input.X);
	AddControllerPitchInput(Input.Y);
}

void APlayerBallCharacter::DevWalk(float Forward, float Right, float Seconds, bool bRun)
{
	DevMoveInput = FVector2D(Right, Forward);
	DevMoveTimeLeft = Seconds;
	SetSprinting(bRun);
}

void APlayerBallCharacter::DevDamage(float Amount)
{
	TakeDamage(Amount, FDamageEvent(), GetController(), this);
}

void APlayerBallCharacter::DevHeal(float Amount)
{
	Health->Heal(Amount);
}

void APlayerBallCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// First-person: the view nudges forward with each punch so it lands with some weight
	int32 PunchHand = 0;
	const float PunchDrive = Melee->IsPunching() ? FMath::Max(Melee->GetPunchExtension(PunchHand), 0.f) : 0.f;
	FirstPersonCamera->SetRelativeLocation(FirstPersonCameraOffset + FVector(PunchCameraNudge * PunchDrive, 0.f, -0.3f * PunchCameraNudge * PunchDrive));

	if (DevMoveTimeLeft > 0.f)
	{
		Move(FInputActionValue(DevMoveInput));
		DevMoveTimeLeft -= DeltaTime;
		if (DevMoveTimeLeft <= 0.f)
		{
			SetSprinting(false);
		}
	}
}

void APlayerBallCharacter::Attack()
{
	Melee->TryPunch();
}

void APlayerBallCharacter::StartGuard()
{
	Melee->SetGuarding(true);
	UpdateRotationMode();
}

void APlayerBallCharacter::StopGuard()
{
	Melee->SetGuarding(false);
	UpdateRotationMode();
}

void APlayerBallCharacter::DevGuard()
{
	if (Melee->WantsGuard())
	{
		StopGuard();
	}
	else
	{
		StartGuard();
	}
}

void APlayerBallCharacter::UpdateRotationMode()
{
	// Face the view in first-person, and in third-person while guarding (fighting stance);
	// otherwise turn toward where we walk
	const bool bFaceView = bFirstPerson || Melee->WantsGuard();
	bUseControllerRotationYaw = bFaceView;
	GetCharacterMovement()->bOrientRotationToMovement = !bFaceView;
}

void APlayerBallCharacter::CycleEmotion()
{
	const uint8 Next = (static_cast<uint8>(GetEmotion()) + 1) % static_cast<uint8>(EBallEmotion::Count);
	SetEmotion(static_cast<EBallEmotion>(Next));
}

void APlayerBallCharacter::Emotion(const FString& Name)
{
	const int64 Value = StaticEnum<EBallEmotion>()->GetValueByNameString(Name);
	if (Value != INDEX_NONE && Value < static_cast<int64>(EBallEmotion::Count))
	{
		SetEmotion(static_cast<EBallEmotion>(Value));
	}
}
