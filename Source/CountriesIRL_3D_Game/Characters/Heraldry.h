// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"

class UTexture2D;

/**
 *  Coats of arms a ball can wear (the textures come from Tools/make_flags.py, in /Game/CountriesIRL/Characters/Flags).
 *  For now a fixed list so the arms can be tried out in the menu; they move into the houses' data when the
 *  house system is built, so DLC regions can bring their own.
 */
struct FCIRLArms
{
	/** Texture name without the T_flag_ prefix, e.g. "neville" */
	FName Id;
	FText House;
	/** Who bore these arms in 1455 */
	FText Holder;
	/** The arms in plain words */
	FText Blazon;
	/** York, Lancaster or neither, as of 1455 */
	FText Side;
};

namespace CIRLHeraldry
{
	/** Every coat of arms in the game, England's St George first */
	const TArray<FCIRLArms>& All();

	/** Index of the arms with this texture (or -1) */
	int32 IndexOf(const UTexture2D* Texture);

	/** Loads the texture for a coat of arms (nullptr if missing) */
	UTexture2D* LoadTexture(const FCIRLArms& Arms);
}
