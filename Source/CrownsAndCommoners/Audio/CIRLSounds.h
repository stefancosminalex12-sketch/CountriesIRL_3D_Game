// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"

class USoundAttenuation;
class USoundBase;

/**
 *  Playing sound effects out in the world: one sound picked at random from a set (so repeated blows don't sound the
 *  same), a slightly different pitch each time, loud close by and fading out with distance.
 */
namespace CIRLSounds
{
	/** Plays one of Sounds at a world location (nothing if the set is empty) */
	void PlayAt(const UObject* WorldContext, const TArray<TSoftObjectPtr<USoundBase>>& Sounds, const FVector& Location, float Volume = 1.f);

	/** How sounds in the world fade with distance: full within 3 m, gone at about 35 m */
	USoundAttenuation* WorldAttenuation();
}
