// Crowns & Commoners

#include "Characters/Heraldry.h"
#include "Engine/Texture2D.h"

#define LOCTEXT_NAMESPACE "CIRLHeraldry"

namespace CIRLHeraldry
{
	const TArray<FCIRLArms>& All()
	{
		static const TArray<FCIRLArms> Arms =
		{
			{ TEXT("england"), LOCTEXT("England", "England"), LOCTEXT("EnglandHolder", "St George, England's patron saint"),
				LOCTEXT("EnglandBlazon", "White, a red cross. English soldiers wore it as their badge."), LOCTEXT("SideEither", "Both sides") },
			{ TEXT("york"), LOCTEXT("York", "York"), LOCTEXT("YorkHolder", "Richard, Duke of York"),
				LOCTEXT("YorkBlazon", "His livery colours, murrey and blue, with the white rose of York."), LOCTEXT("SideYork", "York") },
			{ TEXT("lancaster"), LOCTEXT("Lancaster", "Lancaster"), LOCTEXT("LancasterHolder", "King Henry VI and his house"),
				LOCTEXT("LancasterBlazon", "White and blue with the red rose of Lancaster (the red rose was made famous later, by the Tudors)."), LOCTEXT("SideLancaster", "Lancaster") },
			{ TEXT("neville"), LOCTEXT("Neville", "Neville"), LOCTEXT("NevilleHolder", "Richard Neville, Earl of Salisbury, and his son the Earl of Warwick"),
				LOCTEXT("NevilleBlazon", "Red, a white saltire."), LOCTEXT("SideYork2", "York") },
			{ TEXT("percy"), LOCTEXT("Percy", "Percy"), LOCTEXT("PercyHolder", "Henry Percy, Earl of Northumberland, killed at St Albans in 1455"),
				LOCTEXT("PercyBlazon", "Gold, a blue lion rampant."), LOCTEXT("SideLancasterPercy", "Lancaster") },
			{ TEXT("mowbray"), LOCTEXT("Mowbray", "Mowbray"), LOCTEXT("MowbrayHolder", "John Mowbray, Duke of Norfolk"),
				LOCTEXT("MowbrayBlazon", "Red, a silver lion rampant."), LOCTEXT("SideYorkMowbray", "York") },
			{ TEXT("stafford"), LOCTEXT("Stafford", "Stafford"), LOCTEXT("StaffordHolder", "Humphrey Stafford, Duke of Buckingham"),
				LOCTEXT("StaffordBlazon", "Gold, a red chevron."), LOCTEXT("SideLancaster2", "Lancaster") },
			{ TEXT("talbot"), LOCTEXT("Talbot", "Talbot"), LOCTEXT("TalbotHolder", "John Talbot, Earl of Shrewsbury"),
				LOCTEXT("TalbotBlazon", "Quartered: the Talbot lions, the two red lions of Strange and the red martlets of Furnival."), LOCTEXT("SideLancasterTalbot", "Lancaster") },
			{ TEXT("clifford"), LOCTEXT("Clifford", "Clifford"), LOCTEXT("CliffordHolder", "Thomas, Lord Clifford, killed at St Albans in 1455"),
				LOCTEXT("CliffordBlazon", "Checks of gold and blue, a red band across."), LOCTEXT("SideLancaster3", "Lancaster") },
			{ TEXT("courtenay"), LOCTEXT("Courtenay", "Courtenay"), LOCTEXT("CourtenayHolder", "Thomas Courtenay, Earl of Devon"),
				LOCTEXT("CourtenayBlazon", "Gold, three red roundels, a blue label."), LOCTEXT("SideLancaster4", "Lancaster (later in the war; sides were still shifting in 1455)") },
			{ TEXT("bonville"), LOCTEXT("Bonville", "Bonville"), LOCTEXT("BonvilleHolder", "William, Lord Bonville, the Courtenays' bitter rival in Devon"),
				LOCTEXT("BonvilleBlazon", "Black, six silver stars."), LOCTEXT("SideYork3", "York (later in the war; sides were still shifting in 1455)") },
			{ TEXT("scrope"), LOCTEXT("Scrope", "Scrope of Bolton"), LOCTEXT("ScropeHolder", "John, Lord Scrope of Bolton, a Yorkshire lord"),
				LOCTEXT("ScropeBlazon", "Blue, a gold diagonal band."), LOCTEXT("SideYork4", "York") },
			{ TEXT("vere"), LOCTEXT("Vere", "de Vere"), LOCTEXT("VereHolder", "John de Vere, Earl of Oxford"),
				LOCTEXT("VereBlazon", "Quartered red and gold, a silver star in the first quarter."), LOCTEXT("SideLancaster5", "Lancaster") },
		};
		return Arms;
	}

	UTexture2D* LoadTexture(const FCIRLArms& Arms)
	{
		const FString Name = FString::Printf(TEXT("T_flag_%s"), *Arms.Id.ToString());
		const FString Path = FString::Printf(TEXT("/Game/CrownsAndCommoners/Characters/Flags/%s.%s"), *Name, *Name);
		return Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet));
	}

	int32 IndexOf(const UTexture2D* Texture)
	{
		if (!Texture)
		{
			return INDEX_NONE;
		}
		const TArray<FCIRLArms>& Arms = All();
		for (int32 Index = 0; Index < Arms.Num(); ++Index)
		{
			if (Texture->GetName() == FString::Printf(TEXT("T_flag_%s"), *Arms[Index].Id.ToString()))
			{
				return Index;
			}
		}
		return INDEX_NONE;
	}
}

#undef LOCTEXT_NAMESPACE
