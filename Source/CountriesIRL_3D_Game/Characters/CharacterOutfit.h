// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterOutfit.generated.h"

class USkeletalMesh;
class UIKRetargeter;

/**
 * A set of clothes/body parts worn under the countryball head (humanoid body style).
 * All parts share one skeleton; the first part is animated by retargeting the character's
 * animation onto it, the other parts copy that part's pose.
 * New outfits (and DLC regions) are just new assets of this type.
 */
UCLASS(BlueprintType)
class COUNTRIESIRL_3D_GAME_API UCharacterOutfit : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Mesh parts worn together, e.g. body, arms, legs, feet. The first one leads the others. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Outfit")
	TArray<TSoftObjectPtr<USkeletalMesh>> Parts;

	/** Retargeter from the character's animation skeleton to this outfit's skeleton */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Outfit")
	TSoftObjectPtr<UIKRetargeter> Retargeter;

	/** Bone at the top of the neck: the countryball head sits here */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Outfit")
	FName NeckBone = TEXT("Head");
};
