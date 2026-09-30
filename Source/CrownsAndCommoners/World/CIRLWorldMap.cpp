// Crowns & Commoners

#include "World/CIRLWorldMap.h"
#include "World/WorldSimulationSettings.h"
#include "Engine/World.h"

FVector2D CIRLWorldMap::ToGameKm(const UWorld* World, const FVector& Location)
{
	FVector2D Anchor = FVector2D::ZeroVector;
	if (World)
	{
		// In the editor's Play mode the level's package has a PIE prefix; compare the real name
		const FString LevelPackage = UWorld::RemovePIEPrefix(World->GetOutermost()->GetName());
		for (const FCIRLLevelMapAnchor& Entry : GetDefault<UWorldSimulationSettings>()->LevelMapAnchors)
		{
			if (Entry.Level.GetLongPackageName() == LevelPackage)
			{
				Anchor = Entry.GameKm;
				break;
			}
		}
	}
	return Anchor + FVector2D(Location.Y, Location.X) / UnitsPerGameKm;
}

UCIRLMapDefinition* CIRLWorldMap::LoadWorldMap()
{
	return GetDefault<UWorldSimulationSettings>()->WorldMap.LoadSynchronous();
}
