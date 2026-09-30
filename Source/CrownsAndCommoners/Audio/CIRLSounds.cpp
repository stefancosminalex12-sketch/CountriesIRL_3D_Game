// Crowns & Commoners

#include "Audio/CIRLSounds.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundConcurrency.h"

namespace CIRLSounds
{
	USoundAttenuation* WorldAttenuation()
	{
		static TStrongObjectPtr<USoundAttenuation> Attenuation;
		if (!Attenuation.IsValid())
		{
			Attenuation.Reset(NewObject<USoundAttenuation>(GetTransientPackage(), TEXT("CIRLWorldAttenuation")));
			FSoundAttenuationSettings& Settings = Attenuation->Attenuation;
			Settings.bAttenuate = true;
			Settings.bSpatialize = true;
			Settings.AttenuationShape = EAttenuationShape::Sphere;
			Settings.AttenuationShapeExtents = FVector(300.f, 0.f, 0.f);
			Settings.FalloffDistance = 3500.f;
		}
		return Attenuation.Get();
	}

	USoundConcurrency* WorldConcurrency()
	{
		static TStrongObjectPtr<USoundConcurrency> Concurrency;
		if (!Concurrency.IsValid())
		{
			Concurrency.Reset(NewObject<USoundConcurrency>(GetTransientPackage(), TEXT("CIRLWorldConcurrency")));
			FSoundConcurrencySettings& Settings = Concurrency->Concurrency;
			Settings.MaxCount = 5;
			Settings.ResolutionRule = EMaxConcurrentResolutionRule::StopOldest;
			Settings.RetriggerTime = 0.06f;
		}
		return Concurrency.Get();
	}

	void PlayAt(const UObject* WorldContext, const TArray<TSoftObjectPtr<USoundBase>>& Sounds, const FVector& Location, float Volume)
	{
		if (Sounds.Num() == 0 || !WorldContext)
		{
			return;
		}
		if (USoundBase* Sound = Sounds[FMath::RandHelper(Sounds.Num())].LoadSynchronous())
		{
			UGameplayStatics::PlaySoundAtLocation(WorldContext, Sound, Location, Volume, FMath::FRandRange(0.93f, 1.07f), 0.f, WorldAttenuation(), WorldConcurrency());
		}
	}
}
