// Crowns & Commoners

#include "Characters/AI/BallFighterController.h"
#include "Characters/BallCharacter.h"
#include "Characters/BallMeleeComponent.h"
#include "Components/CapsuleComponent.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"

ABallFighterController::ABallFighterController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABallFighterController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	HomeLocation = InPawn->GetActorLocation();
	HomeRotation = InPawn->GetActorRotation();
	InPawn->OnTakeAnyDamage.AddUniqueDynamic(this, &ABallFighterController::HandleDamaged);
	// Not every ball looks on the same frame
	LookTimer = FMath::FRandRange(0.f, 0.3f);
}

bool ABallFighterController::IsEnemy(const ABallCharacter* Other) const
{
	return Other && Other->IsPlayerControlled();
}

void ABallFighterController::HandleDamaged(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	// Struck by someone it hadn't seen: it turns on them
	ABallCharacter* Ball = Cast<ABallCharacter>(GetPawn());
	ABallCharacter* Attacker = Cast<ABallCharacter>(DamageCauser);
	if (Ball && !Ball->IsDead() && !Target && Attacker && Attacker != Ball && !Attacker->IsDead() && IsEnemy(Attacker))
	{
		StartFight(Ball, Attacker);
	}
}

void ABallFighterController::NoticeKilling(const ABallCharacter* Victim, ABallCharacter* Killer)
{
	ABallCharacter* Ball = Cast<ABallCharacter>(GetPawn());
	if (!Ball || !Victim || !Killer || Ball == Victim || Ball == Killer || Ball->IsDead() || Killer->IsDead() || Target || !IsEnemy(Killer))
	{
		return;
	}
	if (FVector::Dist2D(Ball->GetActorLocation(), Victim->GetActorLocation()) <= KillingNoticeRange)
	{
		StartFight(Ball, Killer);
	}
}

ABallCharacter* ABallFighterController::FindEnemy(const ABallCharacter* Ball) const
{
	ABallCharacter* Best = nullptr;
	float BestDistance = TNumericLimits<float>::Max();
	for (TActorIterator<ABallCharacter> It(GetWorld()); It; ++It)
	{
		ABallCharacter* Other = *It;
		if (Other == Ball || Other->IsDead() || !IsEnemy(Other))
		{
			continue;
		}
		const FVector To = Other->GetActorLocation() - Ball->GetActorLocation();
		const float Distance = To.Size2D();
		// Crouched and quiet, they must come much closer to be heard, and are a little harder to spot.
		// Running, they are heard and seen from further away, in front and to the sides alike
		const bool bSneaking = Other->IsSneaking();
		const bool bRunning = Other->IsRunning();
		const float SeenFrom = SightRange * (bSneaking ? SneakSightScale : (bRunning ? RunSightScale : 1.f));
		const float HeardFrom = NoticeRange * (bSneaking ? SneakNoticeScale : (bRunning ? RunNoticeScale : 1.f));
		if (Distance >= BestDistance || Distance >= SeenFrom)
		{
			continue;
		}
		const bool bInFront = FVector::DotProduct(Ball->GetActorForwardVector(), To.GetSafeNormal2D()) >= SightCosine;
		if ((bInFront || Distance < HeardFrom) && LineOfSightTo(Other))
		{
			Best = Other;
			BestDistance = Distance;
		}
	}
	return Best;
}

ABallCharacter* ABallFighterController::FindFightToJoin(const ABallCharacter* Ball) const
{
	for (TActorIterator<ABallFighterController> It(GetWorld()); It; ++It)
	{
		const ABallFighterController* Other = *It;
		const APawn* Fighter = Other->GetPawn();
		ABallCharacter* Enemy = Other->GetTarget();
		if (Other == this || Other->GetState() != EBallFighterState::Fight || !Fighter || !Enemy || Enemy == Ball || Enemy->IsDead() || !IsEnemy(Enemy))
		{
			continue;
		}
		// The noise of it carries: no need to see it
		if (FVector::Dist2D(Fighter->GetActorLocation(), Ball->GetActorLocation()) <= JoinRange)
		{
			return Enemy;
		}
	}
	return nullptr;
}

