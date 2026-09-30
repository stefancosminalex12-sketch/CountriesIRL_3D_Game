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

	/** The player's footsteps (a loop, louder and quicker when running, quiet when sneaking). Later one per kind of ground */
	UPROPERTY(config, EditAnywhere, Category = "Sound Effects")
	TSoftObjectPtr<USoundBase> FootstepSound;
};
