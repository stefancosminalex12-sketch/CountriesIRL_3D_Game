// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CIRLItemSettings.generated.h"

class UDataTable;

/**
 *  Which items exist and what a new character starts with (Project Settings > Crowns & Commoners Items).
 *  Stored in Config/DefaultGame.ini; a DLC region adds its own item table here.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Crowns & Commoners Items"))
class UCIRLItemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	virtual FName GetCategoryName() const override { return TEXT("Project"); }

	/** Item tables (rows of FCIRLItemRow). Later tables win if two define the same id */
	UPROPERTY(config, EditAnywhere, Category = "Items")
	TArray<TSoftObjectPtr<UDataTable>> ItemTables;

	/** What the commoner owns when a new game starts; each is put on if a slot is free */
	UPROPERTY(config, EditAnywhere, Category = "Items")
	TArray<FName> StartingItems;

	/** Test builds: the player also gets one of every item, to try the Equipment screen */
	UPROPERTY(config, EditAnywhere, Category = "Items")
	bool bGiveAllItemsForTesting = false;

	/** What a person can carry before it slows them (kg) */
	UPROPERTY(config, EditAnywhere, Category = "Items")
	float MaxCarryWeightKg = 30.f;
};
