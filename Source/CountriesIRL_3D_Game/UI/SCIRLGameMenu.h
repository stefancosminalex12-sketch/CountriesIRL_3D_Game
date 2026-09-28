// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SWidgetSwitcher;
class ACIRLPaperDollStage;
class SHorizontalBox;

/** The menu's tabs, left to right (Master file > UI & HUD) */
enum class ECIRLMenuTab : uint8
{
	Map,
	Quests,
	Equipment,
	Character,
	Game,		// resume, controls, quit (later: save/load, settings)
	Count
};

/**
 *  The in-game menu: a paused, blurred game behind one big panel with tabs across the top
 *  (Q / E or the shoulder buttons switch tabs, Esc closes). Each tab's content is its own widget;
 *  tabs that aren't built yet show a short note.
 */
class SCIRLGameMenu : public SCompoundWidget
{
public:

	DECLARE_DELEGATE(FOnCloseRequested);

	SLATE_BEGIN_ARGS(SCIRLGameMenu)
		: _InitialTab(ECIRLMenuTab::Game)
	{}
		SLATE_ARGUMENT(ECIRLMenuTab, InitialTab)
		/** Films the 3D character shown on the Equipment tab */
		SLATE_ARGUMENT(TWeakObjectPtr<ACIRLPaperDollStage>, PaperDollStage)
		/** Esc, Resume: the owner removes the menu and unpauses */
		SLATE_EVENT(FOnCloseRequested, OnCloseRequested)
		/** Back to the title screen */
		SLATE_EVENT(FOnCloseRequested, OnMainMenuRequested)
		/** Quit to desktop */
		SLATE_EVENT(FOnCloseRequested, OnQuitRequested)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetTab(ECIRLMenuTab NewTab);
	ECIRLMenuTab GetTab() const { return CurrentTab; }

	/** Moves keyboard/controller focus into the current tab */
	void FocusCurrentTab();

	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:

	TSharedRef<SWidget> MakeTabBar();
	TSharedRef<SWidget> MakeTabButton(ECIRLMenuTab Tab);
	TSharedRef<SWidget> MakeKeyHint(const FText& Key, const FText& Label) const;
	TSharedRef<SWidget> MakeComingSoon(const FText& Title, const FText& Note) const;
	TSharedRef<SWidget> MakeGameTab();
	TSharedRef<SWidget> MakeControlsList() const;

	/** A wide menu button in the panel style (Resume, Quit...) */
	TSharedRef<SWidget> MakeMenuButton(const FText& Label, FOnClicked OnClicked, TSharedPtr<class SButton>* OutButton = nullptr);

	void RebuildKeyHints();

	ECIRLMenuTab CurrentTab = ECIRLMenuTab::Game;

	TSharedPtr<SWidgetSwitcher> Pages;
	TSharedPtr<SHorizontalBox> KeyHints;

	/** First thing to focus on each tab (for keyboard/controller) */
	TSharedPtr<SWidget> TabFocus[static_cast<int32>(ECIRLMenuTab::Count)];

	FOnCloseRequested OnCloseRequested;
	FOnCloseRequested OnMainMenuRequested;
	FOnCloseRequested OnQuitRequested;
};
