// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Sound/SoundBase.h"
#include "CIRLMusicSettings.generated.h"

/** One piece of music and the name shown while it plays */
USTRUCT()
struct FCIRLMusicTrack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Music")
	TSoftObjectPtr<USoundBase> Sound;

	/** Shown as "Now playing" */
	UPROPERTY(EditAnywhere, Category = "Music")
	FString Title;
};

/** The footstep recordings for one kind of ground; the variations are picked at random */
USTRUCT()
struct FCIRLFootstepSet
{
	GENERATED_BODY()

	/** The ground's physical surface name (Project Settings > Physics > Physical Surface), e.g. Gravel */
	UPROPERTY(EditAnywhere, Category = "Footsteps")
	FName Surface;

	UPROPERTY(EditAnywhere, Category = "Footsteps")
	TArray<TSoftObjectPtr<USoundBase>> Sounds;
};

/**
 *  Which music plays where (Project Settings > Crowns & Commoners Audio). Music assets are in
 *  /Game/CrownsAndCommoners/Audio/Music (Tools/Unreal/import_audio.py); they don't loop, the playlist moves on.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Crowns & Commoners Audio"))
class UCIRLMusicSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	virtual FName GetCategoryName() const override { return TEXT("Project"); }

	/** Main menu music: the first track always plays first, then the rest in random order */
	UPROPERTY(config, EditAnywhere, Category = "Music")
	TArray<FCIRLMusicTrack> TitlePlaylist;

	/** Music in the world (the In-Game Music slider): the first track when you enter the game, then the rest in random order */
	UPROPERTY(config, EditAnywhere, Category = "Music")
	TArray<FCIRLMusicTrack> GamePlaylist;

	/** Plays when the player dies, instead of the world's music (which stops); empty = silence. Under the In-Game Music slider */
	UPROPERTY(config, EditAnywhere, Category = "Music")
	TArray<FCIRLMusicTrack> DeathMusic;

	/** A sound effect when the player dies (e.g. church bells tolling), under the Effects slider, not the music ones */
	UPROPERTY(config, EditAnywhere, Category = "Sound Effects")
	TSoftObjectPtr<USoundBase> DeathSound;

	/** Footsteps by kind of ground (looping recordings). The first set is used on any ground without its own */
	UPROPERTY(config, EditAnywhere, Category = "Sound Effects")
	TArray<FCIRLFootstepSet> Footsteps;

	/** Landing from a jump or a fall (one picked at random) */
	UPROPERTY(config, EditAnywhere, Category = "Sound Effects")
	TArray<TSoftObjectPtr<USoundBase>> LandingSounds;

	/** Fighting (each a set, one picked at random). For now every weapon and every armour shares them; later sets per
	 *  kind of weapon and armour. A weapon swinging through the air */
	UPROPERTY(config, EditAnywhere, Category = "Combat")
	TArray<TSoftObjectPtr<USoundBase>> SwingSounds;

	/** A blow stopped mostly by armour */
	UPROPERTY(config, EditAnywhere, Category = "Combat")
	TArray<TSoftObjectPtr<USoundBase>> ArmourHitSounds;

	/** A blow on flesh or cloth with a fist or a blunt weapon */
	UPROPERTY(config, EditAnywhere, Category = "Combat")
	TArray<TSoftObjectPtr<USoundBase>> FleshHitSounds;

	/** A blade cutting or stabbing into flesh (also the dagger of an assassination) */
	UPROPERTY(config, EditAnywhere, Category = "Combat")
	TArray<TSoftObjectPtr<USoundBase>> CutFleshSounds;

	/** Someone dies */
	UPROPERTY(config, EditAnywhere, Category = "Combat")
	TArray<TSoftObjectPtr<USoundBase>> KilledSounds;

	/** A body hitting the ground as it falls dead */
	UPROPERTY(config, EditAnywhere, Category = "Combat")
	TArray<TSoftObjectPtr<USoundBase>> BodyFallSounds;
};
