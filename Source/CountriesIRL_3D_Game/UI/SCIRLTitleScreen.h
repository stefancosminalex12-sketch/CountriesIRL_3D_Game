// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SCIRLButton;
class SCIRLSoundSettings;
class UCIRLAudioSubsystem;

/**
 *  The title screen shown when the game starts: the village painting, the game's name and
 *  New Game / Continue / Settings / Quit. New Game swaps to a loading painting before the world loads,
 *  so the screen shows something nice during the load instead of freezing on the menu.
 *  Settings swaps the buttons for the settings panel (volume sliders for now); Back or Esc returns.
 */
class SCIRLTitleScreen : public SCompoundWidget
{
public:

	DECLARE_DELEGATE(FOnAction);

	SLATE_BEGIN_ARGS(SCIRLTitleScreen) {}
		/** Volumes shown and changed by Settings */
		SLATE_ARGUMENT(TWeakObjectPtr<UCIRLAudioSubsystem>, Audio)
		/** Name of the song playing, shown bottom right (hidden when empty) */
		SLATE_ATTRIBUTE(FText, NowPlaying)
		/** Skip to the next song (the Next song button, N, controller Y) */
		SLATE_EVENT(FOnAction, OnNextSong)
		/** Load the world (called once the loading painting is on screen) */
		SLATE_EVENT(FOnAction, OnNewGame)
		SLATE_EVENT(FOnAction, OnQuit)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void FocusFirstButton();

	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:

	TSharedRef<SWidget> MakeSettingsPanel();

	/** Bottom right: "Now playing" and the song's name, with the Next song button */
	TSharedRef<SWidget> MakeNowPlaying();

	TAttribute<FText> NowPlaying;
	FOnAction OnNextSong;

	void OpenSettings();
	void CloseSettings();

	TWeakObjectPtr<UCIRLAudioSubsystem> Audio;
	TSharedPtr<SCIRLSoundSettings> SoundSettings;
	TSharedPtr<SCIRLButton> SettingsButton;
	bool bSettingsOpen = false;

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
