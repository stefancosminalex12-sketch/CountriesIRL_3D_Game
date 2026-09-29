// CountriesIRL 3D Game

#include "Characters/CorpseComponent.h"
#include "Characters/BallParts.h"
#include "World/WorldClockSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

namespace
{
	/** How often the decay stage is re-evaluated (real seconds); decay is slow, no need every frame */
	constexpr float StageUpdateInterval = 0.5f;

	FCorpseStage MakeStage(const TCHAR* Name, float Hours, const FLinearColor& Tint, float Strength, float Scale, bool bFlies, bool bScavengers, bool bSkeleton = false)
	{
		FCorpseStage Stage;
		Stage.Name = Name;
		Stage.HoursAfterDeath = Hours;
		Stage.Tint = Tint;
		Stage.TintStrength = Strength;
		Stage.BodyScale = Scale;
		Stage.bFlies = bFlies;
		Stage.bScavengers = bScavengers;
		Stage.bSkeleton = bSkeleton;
		return Stage;
	}
}

UCorpseComponent::UCorpseComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	FlyMesh = BallParts::LoadSphere();

	// In-game hours (1 day = 48 real minutes): pale after ~12 real min, flies ~24 min, decomposing after a day,
	// rotting after two, bones after three
	Stages.Add(MakeStage(TEXT("Fresh"),        0.f,  FLinearColor::White,               0.f,   1.f,   false, false));
	Stages.Add(MakeStage(TEXT("Pale"),         6.f,  FLinearColor(0.5f, 0.53f, 0.56f),  0.5f,  1.f,   false, false));
	Stages.Add(MakeStage(TEXT("Flies"),        12.f, FLinearColor(0.42f, 0.45f, 0.37f), 0.6f,  1.02f, true,  false));
	Stages.Add(MakeStage(TEXT("Decomposing"),  24.f, FLinearColor(0.24f, 0.3f, 0.14f),  0.75f, 1.07f, true,  true));
	Stages.Add(MakeStage(TEXT("Rotting"),      48.f, FLinearColor(0.14f, 0.12f, 0.08f), 0.85f, 1.f,   true,  true));
	Stages.Add(MakeStage(TEXT("Bones"),        72.f, FLinearColor(0.8f, 0.77f, 0.66f),  1.f,   0.9f,  false, false, true));
}

void UCorpseComponent::StartDecay()
{
	const UWorldClockSubsystem* Clock = GetWorld()->GetSubsystem<UWorldClockSubsystem>();
	if (!Clock || bDecaying)
	{
		return;
	}

	DeathTime = Clock->GetDateTime();
	bDecaying = true;
	SetComponentTickEnabled(true);
	UpdateStage();
}

float UCorpseComponent::GetHoursDead() const
{
	const UWorldClockSubsystem* Clock = GetWorld()->GetSubsystem<UWorldClockSubsystem>();
	return (Clock && bDecaying) ? static_cast<float>((Clock->GetDateTime() - DeathTime).GetTotalHours()) : 0.f;
}

void UCorpseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceStageUpdate += DeltaTime;
	if (TimeSinceStageUpdate >= StageUpdateInterval)
	{
		TimeSinceStageUpdate = 0.f;
		UpdateStage();
	}

	if (bFliesActive)
	{
		AnimateFlies(DeltaTime);
	}
}

void UCorpseComponent::UpdateStage()
{
	if (Stages.Num() == 0)
	{
		return;
	}

	const float Hours = GetHoursDead();
	if (Hours >= RemoveAfterHours)
	{
		GetOwner()->Destroy();
		return;
	}

	// Blend from the last reached stage toward the next one, so colors change gradually
	int32 Next = 0;
	while (Next < Stages.Num() && Stages[Next].HoursAfterDeath <= Hours)
	{
		++Next;
	}
	const int32 Reached = FMath::Max(Next - 1, 0);
	const FCorpseStage& From = Stages[Reached];
	const FCorpseStage& To = Stages[FMath::Min(Next, Stages.Num() - 1)];
	const float Span = To.HoursAfterDeath - From.HoursAfterDeath;
	const float Alpha = Span > 0.f ? FMath::Clamp((Hours - From.HoursAfterDeath) / Span, 0.f, 1.f) : 1.f;

	CurrentTint = FMath::Lerp(From.Tint, To.Tint, Alpha);
	CurrentTintStrength = FMath::Lerp(From.TintStrength, To.TintStrength, Alpha);
	CurrentBodyScale = FMath::Lerp(From.BodyScale, To.BodyScale, Alpha);
	bSkeleton = From.bSkeleton;
	bScavengers = From.bScavengers;
	SetFliesActive(From.bFlies);
}

void UCorpseComponent::SetFliesActive(bool bActive)
{
	if (bActive == bFliesActive)
	{
		return;
	}
	bFliesActive = bActive;

	if (bActive && Flies.Num() == 0)
	{
		// Created only when needed: most balls never become corpses
		for (int32 Index = 0; Index < FlyCount; ++Index)
		{
			UStaticMeshComponent* Fly = NewObject<UStaticMeshComponent>(GetOwner());
			Fly->SetStaticMesh(FlyMesh);
			Fly->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Fly->SetCastShadow(false);
			Fly->SetRelativeScale3D(FVector(FlySize / 100.f));
			Fly->SetupAttachment(GetOwner()->GetRootComponent());
			Fly->RegisterComponent();
			BallParts::SetColor(Fly, FLinearColor(0.01f, 0.01f, 0.01f));
			Flies.Add(Fly);

			// Radius, angular speed, height, phase
			FlyOrbits.Add(FVector4f(FMath::FRandRange(18.f, 45.f), FMath::FRandRange(2.5f, 5.f) * (FMath::RandBool() ? 1.f : -1.f),
				FMath::FRandRange(35.f, 70.f), FMath::FRandRange(0.f, UE_TWO_PI)));
		}
	}

	for (UStaticMeshComponent* Fly : Flies)
	{
		Fly->SetVisibility(bActive);
	}
}

void UCorpseComponent::AnimateFlies(float DeltaTime)
{
	FlyTime += DeltaTime;
	for (int32 Index = 0; Index < Flies.Num(); ++Index)
	{
		// Wobbly orbits above the body: circle plus fast jitter reads as buzzing
		const FVector4f& Orbit = FlyOrbits[Index];
		const float Angle = Orbit.W + FlyTime * Orbit.Y;
		const float Radius = Orbit.X * (1.f + 0.25f * FMath::Sin(FlyTime * 3.1f + Orbit.W));
		const FVector Offset(
			FMath::Cos(Angle) * Radius + FMath::Sin(FlyTime * 23.f + Index) * 1.5f,
			FMath::Sin(Angle) * Radius + FMath::Cos(FlyTime * 19.f + Index) * 1.5f,
			Orbit.Z + FMath::Sin(FlyTime * 2.3f + Orbit.W) * 8.f);
		Flies[Index]->SetRelativeLocation(Offset);
	}
}
