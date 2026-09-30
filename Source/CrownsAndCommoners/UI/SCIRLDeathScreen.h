// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SButton;

/**
 *  Shown a moment after the player dies: the screen fades to black (a grim tomb-slab painting, darker at the edges),
 *  a memento mori emblem, DEAD, and the choice to respawn or go back to the main menu. The world keeps going behind
 *  it (your body lies where it fell). Pure Slate, in the menus' style.
 */
class SCIRLDeathScreen : public SCompoundWidget
{
public:

	DECLARE_DELEGATE(FOnChosen);

	SLATE_BEGIN_ARGS(SCIRLDeathScreen) {}
		SLATE_EVENT(FOnChosen, OnRespawn)
		SLATE_EVENT(FOnChosen, OnMainMenu)
		/** The big word: how you died (PERISHED, SLAIN...) */
		SLATE_ARGUMENT(FText, Title)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** The button that gets keyboard and controller focus */
	TSharedPtr<SWidget> GetFocusTarget() const;

private:

	/** Seconds since it appeared */
	float Elapsed() const;

	/** Moves the drips down out of the top edge as time goes on */
	TOptional<FSlateRenderTransform> DripsTransform() const;

	/** The drips start this share of the screen higher (hidden above the top edge) and run down over this many seconds */
	static constexpr float DripsStart = 0.55f;
	static constexpr float DripsSeconds = 9.f;

	TSharedPtr<class SImage> DripsImage;

	/** 0 when it appears, 1 once it has faded in */
	float FadeIn() const;

	/** How long it takes to fade in: slow, while the body falls */
	static constexpr float FadeSeconds = 3.f;

	/** The darkness never covers the world completely */
	static constexpr float MaxDarkness = 0.9f;

	TSharedPtr<SButton> RespawnButton;
	double ShownAt = 0.0;
};
