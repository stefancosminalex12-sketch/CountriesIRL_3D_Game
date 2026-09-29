// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CIRLAudioSubsystem.generated.h"

class USoundClass;
class USoundMix;

/** The player's volume sliders (Settings) */
enum class ECIRLVolume : uint8
{
	Master,		// "Sound": everything
	Music,
	Effects,	// every sound that isn't music (the default sound class)
};

/**
 *  The player's volumes, kept for the whole game session and saved in GameUserSettings.ini.
 *  Sounds belong to a sound class (/Game/CountriesIRL/Audio/Classes): SC_Music for music, SC_SFX for everything
 *  else (Project Settings > Audio > Default Sound Class), both children of SC_Master. The volumes are applied with
 *  one sound mix that overrides the two classes: music plays at Master x Music, effects at Master x Effects.
 */
UCLASS(config = GameUserSettings)
class UCIRLAudioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	static UCIRLAudioSubsystem* Get(const UObject* WorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 0..1 */
	float GetVolume(ECIRLVolume Which) const;

	/** Changes a volume and applies it at once (call SaveVolumes when the player lets go of the slider) */
	void SetVolume(ECIRLVolume Which, float Value);

	/** Writes the volumes to GameUserSettings.ini */
	void SaveVolumes();

	/** Applies the volumes to a world's audio device (done for every loaded level) */
	void ApplyVolumes(UWorld* World);

private:

	void OnWorldBeginPlay(UWorld* World);

	UWorld* CurrentWorld() const;

	UPROPERTY(config)
	float MasterVolume = 1.f;

	UPROPERTY(config)
	float MusicVolume = 0.8f;

	UPROPERTY(config)
	float EffectsVolume = 1.f;

	UPROPERTY(Transient)
	TObjectPtr<USoundMix> VolumeMix;

	UPROPERTY(Transient)
	TObjectPtr<USoundClass> MusicClass;

	UPROPERTY(Transient)
	TObjectPtr<USoundClass> EffectsClass;

	FDelegateHandle WorldBeginPlayHandle;
};
