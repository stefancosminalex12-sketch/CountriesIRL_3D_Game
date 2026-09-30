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
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** The button that gets keyboard and controller focus */
	TSharedPtr<SWidget> GetFocusTarget() const;

private:

	/** Seconds since it appeared */
	float Elapsed() const;

	/** 0 when it appears, 1 once it has faded in */
	float FadeIn() const;

	/** How long it takes to fade in: slow, while the body falls */
	static constexpr float FadeSeconds = 3.f;

	/** The darkness never covers the world completely */
	static constexpr float MaxDarkness = 0.9f;

	TSharedPtr<SButton> RespawnButton;
	double ShownAt = 0.0;
};
