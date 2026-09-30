// Crowns & Commoners

#include "UI/SCIRLPaperDollView.h"
#include "UI/CIRLPaperDollStage.h"
#include "UI/CIRLUIStyle.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "CIRLPaperDollView"

using namespace CIRLUIStyle;

void SCIRLPaperDollView::Construct(const FArguments& InArgs)
{
	Stage = InArgs._Stage;

	if (ACIRLPaperDollStage* Studio = Stage.Get())
	{
		Studio->SetCapturing(true);
		if (UMaterialInstanceDynamic* Picture = Studio->GetPicture())
		{
			DollBrush.SetResourceObject(Picture);
			DollBrush.ImageSize = FVector2D(Studio->GetResolution());
			DollBrush.DrawAs = ESlateBrushDrawType::Image;
		}
	}

	ChildSlot
	[
		SNew(SOverlay)

		+ SOverlay::Slot()
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			[
				SNew(SImage).Image(&DollBrush)
			]
		]

		// Quiet hint while the mouse is over the ball
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("DragToTurn", "Drag to turn"))
			.Font(Font(EFont::BodyItalic, 17.f))
			.ColorAndOpacity_Lambda([this]() { return FSlateColor(TextMuted() * FLinearColor(1.f, 1.f, 1.f, IsHovered() ? 0.9f : 0.f)); })
		]
	];
}

SCIRLPaperDollView::~SCIRLPaperDollView()
{
	if (ACIRLPaperDollStage* Studio = Stage.Get())
	{
		Studio->SetCapturing(false);
	}
}

FReply SCIRLPaperDollView::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return FReply::Handled().CaptureMouse(SharedThis(this));
	}
	return FReply::Unhandled();
}

FReply SCIRLPaperDollView::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (HasMouseCapture() && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return FReply::Handled().ReleaseMouseCapture();
	}
	return FReply::Unhandled();
}

FReply SCIRLPaperDollView::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (HasMouseCapture())
	{
		if (ACIRLPaperDollStage* Studio = Stage.Get())
		{
			// Drag right: the ball turns to its left, like spinning a globe
			Studio->AddDollYaw(-MouseEvent.GetCursorDelta().X * TurnSpeed);
		}
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SCIRLPaperDollView::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (ACIRLPaperDollStage* Studio = Stage.Get())
	{
		Studio->ResetDollYaw();
	}
	return FReply::Handled();
}

FCursorReply SCIRLPaperDollView::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	return FCursorReply::Cursor(HasMouseCapture() ? EMouseCursor::GrabHandClosed : EMouseCursor::GrabHand);
}

#undef LOCTEXT_NAMESPACE
