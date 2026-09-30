// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "World/CIRLWorldMap.h"
#include "WorldSimulationSettings.generated.h"

class UClimateProfile;
class UMaterialParameterCollection;

/**
 *  Project-wide settings for the living world (Project Settings > Game > Crowns & Commoners World).
 *  Stored in Config/DefaultGame.ini so DLC regions can override them with data, not code.
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="Crowns & Commoners World"))
class UWorldSimulationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	/** Level shown when the game starts: the title screen */
	UPROPERTY(config, EditAnywhere, Category="Maps", meta=(AllowedClasses="/Script/Engine.World"))
	FSoftObjectPath TitleMap = FSoftObjectPath(TEXT("/Game/CrownsAndCommoners/Maps/L_MainMenu.L_MainMenu"));

	/** Level that New Game opens (the world; for now the test sandbox) */
	UPROPERTY(config, EditAnywhere, Category="Maps", meta=(AllowedClasses="/Script/Engine.World"))
	FSoftObjectPath NewGameMap = FSoftObjectPath(TEXT("/Game/CrownsAndCommoners/Maps/L_DevSandbox.L_DevSandbox"));

	/** The map of the world shown on the menu's Map tab */
	UPROPERTY(config, EditAnywhere, Category="Maps")
	TSoftObjectPtr<UCIRLMapDefinition> WorldMap = TSoftObjectPtr<UCIRLMapDefinition>(FSoftObjectPath(TEXT("/Game/CrownsAndCommoners/UI/Map/DA_WorldMap_England1455.DA_WorldMap_England1455")));

	/** Test levels placed somewhere on the world map (the sandbox stands in for Middleham) */
	UPROPERTY(config, EditAnywhere, Category="Maps")
	TArray<FCIRLLevelMapAnchor> LevelMapAnchors;

	/** In-game date and time when a new game starts. Dates are in the Julian calendar, as used in England until 1752. */
	UPROPERTY(config, EditAnywhere, Category="Time")
	FDateTime StartDateTime = FDateTime(1455, 5, 1, 7, 0, 0);

	/** Real minutes for one full in-game day while playing normally */
	UPROPERTY(config, EditAnywhere, Category="Time", meta=(ClampMin=1))
	float DayLengthMinutes = 48.f;

	/** Days to add to a Julian date to get the astronomical (Gregorian) date. 9 for 1300-1500, 10 for 1500-1700. */
	UPROPERTY(config, EditAnywhere, Category="Time")
	int32 JulianToGregorianDays = 9;

	/** Latitude used for the sun and moon (degrees north). York is 53.96. */
	UPROPERTY(config, EditAnywhere, Category="Sky")
	float Latitude = 53.96f;

	/** World height (cm) that counts as sea level: the sandbox floor sits at York's height (about 14 m) */
	UPROPERTY(config, EditAnywhere, Category="Sky")
	float WorldZAtSeaLevel = -1400.f;

	/** The region's climate (temperatures, foliage and grass through the year). Empty = built-in Yorkshire defaults. */
	UPROPERTY(config, EditAnywhere, Category="Seasons")
	TSoftObjectPtr<UClimateProfile> Climate = TSoftObjectPtr<UClimateProfile>(FSoftObjectPath(TEXT("/Game/CrownsAndCommoners/World/DA_Climate_Yorkshire.DA_Climate_Yorkshire")));

	/** Material parameters the seasons and weather write to (LeafAmount, LeafTint, GrassTint, Frost, Mist, Temperature,
	 *  CloudCover, Rain, Snowfall, Wetness, SnowCover, Wind) */
	UPROPERTY(config, EditAnywhere, Category="Seasons")
	TSoftObjectPtr<UMaterialParameterCollection> SeasonParameters = TSoftObjectPtr<UMaterialParameterCollection>(FSoftObjectPath(TEXT("/Game/CrownsAndCommoners/World/MPC_Season.MPC_Season")));
};