void ABallFighterController::SetFacesEnemy(ABallCharacter* Ball, bool bFaceEnemy)
{
	UCharacterMovementComponent* Movement = Ball->GetCharacterMovement();
	Movement->bOrientRotationToMovement = !bFaceEnemy;
	Movement->bUseControllerDesiredRotation = bFaceEnemy;
}

void ABallFighterController::StartFight(ABallCharacter* Ball, ABallCharacter* Enemy)
{
	Target = Enemy;
	State = EBallFighterState::Fight;
	// Looking at the enemy also aims its blows at them
	SetFocus(Enemy);
	SetFacesEnemy(Ball, true);
	Ball->SetEmotion(EBallEmotion::Angry);
	StrikeTimer = FMath::FRandRange(0.f, StrikePause.X);
	GuardDelay = -1.f;
	GuardLeft = 0.f;
	CircleLeft = 0.f;
	bSawSwing = false;
}

void ABallFighterController::StopFight(ABallCharacter* Ball, bool bWon)
{
	Target = nullptr;
	State = EBallFighterState::Return;
	ClearFocus(EAIFocusPriority::Gameplay);
	SetFacesEnemy(Ball, false);
	Ball->GetMelee()->SetGuarding(false);
	Ball->SetSprinting(false);
	Ball->SetEmotion(bWon ? EBallEmotion::Happy : EBallEmotion::Suspicious);
	LookTimer = 1.f;
}

void ABallFighterController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ABallCharacter* Ball = Cast<ABallCharacter>(GetPawn());
	if (!Ball)
	{
		return;
	}
	if (Ball->IsDead())
	{
		if (Target)
		{
			Target = nullptr;
			ClearFocus(EAIFocusPriority::Gameplay);
			Ball->GetMelee()->SetGuarding(false);
		}
		return;
	}

	if (ShakeOffRider(Ball, DeltaTime))
	{
		return;
	}

	if (State == EBallFighterState::Fight)
	{
		TickFight(Ball, DeltaTime);
		return;
	}

	// At its post or on the way back: keeps an eye out (a few times a second is plenty)
	LookTimer -= DeltaTime;
	if (LookTimer <= 0.f)
	{
		LookTimer = 0.25f;
		ABallCharacter* Enemy = FindEnemy(Ball);
		if (!Enemy)
		{
			Enemy = FindFightToJoin(Ball);
		}
		if (Enemy)
		{
			StartFight(Ball, Enemy);
			return;
		}
	}
	if (State == EBallFighterState::Return)
	{
		TickReturn(Ball);
	}
}

bool ABallFighterController::ShakeOffRider(ABallCharacter* Ball, float DeltaTime)
{
	ABallCharacter* Rider = nullptr;
	for (TActorIterator<ABallCharacter> It(GetWorld()); It; ++It)
	{
		const UPrimitiveComponent* Base = It->GetMovementBase();
		if (*It != Ball && !It->IsDead() && Base && Base->GetOwner() == Ball)
		{
			Rider = *It;
			break;
		}
	}
	if (!Rider)
	{
		ShakeTimer = 0.f;
		return false;
	}

	// Nobody misses an enemy landing on their head
	if (!Target && IsEnemy(Rider))
	{
		StartFight(Ball, Rider);
	}
	Ball->GetMelee()->SetGuarding(false);
	Ball->SetSprinting(false);

	ShakeTimer -= DeltaTime;
	if (ShakeTimer <= 0.f)
	{
		ShakeTimer = 0.6f;
		const FVector Side = Ball->GetActorRightVector() * (FMath::RandBool() ? 1.f : -1.f);
		Rider->LaunchCharacter(Side * 320.f + FVector(0.f, 0.f, 260.f), true, true);
	}
	return true;
}

