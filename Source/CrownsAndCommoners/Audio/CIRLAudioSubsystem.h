// Crowns & Commoners

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
	Music,		// all music (main menu and in the world)
	GameMusic,	// "In-Game Music": music in the world, on top of Music
	Effects,	// every sound that isn't music (the default sound class)
};

/**
 *  The player's volumes, kept for the whole game session and saved in GameUserSettings.ini.
 *  Sounds belong to a sound class (/Game/CrownsAndCommoners/Audio/Classes): SC_Music for music, SC_GameMusic for music
 *  playing in the world (the in-game playlist sets it), SC_SFX for everything else (Project Settings > Audio >
 *  Default Sound Class), all children of SC_Master. The volumes are applied with one sound mix that overrides the
 *  classes: menu music plays at Master x Music, in-game music at Master x Music x In-Game Music, effects at Master x Effects.
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

	/** Sound class of music playing in the world (the In-Game Music slider) */
	USoundClass* GetGameMusicClass() const { return GameMusicClass; }

private:

	void OnWorldBeginPlay(UWorld* World);

	UWorld* CurrentWorld() const;

	UPROPERTY(config)
	// A new player starts at 60%, not full blast (user, 2026-09-30)
	float MasterVolume = 0.6f;

	UPROPERTY(config)
	float MusicVolume = 0.8f;

	/** Starts at half: the world's music sits under the game */
	UPROPERTY(config)
	float GameMusicVolume = 0.5f;

	UPROPERTY(config)
	float EffectsVolume = 1.f;

	UPROPERTY(Transient)
	TObjectPtr<USoundMix> VolumeMix;

	UPROPERTY(Transient)
	TObjectPtr<USoundClass> MusicClass;

	UPROPERTY(Transient)
	TObjectPtr<USoundClass> GameMusicClass;

	UPROPERTY(Transient)
	TObjectPtr<USoundClass> EffectsClass;

	FDelegateHandle WorldBeginPlayHandle;
};
