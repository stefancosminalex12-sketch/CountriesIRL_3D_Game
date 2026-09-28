// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SCIRLButton;

/**
 *  The title screen shown when the game starts: the village painting, the game's name and
 *  New Game / Continue / Settings / Quit. New Game swaps to a loading painting before the world loads,
 *  so the screen shows something nice during the load instead of freezing on the menu.
 */
class SCIRLTitleScreen : public SCompoundWidget
{
public:

	DECLARE_DELEGATE(FOnAction);

	SLATE_BEGIN_ARGS(SCIRLTitleScreen) {}
		/** Load the world (called once the loading painting is on screen) */
		SLATE_EVENT(FOnAction, OnNewGame)
		SLATE_EVENT(FOnAction, OnQuit)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void FocusFirstButton();

	virtual bool SupportsKeyboardFocus() const override { return true; }

private:

	/** A title-screen button: large text, a gold diamond in front of it while highlighted */
	TSharedRef<SWidget> MakeTitleButton(const FText& Label, const FText& DisabledNote, bool bEnabled, TFunction<void()> OnClicked, TSharedPtr<SCIRLButton>* OutButton = nullptr);

	TSharedRef<SWidget> MakeLoadingScreen();

	/** Shows the loading painting, then starts the load a moment later (after it has been drawn) */
	void StartNewGame();

	TSharedPtr<SCIRLButton> NewGameButton;

	bool bLoading = false;

	/** Seconds since the title appeared, for the fade in from black */
	float Age = 0.f;

	FOnAction OnNewGame;
	FOnAction OnQuit;
};
