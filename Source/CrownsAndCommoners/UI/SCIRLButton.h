// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Input/SButton.h"

/**
 *  A button without Slate's dashed focus rectangle. Our menus show keyboard/controller focus
 *  the same way as mouse hover (gold glow), so the default rectangle would only add noise.
 *  Built with SNew(SCIRLButton) and the normal SButton arguments.
 */
class SCIRLButton : public SButton
{
public:

	virtual const FSlateBrush* GetFocusBrush() const override { return nullptr; }

	/** Hovered by the mouse or focused by keyboard/controller: draw it highlighted */
	bool IsHighlighted() const { return IsHovered() || HasKeyboardFocus(); }
};
