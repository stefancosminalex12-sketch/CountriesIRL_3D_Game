// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"
#include "Styling/SlateBrush.h"
#include "UObject/StrongObjectPtr.h"
#include "Engine/Texture2D.h"
#include "World/CIRLWorldMap.h"

/**
 *  The menu's Map tab: the world map, with you on it (a marker pointing where you face).
 *  Drag or WASD / arrows / left stick to move around, mouse wheel, + / - or the triggers to zoom,
 *  Space (controller Y) to find yourself again. Opens centred on you.
 *  Later: waypoints, pins, what you know (Master file > UI & HUD > Two maps).
 */
class SCIRLMapPage : public SLeafWidget
{
public:

	SLATE_BEGIN_ARGS(SCIRLMapPage) {}
		/** Who "you" are on the map */
		SLATE_ARGUMENT(TWeakObjectPtr<AActor>, Player)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(400.f, 300.f); }
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnAnalogValueChanged(const FGeometry& MyGeometry, const FAnalogInputEvent& InAnalogInputEvent) override;
	virtual void OnFocusLost(const FFocusEvent& InFocusEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;

	/** Puts you in the middle of the view */
	void CenterOnPlayer();

private:

	/** Moves and zooms while keys or sticks are held */
	EActiveTimerReturnType Update(double InCurrentTime, float InDeltaTime);

	/** Zooms by a factor, keeping the map point under LocalPoint where it is */
	void ZoomAt(float Factor, const FVector2D& LocalPoint);

	/** Size of the whole map picture on screen at the current zoom, and where its top-left corner is */
	FVector2D MapScreenSize() const;
	FVector2D MapTopLeft() const;

	void ClampView();

	TStrongObjectPtr<UCIRLMapDefinition> Map;
	TStrongObjectPtr<UTexture2D> Texture;
	FSlateBrush MapBrush;

	TWeakObjectPtr<AActor> Player;

	/** The map point (0..1) in the middle of the view, and the zoom (1 = the whole map's height fits) */
	FVector2D ViewCenter = FVector2D(0.5, 0.5);
	float Zoom = 4.f;

	/** Size of the view the last time it was drawn (input needs it between frames) */
	mutable FVector2D ViewSize = FVector2D(1200.f, 650.f);

	bool bDragging = false;
	FVector2D LastMouse = FVector2D::ZeroVector;

	/** Held keys, the left stick (x right, y up) and the triggers (0..1) */
	TSet<FKey> HeldKeys;
	FVector2D StickPan = FVector2D::ZeroVector;
	float TriggerZoomIn = 0.f;
	float TriggerZoomOut = 0.f;
};
