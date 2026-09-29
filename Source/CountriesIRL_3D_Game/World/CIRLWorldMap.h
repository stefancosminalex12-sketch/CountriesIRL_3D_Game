// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CIRLWorldMap.generated.h"

class UTexture2D;

/**
 *  A map picture of the world and how it lines up with the game world. Positions on it are in
 *  "game km" east and north from the map's zero (the rulers' zero: England's west and south edges).
 *  A DLC region can bring its own map as another asset of this type.
 *  Made by Tools/world/export_game_map.py + Tools/Unreal/import_world_map.py from the planning map.
 */
UCLASS(BlueprintType)
class UCIRLMapDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	TSoftObjectPtr<UTexture2D> Texture;

	/** Width / height of the picture */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	float Aspect = 1.f;

	/** Where game km (0, 0) is on the picture (0..1 across and down) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	FVector2D OriginUV = FVector2D::ZeroVector;

	/** How far one game km east (X) and north (Y) moves across the picture (0..1) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	FVector2D UVPerGameKm = FVector2D(0.01, 0.01);

	/** Game km (east, north) -> place on the picture (0..1 across, 0..1 down) */
	FVector2D GameKmToUV(const FVector2D& GameKm) const
	{
		return FVector2D(OriginUV.X + GameKm.X * UVPerGameKm.X, OriginUV.Y - GameKm.Y * UVPerGameKm.Y);
	}
};

/** Where a level's origin sits on the world map (for test levels that aren't built at their real place yet) */
USTRUCT()
struct FCIRLLevelMapAnchor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Map", meta=(AllowedClasses="/Script/Engine.World"))
	FSoftObjectPath Level;

	/** Game km east and north of the map's zero where this level's origin (0, 0, 0) is */
	UPROPERTY(EditAnywhere, Category="Map")
	FVector2D GameKm = FVector2D::ZeroVector;
};

namespace CIRLWorldMap
{
	/** World units in one game km */
	constexpr double UnitsPerGameKm = 100000.0;

	/**
	 *  Where a place in this level is on the world map, in game km (east, north).
	 *  World convention: +X is north, +Y is east. The real world's origin is the map's zero; test levels
	 *  listed in the settings' LevelMapAnchors sit wherever they are anchored.
	 */
	FVector2D ToGameKm(const UWorld* World, const FVector& Location);

	/** The world map from the settings (loads it) */
	UCIRLMapDefinition* LoadWorldMap();
}
