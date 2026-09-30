// Crowns & Commoners

#include "Characters/BallMeleeComponent.h"
#include "Characters/BallCharacter.h"
#include "Characters/HealthComponent.h"
#include "Characters/StaminaComponent.h"
#include "Items/CIRLInventoryComponent.h"
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
	// What's in the main hand decides the strike. Bows and crossbows aren't swung: with one in hand it's still a fist
	FStrike NewStrike;
	const UCIRLInventoryComponent* Things = Ball->GetInventory();
	const FCIRLItemRow* Weapon = Things->FindItem(Things->GetEquipped(ECIRLEquipSlot::WeaponMain));
	float Cost = StaminaCost;
	if (Weapon && Weapon->IsWeapon() && !Weapon->IsRanged())
	{
		NewStrike.bWeapon = true;
		NewStrike.Damage = Weapon->Damage;
		NewStrike.DamageType = static_cast<uint8>(Weapon->DamageType);
		NewStrike.Reach = Reach + Weapon->ReachCm * WeaponReachShare;
		NewStrike.TimeScale = 1.f + Weapon->WeightKg * SlowerPerKg;
		NewStrike.Name = Weapon->Name;
		Cost += Weapon->WeightKg * StaminaPerKg;
	}
	else
	{
		NewStrike.DamageType = static_cast<uint8>(ECIRLDamageType::Blunt);
		NewStrike.Reach = Reach;
	}
	if (!Ball->GetStamina()->TryConsume(Cost * Ball->GetLoadStaminaScale()))
	{
		return false;
	}
	Strike = NewStrike;

	// Fists alternate; a weapon is always in the right hand
	PunchHand = Strike.bWeapon ? 1 : 1 - PunchHand;
	PunchTime = 0.f;
	bHitResolved = false;
	CooldownLeft = Cooldown * Strike.TimeScale;

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
		const float Forward = (PunchTime - WindUpTime) / FMath::Max(ImpactTime - WindUpTime, KINDA_SMALL_NUMBER);
		return FMath::Lerp(-WindUpPull, 1.f, FMath::InterpEaseIn(0.f, 1.f, Forward, 2.f));
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

	// A heavier weapon goes through the same motion more slowly
	PunchTime += DeltaTime / Strike.TimeScale;
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

	if (!Ball->GetStamina()->TryConsume(BlockStaminaCost))
	{
		return Damage;
	}
	// Bare fists stop some of it; a shield or buckler in the off hand (or gauntlets) stops more
	static const ECIRLEquipSlot GuardSlots[] = { ECIRLEquipSlot::WeaponOff, ECIRLEquipSlot::Gloves };
	const float Shield = Ball->GetInventory()->GetArmour(GuardSlots, ECIRLDamageType::Blunt) * ShieldBlockScale;
	return Damage * BlockDamageMultiplier * 60.f / (60.f + Shield);
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
	const float StrikeReach = Strike.Reach;
	const FVector End = Start + Direction * StrikeReach;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(BallPunch), false, Ball);

	// Walls and objects stop the fist before anyone behind them
	float BlockedAt = StrikeReach;
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
		// A weapon's damage varies a little from blow to blow; a fist picks from its range
		const float Base = Strike.bWeapon ? Strike.Damage * FMath::FRandRange(0.85f, 1.15f) : FMath::FRandRange(PunchDamage.X, PunchDamage.Y);
		const float Damage = FMath::RoundToFloat(Base * MultiplierFor(Zone));
		float ArmourStopped = 0.f;
		const float Dealt = Target->TakeStrike(Damage, static_cast<ECIRLDamageType>(Strike.DamageType), Zone, Hit, Direction, Ball->GetController(), Ball, &ArmourStopped);
		UE_LOG(LogTemp, Verbose, TEXT("Strike: hit %s zone %d for %.0f (dealt %.0f)"), *Target->GetName(), static_cast<int32>(Zone), Damage, Dealt);

		if (!Target->IsDead())
		{
			// A heavy blow shoves harder than a jab
			const float Shove = Knockback * (Strike.bWeapon ? FMath::Clamp(Strike.Damage / 12.f, 1.f, 2.2f) : 1.f);
			Target->LaunchCharacter(Direction.GetSafeNormal2D() * Shove + FVector(0.f, 0.f, 80.f), true, false);
		}

#if !UE_BUILD_SHIPPING
		if (Ball->IsPlayerControlled() && GEngine)
		{
			const TCHAR* ZoneName = Zone == EBallHitZone::Head ? TEXT("Head") : (Zone == EBallHitZone::Chest ? TEXT("Chest") : TEXT("Lower"));
			GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Orange, FString::Printf(TEXT("%s, %s hit: %.0f damage, armour stopped %.0f%%, %.0f got through (target HP %.0f)"),
				Strike.bWeapon ? *Strike.Name.ToString() : TEXT("Fist"), ZoneName, Damage, ArmourStopped * 100.f, Dealt, Target->GetHealth()->GetHealth()));
		}
#endif
		break;
	}
}
