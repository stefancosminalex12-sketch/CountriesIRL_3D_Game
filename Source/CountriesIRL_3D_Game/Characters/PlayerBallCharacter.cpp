// CountriesIRL 3D Game

#include "Characters/PlayerBallCharacter.h"
#include "Characters/BallAnimatorComponent.h"
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
	FirstPersonCamera->SetRelativeLocation(FVector(BallRadius * 0.55f, 0.f, GetBallCenterZ() + 10.f));
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

	// In first-person the body turns with the view; in third-person it turns toward where it walks
	bUseControllerRotationYaw = bEnable;
	GetCharacterMovement()->bOrientRotationToMovement = !bEnable;

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
}

void APlayerBallCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	if (!Controller)
	{
		return;
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
