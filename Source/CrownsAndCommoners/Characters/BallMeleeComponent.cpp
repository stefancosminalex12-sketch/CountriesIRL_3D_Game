// Crowns & Commoners

#include "Characters/BallMeleeComponent.h"
#include "Characters/BallCharacter.h"
#include "Characters/HealthComponent.h"
#include "Characters/StaminaComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UBallMeleeComponent::UBallMeleeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

bool UBallMeleeComponent::TryPunch()
{
	ABallCharacter* Ball = Cast<ABallCharacter>(GetOwner());
	if (!Ball || Ball->IsDead() || CooldownLeft > 0.f || IsPunching())
	{
		return false;
	}
	if (!Ball->GetStamina()->TryConsume(StaminaCost))
	{
		return false;
	}

	PunchHand = 1 - PunchHand;
	PunchTime = 0.f;
	bHitResolved = false;
	CooldownLeft = Cooldown;

	// Turn to face where we're punching (matters in third-person, where the ball faces its movement)
	Ball->SetActorRotation(FRotator(0.f, Ball->GetBaseAimRotation().Yaw, 0.f));
	return true;
}

float UBallMeleeComponent::GetPunchExtension(int32& OutHand) const
{
	OutHand = PunchHand;
	if (!IsPunching())
	{
		return 0.f;
	}

	// Wind-up: ease back
	if (PunchTime < WindUpTime)
	{
		return -WindUpPull * FMath::InterpEaseInOut(0.f, 1.f, PunchTime / WindUpTime, 2.f);
	}

	// Strike: accelerate into the impact
	if (PunchTime < ImpactTime)
	{
		const float Strike = (PunchTime - WindUpTime) / FMath::Max(ImpactTime - WindUpTime, KINDA_SMALL_NUMBER);
		return FMath::Lerp(-WindUpPull, 1.f, FMath::InterpEaseIn(0.f, 1.f, Strike, 2.f));
	}

	// Recoil: snap back, then settle
	const float Back = (PunchTime - ImpactTime) / FMath::Max(PunchDuration - ImpactTime, KINDA_SMALL_NUMBER);
	return 1.f - FMath::InterpEaseOut(0.f, 1.f, FMath::Clamp(Back, 0.f, 1.f), 2.f);
}

float UBallMeleeComponent::GetPunchEnvelope() const
{
	return IsPunching() ? FMath::Sin(FMath::Clamp(PunchTime / PunchDuration, 0.f, 1.f) * UE_PI) : 0.f;
}

void UBallMeleeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CooldownLeft = FMath::Max(CooldownLeft - DeltaTime, 0.f);

	if (IsGuarding())
	{
		CastChecked<ABallCharacter>(GetOwner())->GetStamina()->Drain(GuardStaminaPerSecond, DeltaTime);
	}

	if (!IsPunching())
	{
		return;
	}

	PunchTime += DeltaTime;
	if (!bHitResolved && PunchTime >= ImpactTime)
	{
		bHitResolved = true;
		ResolveHit();
	}
	if (PunchTime >= PunchDuration)
	{
		PunchTime = -1.f;
	}
}

bool UBallMeleeComponent::IsGuarding() const
{
	const ABallCharacter* Ball = Cast<ABallCharacter>(GetOwner());
	return bWantsGuard && Ball && !Ball->IsDead() && Ball->GetStamina()->HasStamina();
}

float UBallMeleeComponent::ModifyIncomingDamage(float Damage, const AActor* DamageCauser)
{
	const ABallCharacter* Ball = Cast<ABallCharacter>(GetOwner());
	if (!IsGuarding() || !DamageCauser || !Ball)
	{
		return Damage;
	}

	// Only hits from roughly in front can be blocked
	const FVector ToAttacker = (DamageCauser->GetActorLocation() - Ball->GetActorLocation()).GetSafeNormal2D();
	if (FVector::DotProduct(Ball->GetActorForwardVector(), ToAttacker) < BlockCosine)
	{
		return Damage;
	}

	return Ball->GetStamina()->TryConsume(BlockStaminaCost) ? Damage * BlockDamageMultiplier : Damage;
}

