// Crowns & Commoners

#include "World/PrecipitationComponent.h"
#include "World/WeatherSubsystem.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

namespace
{
	float Random01(int32 Index, int32 Channel)
	{
		uint32 Hash = static_cast<uint32>(Index) * 2654435761u ^ static_cast<uint32>(Channel) * 0x85EBCA6Bu;
		Hash ^= Hash >> 16;
		Hash *= 0x7FEB352Du;
		Hash ^= Hash >> 15;
		return (Hash & 0xFFFFFF) / 16777215.f;
	}

	/** Wraps a value into [-HalfSize, HalfSize) */
	float Wrap(float Value, float HalfSize)
	{
		const float Size = HalfSize * 2.f;
		return FMath::Fmod(FMath::Fmod(Value + HalfSize, Size) + Size, Size) - HalfSize;
	}
}

UPrecipitationComponent::UPrecipitationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetCastShadow(false);
	SetGenerateOverlapEvents(false);
	SetCanEverAffectNavigation(false);
	bUseAsOccluder = false;
	// Drops are placed around the camera every frame; skip bounds-based culling of the whole set
	SetAbsolute(true, true, true);
}

void UPrecipitationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const UWeatherSubsystem* Weather = GetWorld()->GetSubsystem<UWeatherSubsystem>();
	const APlayerController* Player = GetWorld()->GetFirstPlayerController();
	if (!Weather || !Player || !Player->PlayerCameraManager || !GetStaticMesh())
	{
		return;
	}

	// How much is falling (rain and snow each only when it's their turn), eased in and out
	const FWeatherState& Now = Weather->GetState();
	const float Target = (Now.bSnowing == bSnow) ? Now.Precipitation : 0.f;
	// Starts at the current weather; later changes ease in and out
	Amount = bStarted ? FMath::FInterpTo(Amount, Target, DeltaTime, 0.8f) : Target;
	bStarted = true;
	Time += DeltaTime;

	const int32 Visible = FMath::RoundToInt(MaxDrops * Amount);
	if (Visible == 0 && GetInstanceCount() == 0)
	{
		return;
	}

	// The box follows the camera; each drop keeps a fixed spot in a world-sized tiling so it doesn't slide with us
	const FVector Camera = Player->PlayerCameraManager->GetCameraLocation();
	SetWorldLocationAndRotation(Camera, FRotator::ZeroRotator);

	// Wind: blows FROM WindFrom (0 = north = +X, 90 = east = +Y)
	const float WindRadians = FMath::DegreesToRadians(Now.WindFrom);
	const FVector WindVelocity = -FVector(FMath::Cos(WindRadians), FMath::Sin(WindRadians), 0.f) * Now.WindSpeed * 100.f * WindInfluence;
	const FVector Fall = FVector(WindVelocity.X, WindVelocity.Y, -FallSpeed);
	const FQuat Tilt = FRotationMatrix::MakeFromZ(-Fall.GetSafeNormal()).ToQuat();
	const float FallTime = AreaHeight / FallSpeed;

	Transforms.SetNum(MaxDrops);
	for (int32 Index = 0; Index < MaxDrops; ++Index)
	{
		if (Index >= Visible)
		{
			Transforms[Index] = FTransform(FQuat::Identity, FVector::ZeroVector, FVector::ZeroVector);
			continue;
		}

		// Height along a repeating fall; sideways drift while falling
		const float Phase = FMath::Fmod(Time / FallTime + Random01(Index, 2), 1.f);
		const FVector Drift = WindVelocity * (Phase * FallTime);
		FVector Local(
			Wrap(Random01(Index, 0) * AreaRadius * 2.f - Camera.X + Drift.X, AreaRadius),
			Wrap(Random01(Index, 1) * AreaRadius * 2.f - Camera.Y + Drift.Y, AreaRadius),
			AreaHeight * (0.5f - Phase));
		if (bSnow)
		{
			// Flakes flutter
			const float Seed = Random01(Index, 3) * 10.f;
			Local.X += FMath::Sin(Time * 1.3f + Seed) * 18.f;
			Local.Y += FMath::Cos(Time * 1.1f + Seed * 1.7f) * 18.f;
		}
		Transforms[Index] = FTransform(Tilt, Local, DropSize / 100.f);
	}

	if (GetInstanceCount() != MaxDrops)
	{
		ClearInstances();
		AddInstances(Transforms, false, false);
	}
	else
	{
		BatchUpdateInstancesTransforms(0, Transforms, false, true, true);
	}
}
