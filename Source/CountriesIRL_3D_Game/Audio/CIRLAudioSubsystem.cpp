// CountriesIRL 3D Game

#include "Audio/CIRLAudioSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	const TCHAR* MusicClassPath = TEXT("/Game/CountriesIRL/Audio/Classes/SC_Music.SC_Music");
	const TCHAR* EffectsClassPath = TEXT("/Game/CountriesIRL/Audio/Classes/SC_SFX.SC_SFX");
}

UCIRLAudioSubsystem* UCIRLAudioSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UCIRLAudioSubsystem>() : nullptr;
}

void UCIRLAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MusicClass = LoadObject<USoundClass>(nullptr, MusicClassPath);
	EffectsClass = LoadObject<USoundClass>(nullptr, EffectsClassPath);
	VolumeMix = NewObject<USoundMix>(this, TEXT("CIRLVolumeMix"));

	// Every level gets its volumes as soon as it has loaded (the player controllers also apply them on BeginPlay,
	// which covers the first level of a Play-in-Editor session)
	WorldBeginPlayHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UCIRLAudioSubsystem::OnWorldBeginPlay);
	ApplyVolumes(CurrentWorld());
}

void UCIRLAudioSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(WorldBeginPlayHandle);
	Super::Deinitialize();
}

float UCIRLAudioSubsystem::GetVolume(ECIRLVolume Which) const
{
	switch (Which)
	{
	case ECIRLVolume::Master:	return MasterVolume;
	case ECIRLVolume::Music:	return MusicVolume;
	default:					return EffectsVolume;
	}
}

void UCIRLAudioSubsystem::SetVolume(ECIRLVolume Which, float Value)
{
	Value = FMath::Clamp(Value, 0.f, 1.f);
	switch (Which)
	{
	case ECIRLVolume::Master:	MasterVolume = Value; break;
	case ECIRLVolume::Music:	MusicVolume = Value; break;
	default:					EffectsVolume = Value; break;
	}
	ApplyVolumes(CurrentWorld());
}

void UCIRLAudioSubsystem::SaveVolumes()
{
	SaveConfig();
}

void UCIRLAudioSubsystem::ApplyVolumes(UWorld* World)
{
	if (!World || !VolumeMix)
	{
		return;
	}
	// Each class gets its final volume directly (not through the parent), so the result is exactly Master x slider
	if (MusicClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(World, VolumeMix, MusicClass, MasterVolume * MusicVolume, 1.f, 0.f, false);
	}
	if (EffectsClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(World, VolumeMix, EffectsClass, MasterVolume * EffectsVolume, 1.f, 0.f, false);
	}
	UGameplayStatics::PushSoundMixModifier(World, VolumeMix);
}

void UCIRLAudioSubsystem::OnWorldBeginPlay(UWorld* World)
{
	if (World && World->GetGameInstance() == GetGameInstance())
	{
		ApplyVolumes(World);
	}
}

UWorld* UCIRLAudioSubsystem::CurrentWorld() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetWorld() : nullptr;
}
