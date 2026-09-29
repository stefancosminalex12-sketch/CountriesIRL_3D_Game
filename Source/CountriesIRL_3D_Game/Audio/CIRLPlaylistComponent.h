// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Audio/CIRLMusicSettings.h"
#include "CIRLPlaylistComponent.generated.h"

class UAudioComponent;

/**
 *  Plays a list of music tracks one after another: the first track first, then the others in random order
 *  (each plays once before any repeats, never the same track twice in a row). Next() skips with a short
 *  crossfade. Tracks play as UI sounds, so they keep going while the game is paused.
 */
UCLASS()
class UCIRLPlaylistComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Starts the playlist from its first track */
	void Play(const TArray<FCIRLMusicTrack>& InTracks);

	/** Skips to the next track */
	void Next();

	/** Fades the music out and stops the playlist */
	void Stop(float FadeSeconds);

	/** Name of the track playing now (empty if none) */
	FString GetCurrentTitle() const;

	bool IsPlaying() const { return CurrentIndex != INDEX_NONE; }

protected:

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	void PlayTrack(int32 Index, float FadeInSeconds);
	int32 PickNext();

	UFUNCTION()
	void HandleTrackFinished();

	TArray<FCIRLMusicTrack> Tracks;

	/** Tracks still to play before the order is shuffled again */
	TArray<int32> Bag;

	int32 CurrentIndex = INDEX_NONE;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> Current;

	FTimerHandle GapTimer;
};
