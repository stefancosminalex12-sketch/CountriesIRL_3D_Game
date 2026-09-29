// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Styling/SlateBrush.h"

class ACIRLPaperDollStage;

/**
 *  Shows the paper-doll studio's picture of the ball (UI/CIRLPaperDollStage) and turns the ball when the mouse drags
 *  across it; double-click faces it forward again. Films only while this widget exists.
 */
class SCIRLPaperDollView : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SCIRLPaperDollView) {}
		SLATE_ARGUMENT(TWeakObjectPtr<ACIRLPaperDollStage>, Stage)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SCIRLPaperDollView() override;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;

private:

	TWeakObjectPtr<ACIRLPaperDollStage> Stage;
	FSlateBrush DollBrush;

	/** Degrees the ball turns per pixel of mouse movement */
	float TurnSpeed = 0.5f;
};
