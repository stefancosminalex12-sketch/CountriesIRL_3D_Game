// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Items/ItemTypes.h"
#include "CIRLItemDatabase.generated.h"

class UDataTable;

/** Every kind of item in the game, looked up by id. Loads the item tables named in UCIRLItemSettings. */
UCLASS()
class UCIRLItemDatabase : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	static UCIRLItemDatabase* Get(const UObject* WorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** The item with this id, or nullptr */
	const FCIRLItemRow* Find(FName ItemId) const;

	/** Every item id, in table order */
	const TArray<FName>& GetAllIds() const { return AllIds; }

private:

	UPROPERTY(Transient)
	TArray<TObjectPtr<UDataTable>> Tables;

	TArray<FName> AllIds;
};