void ABallFighterController::TickReturn(ABallCharacter* Ball)
{
	const FVector ToHome = HomeLocation - Ball->GetActorLocation();
	if (ToHome.Size2D() > 60.f)
	{
		Ball->AddMovementInput(ToHome.GetSafeNormal2D(), 1.f);
		return;
	}
	// Back at its post, facing the way it did
	State = EBallFighterState::Idle;
	Ball->SetActorRotation(FRotator(0.f, HomeRotation.Yaw, 0.f));
	Ball->SetEmotion(Ball->GetStartingEmotion());
}

void ABallFighterController::TickFight(ABallCharacter* Ball, float DeltaTime)
{
	if (!Target || Target->IsDead())
	{
		StopFight(Ball, Target != nullptr);
		return;
	}

	const FVector To = Target->GetActorLocation() - Ball->GetActorLocation();
	const float Distance = To.Size2D();
	if (Distance > GiveUpRange || FVector::Dist2D(Ball->GetActorLocation(), HomeLocation) > LeashRange)
	{
		StopFight(Ball, false);
		return;
	}
	const FVector Toward = To.GetSafeNormal2D();
	UBallMeleeComponent* Melee = Ball->GetMelee();

	// A blow reaches from the eyes to the enemy's body: the furthest it can stand and still hit, and where it likes
	// to stand (well inside that, but not pressed against the enemy)
	const float MyRadius = Ball->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float TargetRadius = Target->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float StrikeDistance = Melee->GetReadyReach() + TargetRadius - 15.f;
	const float StandDistance = FMath::Max(StrikeDistance * 0.8f, MyRadius + TargetRadius + 20.f);

	// Guard: a swing coming its way is sometimes answered, after a moment to react
	const UBallMeleeComponent* TheirMelee = Target->GetMelee();
	if (!TheirMelee->IsPunching())
	{
		bSawSwing = false;
	}
	else if (!bSawSwing)
	{
		bSawSwing = true;
		const bool bAimedAtMe = FVector::DotProduct(Target->GetActorForwardVector(), -Toward) > 0.5f;
		if (bAimedAtMe && Distance < TheirMelee->GetReadyReach() + MyRadius + 60.f && !Melee->IsPunching() && FMath::FRand() < GuardChance)
		{
			GuardDelay = ReactionTime;
		}
	}
	if (GuardDelay >= 0.f)
	{
		GuardDelay -= DeltaTime;
		if (GuardDelay < 0.f)
		{
			GuardLeft = FMath::FRandRange(GuardTime.X, GuardTime.Y);
		}
	}
	GuardLeft = FMath::Max(GuardLeft - DeltaTime, 0.f);
	Melee->SetGuarding(GuardLeft > 0.f);

	// Movement: run in from afar, step back when crowded, otherwise circle
	Ball->SetSprinting(Distance > RunBeyond);
	if (Distance > StandDistance)
	{
		Ball->AddMovementInput(Toward, 1.f);
	}
	else if (Distance < StandDistance * 0.75f)
	{
		Ball->AddMovementInput(-Toward, 0.6f);
	}
	else
	{
		CircleLeft -= DeltaTime;
		if (CircleLeft <= 0.f)
		{
			CircleLeft = FMath::FRandRange(1.2f, 3.f);
			CircleSide = FMath::RandBool() ? 1.f : -1.f;
		}
		Ball->AddMovementInput(FVector::CrossProduct(FVector::UpVector, Toward) * CircleSide, CircleInput);
	}

	// Strike: in reach, facing the enemy, guard down, and its pause since the last blow over
	StrikeTimer -= DeltaTime;
	const bool bFacing = FVector::DotProduct(Ball->GetActorForwardVector(), Toward) > 0.85f;
	if (StrikeTimer <= 0.f && Distance <= StrikeDistance && bFacing && GuardLeft <= 0.f && GuardDelay < 0.f && Melee->IsReadyToStrike())
	{
		if (Melee->TryPunch())
		{
			StrikeTimer = FMath::FRandRange(StrikePause.X, StrikePause.Y);
		}
		else
		{
			// Out of breath: backs off its attack for a moment
			StrikeTimer = 0.5f;
		}
	}
}