EBallHitZone UBallMeleeComponent::ZoneForPoint(const ABallCharacter* Target, const FVector& WorldPoint)
{
	// Height of the hit relative to the ball's center, in ball radii. The face (eyes) sits just above
	// the center, so everything from there up counts as the head.
	const float CenterZ = Target->GetActorLocation().Z + Target->GetBallCenterZ();
	const float Height = (WorldPoint.Z - CenterZ) / FMath::Max(Target->GetBallHalfHeight(), 1.f);
	if (Height > 0.05f)
	{
		return EBallHitZone::Head;
	}
	return Height < -0.45f ? EBallHitZone::Lower : EBallHitZone::Chest;
}

float UBallMeleeComponent::MultiplierFor(EBallHitZone Zone) const
{
	switch (Zone)
	{
	case EBallHitZone::Head:  return HeadMultiplier;
	case EBallHitZone::Lower: return LowerMultiplier;
	default:                  return ChestMultiplier;
	}
}

void UBallMeleeComponent::ResolveHit()
{
	ABallCharacter* Ball = Cast<ABallCharacter>(GetOwner());
	if (!Ball)
	{
		return;
	}

	const FVector Start = Ball->GetPawnViewLocation();
	const FVector Direction = Ball->GetBaseAimRotation().Vector();
	const FVector End = Start + Direction * Reach;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(BallPunch), false, Ball);

	// Walls and objects stop the fist before anyone behind them
	float BlockedAt = Reach;
	FHitResult Wall;
	const FCollisionObjectQueryParams WorldObjects(ECC_TO_BITFIELD(ECC_WorldStatic) | ECC_TO_BITFIELD(ECC_WorldDynamic));
	if (GetWorld()->LineTraceSingleByObjectType(Wall, Start, End, WorldObjects, Params))
	{
		BlockedAt = Wall.Distance;
	}

	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByObjectType(Hits, Start, End, FQuat::Identity, FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeSphere(HitRadius), Params);

	for (const FHitResult& Hit : Hits)
	{
		ABallCharacter* Target = Cast<ABallCharacter>(Hit.GetActor());
		if (!Target || Target == Ball || Target->IsDead())
		{
			continue;
		}
		if (Hit.Distance > BlockedAt)
		{
			break;
		}

		const EBallHitZone Zone = ZoneForPoint(Target, Hit.ImpactPoint);
		const float Damage = FMath::RoundToFloat(FMath::FRandRange(PunchDamage.X, PunchDamage.Y) * MultiplierFor(Zone));
		const FPointDamageEvent DamageEvent(Damage, Hit, Direction, nullptr);
		const float Dealt = Target->TakeDamage(Damage, DamageEvent, Ball->GetController(), Ball);
		UE_LOG(LogTemp, Verbose, TEXT("Punch: hit %s zone %d for %.0f (dealt %.0f)"), *Target->GetName(), static_cast<int32>(Zone), Damage, Dealt);

		if (!Target->IsDead())
		{
			Target->LaunchCharacter(Direction.GetSafeNormal2D() * Knockback + FVector(0.f, 0.f, 80.f), true, false);
		}

#if !UE_BUILD_SHIPPING
		if (Ball->IsPlayerControlled() && GEngine)
		{
			const TCHAR* ZoneName = Zone == EBallHitZone::Head ? TEXT("Head") : (Zone == EBallHitZone::Chest ? TEXT("Chest") : TEXT("Lower"));
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, FString::Printf(TEXT("%s hit: %.0f damage (target HP %.0f)"),
				ZoneName, Damage, Target->GetHealth()->GetHealth()));
		}
#endif
		break;
	}
}
