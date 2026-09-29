// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class ACIRLPaperDollStage;
class SCIRLButton;

/**
 *  The Character tab. For now: choose which coat of arms your ball wears, from the houses of 1455
 *  (Characters/Heraldry), with the 3D ball in the middle and a card about the house on the right.
 *  Name, house, skills and reputation come here later.
 */
class SCIRLCharacterPage : public SCompoundWidget
{
public:

	DECLARE_DELEGATE_OneParam(FOnArmsChosen, int32 /*Index into CIRLHeraldry::All()*/);

	SLATE_BEGIN_ARGS(SCIRLCharacterPage)
		: _InitialArms(0)
	{}
		SLATE_ARGUMENT(TWeakObjectPtr<ACIRLPaperDollStage>, Stage)
		SLATE_ARGUMENT(int32, InitialArms)
		SLATE_EVENT(FOnArmsChosen, OnArmsChosen)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Where keyboard/controller focus starts: the chosen house */
	TSharedPtr<SWidget> GetFirstFocus() const;

private:

	TSharedRef<SWidget> MakeHouseButton(int32 Index);
	TSharedRef<SWidget> MakeHouseCard();

	void Choose(int32 Index);

	TArray<TSharedPtr<SCIRLButton>> Buttons;
	int32 Chosen = 0;
	FOnArmsChosen OnArmsChosen;
};
