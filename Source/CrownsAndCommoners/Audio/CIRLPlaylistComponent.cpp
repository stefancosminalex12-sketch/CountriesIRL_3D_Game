// CountriesIRL 3D Game

#include "Audio/CIRLPlaylistComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

namespace
{
	/** Pause between two tracks that end naturally */
	constexpr float GapSeconds = 1.5f;
	/** Crossfade when the player skips */
	constexpr float SkipFadeSeconds = 0.8f;
}

void UCIRLPlaylistComponent::Play(const TArray<FCIRLMusicTrack>& InTracks, USoundClass* SoundClass)
{
	TrackClass = SoundClass;
	Tracks = InTracks.FilterByPredicate([](const FCIRLMusicTrack& Track) { return !Track.Sound.IsNull(); });
	Bag.Reset();
	if (Tracks.Num() > 0)
	{
		PlayTrack(0, 2.f);
	}
}

void UCIRLPlaylistComponent::Next()
{
	if (Tracks.Num() > 1 && CurrentIndex != INDEX_NONE)
	{
		PlayTrack(PickNext(), SkipFadeSeconds);
	}
}

void UCIRLPlaylistComponent::Stop(float FadeSeconds)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(GapTimer);
	}
	if (Current)
	{
		Current->OnAudioFinished.RemoveAll(this);
		Current->FadeOut(FadeSeconds, 0.f);
		Current = nullptr;
	}
	CurrentIndex = INDEX_NONE;
}

FString UCIRLPlaylistComponent::GetCurrentTitle() const
{
	return Tracks.IsValidIndex(CurrentIndex) ? Tracks[CurrentIndex].Title : FString();
}

void UCIRLPlaylistComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Stop(0.f);
	Super::EndPlay(EndPlayReason);
}

void UCIRLPlaylistComponent::PlayTrack(int32 Index, float FadeInSeconds)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(GapTimer);
	}
	// The old track fades out while the new one fades in
	if (Current)
	{
		Current->OnAudioFinished.RemoveAll(this);
		Current->FadeOut(FadeInSeconds, 0.f);
		Current = nullptr;
	}

	CurrentIndex = Index;
	USoundBase* Sound = Tracks[Index].Sound.LoadSynchronous();
	if (!Sound)
	{
		return;
	}
	// Not auto-destroyed until it stops; a UI sound keeps playing while the game is paused
	Current = UGameplayStatics::CreateSound2D(this, Sound, 1.f, 1.f, 0.f, nullptr, false, true);
	if (Current)
	{
		Current->bIsUISound = true;
		if (TrackClass)
		{
			Current->SoundClassOverride = TrackClass;
		}
		Current->OnAudioFinished.AddDynamic(this, &UCIRLPlaylistComponent::HandleTrackFinished);
		Current->FadeIn(FadeInSeconds);
	}
}

int32 UCIRLPlaylistComponent::PickNext()
{
	if (Bag.Num() == 0)
	{
		// Every track once, in a new random order, never starting with the one that just played
		for (int32 i = 0; i < Tracks.Num(); ++i)
		{
			if (i != CurrentIndex)
			{
				Bag.Add(i);
			}
		}
		for (int32 i = Bag.Num() - 1; i > 0; --i)
		{
			Bag.Swap(i, FMath::RandRange(0, i));
		}
	}
	return Bag.Pop();
}

void UCIRLPlaylistComponent::HandleTrackFinished()
{
	Current = nullptr;
	UWorld* World = GetWorld();
	if (!World || Tracks.Num() == 0)
	{
		return;
	}
	const int32 NextIndex = Tracks.Num() > 1 ? PickNext() : CurrentIndex;
	World->GetTimerManager().SetTimer(GapTimer, FTimerDelegate::CreateWeakLambda(this, [this, NextIndex]()
	{
		PlayTrack(NextIndex, 0.5f);
	}), GapSeconds, false);
}
