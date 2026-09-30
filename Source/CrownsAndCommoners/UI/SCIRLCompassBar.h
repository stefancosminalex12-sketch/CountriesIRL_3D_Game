// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

class APlayerController;
struct FSlateBrush;

/**
 *  The compass bar at the top of the screen (Master file > UI & HUD: compass bar by default, no minimap): a strip
 *  showing the directions the camera faces (N, NE, E...) with ticks, and markers for what matters: your horse (with
 *  its distance) and anyone fighting you. Markers outside the strip wait at its ends, so an enemy behind you still
 *  shows which way to turn. Quests, waypoints and places join it when they exist.
 *  Drawn in the menus' colours (charcoal and worn gold, Cinzel letters); the marker icons are the user's map markers.
 */
class SCIRLCompassBar : public SLeafWidget
{
public:

	SLATE_BEGIN_ARGS(SCIRLCompassBar) {}
		SLATE_ARGUMENT(TWeakObjectPtr<APlayerController>, Owner)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:

	/** Something shown on the strip */
	struct FMarker
	{
		const FSlateBrush* Icon = nullptr;
		FVector Location = FVector::ZeroVector;
		/** Shown under the icon when far enough away */
		bool bShowDistance = false;
		/** Stays at the strip's end when outside it */
		bool bClampToEnds = true;
	};

	/** Everything to show right now */
	void GatherMarkers(TArray<FMarker>& OutMarkers) const;

	TWeakObjectPtr<APlayerController> Owner;

	/** Strip size in 1080p pixels, and how many degrees of the horizon it shows */
	static constexpr float Width = 640.f;
	static constexpr float StripHeight = 30.f;
	static constexpr float FieldOfView = 160.f;
};
