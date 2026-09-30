// Crowns & Commoners

#include "Characters/PlayerBallCharacter.h"
#include "Characters/BallAnimatorComponent.h"
#include "Characters/HealthComponent.h"
#include "Characters/BallMeleeComponent.h"
#include "Animals/Horse.h"
#include "EngineUtils.h"
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
	CameraBoom->TargetArmLength = CameraDistance.X;
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
	// ...but the hidden ball still casts its shadow, so your shadow is whole in first-person too
	BodyMesh->bCastHiddenShadow = true;
	BodyMesh->MarkRenderStateDirty();
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
	EIC->BindAction(Input->Move, ETriggerEvent::Completed, this, &APlayerBallCharacter::StopMove);
	EIC->BindAction(Input->Jump, ETriggerEvent::Started, this, &APlayerBallCharacter::JumpPressed);
	EIC->BindAction(Input->Jump, ETriggerEvent::Completed, this, &APlayerBallCharacter::JumpReleased);
	EIC->BindAction(Input->Interact, ETriggerEvent::Started, this, &APlayerBallCharacter::Interact);
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

	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FRotationMatrix Matrix(YawRotation);

	// In the saddle the keys steer the horse, relative to where we look; Shift picks the gait
	if (AHorse* Horse = GetMount())
	{
		const FVector Direction = Matrix.GetUnitAxis(EAxis::X) * Input.Y + Matrix.GetUnitAxis(EAxis::Y) * Input.X;
		Horse->SetRiderInput(Direction, GetRideGait());
		return;
	}

	// Side-steps are slower. Only in first-person: in third-person the ball turns to face where it walks.
	if (bFirstPerson)
	{
		Input.X *= StrafeSpeedScale;
	}

	AddMovementInput(Matrix.GetUnitAxis(EAxis::X), Input.Y);
	AddMovementInput(Matrix.GetUnitAxis(EAxis::Y), Input.X);
}

void APlayerBallCharacter::StopMove(const FInputActionValue& Value)
{
	if (AHorse* Horse = GetMount())
	{
		// Reined in: the next start is at a walk again
		bRideAtTrot = false;
		Horse->SetRiderInput(FVector::ZeroVector, EHorseGait::Walk);
	}
}

void APlayerBallCharacter::JumpPressed()
{
	if (AHorse* Horse = GetMount())
	{
		Horse->RiderJump();
		return;
	}
	Jump();
}

void APlayerBallCharacter::JumpReleased()
{
	StopJumping();
}

void APlayerBallCharacter::Interact()
{
	if (IsMounted())
	{
		Dismount();
	}
	else if (AHorse* Horse = FindHorseToMount())
	{
		bRideAtTrot = false;
		Mount(Horse);
	}
	UpdateRotationMode();
}

void APlayerBallCharacter::DevRide()
{
	AHorse* Best = nullptr;
	float BestDistance = TNumericLimits<float>::Max();
	for (TActorIterator<AHorse> It(GetWorld()); It; ++It)
	{
		const float Distance = FVector::Dist(It->GetActorLocation(), GetActorLocation());
		if (It->CanBeMounted() && Distance < BestDistance)
		{
			Best = *It;
			BestDistance = Distance;
		}
	}
	if (Best && !IsMounted())
	{
		bRideAtTrot = false;
		Mount(Best);
		UpdateRotationMode();
	}
}

void APlayerBallCharacter::StartSprint()
{
	// On foot Shift is simply held to run
	SetSprinting(true);

	// In the saddle, a press right after a tap is a double press (gallop while held); so is every
	// press in a quick burst (tap, tap, hold), and a quick re-press after letting go of a canter.
	// It takes back the first tap's walk/trot switch, so double-pressing never changes the travelling gait
	const float Now = GetWorld()->GetRealTimeSeconds();
	bShiftDoublePress = IsMounted() && Now - LastShiftTapTime <= ShiftDoublePressSeconds;
	if (bShiftDoublePress)
	{
		bRideAtTrot = bTrotBeforeTap;
	}
	ShiftPressTime = Now;
}

void APlayerBallCharacter::StopSprint()
{
	const EHorseGait GaitBefore = GetRideGait();
	SetSprinting(false);
	const float Now = GetWorld()->GetRealTimeSeconds();
	if (IsMounted())
	{
		// A quick tap switches between walk and trot (only the first tap of a burst does).
		// Letting go of a long hold changes nothing, but a quick press after it still counts as a double press
		if (Now - ShiftPressTime <= ShiftTapSeconds && !bShiftDoublePress)
		{
			bTrotBeforeTap = bRideAtTrot;
			bRideAtTrot = !bRideAtTrot;
		}
		else if (!bShiftDoublePress)
		{
			bTrotBeforeTap = bRideAtTrot;
		}
		LastShiftTapTime = Now;

		if (GaitBefore == EHorseGait::Canter || GaitBefore == EHorseGait::Gallop)
		{
			HeldGait = GaitBefore;
			HeldGaitUntil = Now + ShiftDoublePressSeconds;
		}
	}
	bShiftDoublePress = false;
}

EHorseGait APlayerBallCharacter::GetRideGait() const
{
	// Double press and hold gallops; press and hold canters (once it's clearly not a tap)
	if (WantsToRun())
	{
		if (bShiftDoublePress)
		{
			return EHorseGait::Gallop;
		}
		if (GetWorld()->GetRealTimeSeconds() - ShiftPressTime > ShiftTapSeconds)
		{
			return EHorseGait::Canter;
		}
	}
	// Just let go of a canter or gallop (or pressed again and it isn't a hold yet): keep the pace a moment
	if (GetWorld()->GetRealTimeSeconds() < HeldGaitUntil)
	{
		return HeldGait;
	}
	return bRideAtTrot ? EHorseGait::Trot : EHorseGait::Walk;
}

AHorse* APlayerBallCharacter::FindHorseToMount() const
{
	if (IsDead())
	{
		return nullptr;
	}
	AHorse* Best = nullptr;
	float BestDistance = MountRange;
	for (TActorIterator<AHorse> It(GetWorld()); It; ++It)
	{
		const float Distance = FVector::Dist2D(It->GetActorLocation(), GetActorLocation());
		if (It->CanBeMounted() && Distance < BestDistance)
		{
			Best = *It;
			BestDistance = Distance;
		}
	}
	return Best;
}

FString APlayerBallCharacter::GetInteractPrompt() const
{
	if (IsMounted())
	{
		return TEXT("E  Get off");
	}
	return FindHorseToMount() ? TEXT("E  Get on the horse") : FString();
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

	// Third-person: pull the camera back in the saddle so the whole horse is in view
	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, IsMounted() ? CameraDistance.Y : CameraDistance.X, DeltaTime, 3.f);

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
	// In the saddle we face where the horse faces (the camera still looks around freely)
	if (IsMounted())
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		return;
	}

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
