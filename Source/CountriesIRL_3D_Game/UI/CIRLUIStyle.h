// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateBrush.h"

class SWidget;
struct FButtonStyle;

/**
 *  The look shared by every menu (Master file > UI & HUD > Menu look): dark charcoal panels with a thin
 *  worn-gold border, Cinzel for titles and tabs, EB Garamond for text. Menus use these instead of their own
 *  colours and fonts, so the whole UI can be restyled here.
 *  Sizes are in 1080p pixels; Slate's DPI scaling adapts them to the screen.
 */
namespace CIRLUIStyle
{
	enum class EFont : uint8
	{
		Title,			// Cinzel Regular: tabs, headings
		TitleSemiBold,
		TitleBold,
		Body,			// EB Garamond Regular: descriptions, labels
		BodySemiBold,
		BodyItalic,
	};

	/** Font from the imported font faces (/Game/CountriesIRL/UI/Fonts), built once */
	FSlateFontInfo Font(EFont Face, float Size);

	// Colours

	/** Worn gold: borders, dividers */
	FLinearColor Gold();
	/** Bright gold: the active tab, highlighted slots, selected text */
	FLinearColor GoldBright();
	/** Parchment: normal text */
	FLinearColor Text();
	/** Faded text: inactive tabs, hints, disabled items */
	FLinearColor TextMuted();

	// Brushes

	/** Main panel: near-black charcoal with the worn-gold border */
	const FSlateBrush* Panel();
	/** Inner card inside a panel (item card, slot box): a little lighter, faint border */
	const FSlateBrush* Card();
	/** Button and slot states */
	const FSlateBrush* ButtonNormal();
	const FSlateBrush* ButtonHovered();
	const FSlateBrush* ButtonPressed();
	/** A slot that isn't highlighted but is the one the item card describes: thin bright-gold outline */
	const FSlateBrush* SlotSelected();
	/** The open tab in the tab bar: slightly lighter box with a gold frame */
	const FSlateBrush* TabActive();
	/** Draws nothing */
	const FSlateBrush* NoBrush();
	/** Faint gold circle behind the character on the Equipment tab */
	const FSlateBrush* DollRing();
	/** Small key cap for key hints (Q, E, F, R...) */
	const FSlateBrush* KeyCap();
	/** Solid gold, for dividers, underlines and the little diamonds between tabs */
	const FSlateBrush* GoldFill();
	/** Darkens the game behind an open menu */
	const FSlateBrush* ScreenDim();
	/** Dark leather texture that tiles across big panels (Art/AI/Menu/panel_texture) */
	const FSlateBrush* PanelTexture();
	/** Worn-gold filigree corner (top-left; mirrored for the other corners) */
	const FSlateBrush* CornerOrnament();
	/** Title screen painting (the village at golden hour) */
	const FSlateBrush* TitleBackground();
	/** Loading screen paintings (market, soldiers, castle, army camp) */
	int32 LoadingPaintingCount();
	const FSlateBrush* LoadingPainting(int32 Index);
	/** White fading to clear left-to-right / top-to-bottom; tint it to darken part of a painting */
	const FSlateBrush* GradientLeft();
	const FSlateBrush* GradientBottom();

	/** Item icon from /Game/CountriesIRL/UI/Icons (T_<Name>), or nullptr if that icon hasn't been made yet */
	const FSlateBrush* ItemIcon(FName Name);

	/** Invisible button frame, for buttons that draw their own hover/pressed look */
	const FButtonStyle& PlainButtonStyle();

	// Widgets

	/** The big menu panel: leather texture, worn-gold border and a gold ornament in each corner */
	TSharedRef<SWidget> MakeOrnatePanel(const TSharedRef<SWidget>& Content, const FMargin& Padding);
}
