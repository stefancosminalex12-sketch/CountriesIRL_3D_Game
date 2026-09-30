// Crowns & Commoners

#include "Items/CIRLItemDatabase.h"
#include "Items/CIRLItemSettings.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UCIRLItemDatabase* UCIRLItemDatabase::Get(const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UCIRLItemDatabase>() : nullptr;
}

void UCIRLItemDatabase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	for (const TSoftObjectPtr<UDataTable>& TablePath : GetDefault<UCIRLItemSettings>()->ItemTables)
	{
		UDataTable* Table = TablePath.LoadSynchronous();
		if (!Table || !Table->GetRowStruct() || !Table->GetRowStruct()->IsChildOf(FCIRLItemRow::StaticStruct()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Item table %s is missing or isn't made of FCIRLItemRow rows"), *TablePath.ToString());
			continue;
		}
		Tables.Add(Table);
		for (const FName& RowName : Table->GetRowNames())
		{
			AllIds.AddUnique(RowName);
		}
	}
}

const FCIRLItemRow* UCIRLItemDatabase::Find(FName ItemId) const
{
	// Later tables win, so a DLC can adjust an item
	for (int32 Index = Tables.Num() - 1; Index >= 0; --Index)
	{
		if (const FCIRLItemRow* Row = Tables[Index]->FindRow<FCIRLItemRow>(ItemId, TEXT("ItemDatabase"), false))
		{
			return Row;
		}
	}
	return nullptr;
}
