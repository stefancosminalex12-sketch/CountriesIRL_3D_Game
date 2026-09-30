// Crowns & Commoners

#include "UI/SCIRLMapPage.h"
#include "UI/CIRLUIStyle.h"
#include "World/CIRLWorldMap.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/Actor.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "CIRLMapPage"

using namespace CIRLUIStyle;

namespace
{
	/** How many game km of map are visible top to bottom when the map opens */
	constexpr float OpeningViewGameKm = 8.f;

	/** Panning speed with keys or a stick: view heights per second */
	constexpr float PanSpeed = 0.8f;

	/** Zoom speed with keys or triggers: times per second */
	constexpr float ZoomSpeed = 2.5f;

	/** Height of your marker arrow on screen */
	constexpr double MarkerArrowHeight = 34.0;

	const TArray<FKey>& UpKeys()		{ static TArray<FKey> K = { EKeys::W, EKeys::Up }; return K; }
	const TArray<FKey>& DownKeys()		{ static TArray<FKey> K = { EKeys::S, EKeys::Down }; return K; }
	const TArray<FKey>& LeftKeys()		{ static TArray<FKey> K = { EKeys::A, EKeys::Left }; return K; }
	const TArray<FKey>& RightKeys()		{ static TArray<FKey> K = { EKeys::D, EKeys::Right }; return K; }
	const TArray<FKey>& ZoomInKeys()	{ static TArray<FKey> K = { EKeys::Equals, EKeys::Add, EKeys::PageUp }; return K; }
	const TArray<FKey>& ZoomOutKeys()	{ static TArray<FKey> K = { EKeys::Hyphen, EKeys::Subtract, EKeys::PageDown }; return K; }

	bool IsMapKey(const FKey& Key)
	{
		return UpKeys().Contains(Key) || DownKeys().Contains(Key) || LeftKeys().Contains(Key) || RightKeys().Contains(Key)
			|| ZoomInKeys().Contains(Key) || ZoomOutKeys().Contains(Key);
	}

	bool AnyHeld(const TSet<FKey>& Held, const TArray<FKey>& Keys)
	{
		for (const FKey& Key : Keys)
		{
			if (Held.Contains(Key))
			{
				return true;
			}
		}
		return false;
	}
}

void SCIRLMapPage::Construct(const FArguments& InArgs)
{
	Player = InArgs._Player;
	SetClipping(EWidgetClipping::ClipToBounds);

	Map.Reset(CIRLWorldMap::LoadWorldMap());
	Texture.Reset(Map ? Map->Texture.LoadSynchronous() : nullptr);
	if (Texture)
	{
		MapBrush.SetResourceObject(Texture.Get());
		MapBrush.ImageSize = FVector2D(Texture->GetSizeX(), Texture->GetSizeY());
		MapBrush.DrawAs = ESlateBrushDrawType::Image;
	}
	MarkerTexture.Reset(Map ? Map->PlayerMarker.LoadSynchronous() : nullptr);
	if (MarkerTexture)
	{
		MarkerBrush.SetResourceObject(MarkerTexture.Get());
		MarkerBrush.ImageSize = FVector2D(MarkerTexture->GetSizeX(), MarkerTexture->GetSizeY());
		MarkerBrush.DrawAs = ESlateBrushDrawType::Image;
	}
	if (Map)
	{
		Zoom = 1.f / FMath::Max(Map->UVPerGameKm.Y * OpeningViewGameKm, KINDA_SMALL_NUMBER);
	}
	CenterOnPlayer();

	RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SCIRLMapPage::Update));
}

void SCIRLMapPage::CenterOnPlayer()
{
	const AActor* You = Player.Get();
	if (Map && You)
	{
		ViewCenter = Map->GameKmToUV(CIRLWorldMap::ToGameKm(You->GetWorld(), You->GetActorLocation()));
		ClampView();
	}
}

FVector2D SCIRLMapPage::MapScreenSize() const
{
	const double Height = ViewSize.Y * Zoom;
	return FVector2D(Height * (Map ? Map->Aspect : 1.f), Height);
}

FVector2D SCIRLMapPage::MapTopLeft() const
{
	return ViewSize * 0.5 - ViewCenter * MapScreenSize();
}

void SCIRLMapPage::ClampView()
{
	// From the whole map down to about 1.5 screen pixels per map pixel
	const float TextureHeight = Texture ? Texture->GetSizeY() : 4096.f;
	const float MaxZoom = FMath::Max(1.5f * TextureHeight / FMath::Max(ViewSize.Y, 1.0), 2.f);
	Zoom = FMath::Clamp(Zoom, 1.f, MaxZoom);
	ViewCenter.X = FMath::Clamp(ViewCenter.X, 0.0, 1.0);
	ViewCenter.Y = FMath::Clamp(ViewCenter.Y, 0.0, 1.0);
}

void SCIRLMapPage::ZoomAt(float Factor, const FVector2D& LocalPoint)
{
	// Keep the map point under the cursor (or the view's middle) in place while zooming
	const FVector2D Before = (LocalPoint - MapTopLeft()) / MapScreenSize();
	Zoom *= Factor;
	ClampView();
	ViewCenter = Before - (LocalPoint - ViewSize * 0.5) / MapScreenSize();
	ClampView();
}

EActiveTimerReturnType SCIRLMapPage::Update(double InCurrentTime, float InDeltaTime)
{
	FVector2D Pan = StickPan * FVector2D(1.0, -1.0);
	Pan.Y -= AnyHeld(HeldKeys, UpKeys()) ? 1.0 : 0.0;
	Pan.Y += AnyHeld(HeldKeys, DownKeys()) ? 1.0 : 0.0;
	Pan.X -= AnyHeld(HeldKeys, LeftKeys()) ? 1.0 : 0.0;
	Pan.X += AnyHeld(HeldKeys, RightKeys()) ? 1.0 : 0.0;
	if (!Pan.IsNearlyZero())
	{
		ViewCenter += Pan.GetClampedToMaxSize(1.0) * (PanSpeed * ViewSize.Y * InDeltaTime) / MapScreenSize();
		ClampView();
	}

	const float ZoomInput = (AnyHeld(HeldKeys, ZoomInKeys()) ? 1.f : 0.f) - (AnyHeld(HeldKeys, ZoomOutKeys()) ? 1.f : 0.f)
		+ TriggerZoomIn - TriggerZoomOut;
	if (FMath::Abs(ZoomInput) > 0.05f)
	{
		ZoomAt(FMath::Pow(ZoomSpeed, ZoomInput * InDeltaTime), ViewSize * 0.5);
	}
	return EActiveTimerReturnType::Continue;
}

int32 SCIRLMapPage::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	ViewSize = FVector2D(AllottedGeometry.GetLocalSize());
	const FSlateBrush* White = FCoreStyle::Get().GetBrush("GenericWhiteBox");

	// Behind the map (seen when zoomed out or at the edges): dark, like the menu's cards
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), White,
		ESlateDrawEffect::None, FLinearColor(0.035f, 0.03f, 0.025f, 0.9f));

	if (!Map || !Texture)
	{
		const FSlateFontInfo MessageFont = Font(EFont::BodyItalic, 22.f);
		const FText Message = LOCTEXT("NoMap", "The map could not be found.");
		FSlateDrawElement::MakeText(OutDrawElements, LayerId + 1,
			AllottedGeometry.ToPaintGeometry(FVector2f(ViewSize), FSlateLayoutTransform(FVector2f(40.f, 40.f))),
			Message, MessageFont, ESlateDrawEffect::None, TextMuted());
		return LayerId + 1;
	}

	// The map picture
	const FVector2D MapSize = MapScreenSize();
	const FVector2D TopLeft = MapTopLeft();
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 1,
		AllottedGeometry.ToPaintGeometry(FVector2f(MapSize), FSlateLayoutTransform(FVector2f(TopLeft))), &MapBrush);

	// You: the marker arrow pointing where you face (+X north = up, +Y east = right), or a drawn dot and needle
	int32 TopLayer = LayerId + 1;
	if (const AActor* You = Player.Get())
	{
		const FVector2D UV = Map->GameKmToUV(CIRLWorldMap::ToGameKm(You->GetWorld(), You->GetActorLocation()));
		const FVector2D Spot = TopLeft + UV * MapSize;
		const float Angle = FMath::DegreesToRadians(You->GetActorRotation().Yaw);

		static const FSlateRoundedBoxBrush Dot(FLinearColor(0.72f, 0.08f, 0.06f), 9.f, FLinearColor(0.98f, 0.93f, 0.8f), 2.5f);
		static const FSlateRoundedBoxBrush NeedleEdge(FLinearColor(0.12f, 0.08f, 0.05f), 4.f);
		static const FSlateRoundedBoxBrush Needle(FLinearColor(0.72f, 0.08f, 0.06f), 2.5f);

		auto Box = [&](const FSlateBrush* Brush, const FVector2D& Size, int32 Layer)
		{
			FSlateDrawElement::MakeBox(OutDrawElements, Layer,
				AllottedGeometry.ToPaintGeometry(FVector2f(Size), FSlateLayoutTransform(FVector2f(Spot - Size * 0.5))), Brush);
		};
		auto RotatedBar = [&](const FSlateBrush* Brush, const FVector2D& Size, int32 Layer)
		{
			// A bar standing on the dot's centre, turned to face your heading
			const FGeometry Bar = AllottedGeometry.MakeChild(FVector2f(Size),
				FSlateLayoutTransform(FVector2f(Spot - FVector2D(Size.X * 0.5, Size.Y))),
				FSlateRenderTransform(FQuat2f(Angle)), FVector2f(0.5f, 1.f));
			FSlateDrawElement::MakeBox(OutDrawElements, Layer, Bar.ToPaintGeometry(), Brush);
		};

		if (MarkerTexture)
		{
			// The arrow turns around its middle, which sits on your spot
			const FVector2D ArrowSize = MarkerBrush.ImageSize * (MarkerArrowHeight / FMath::Max(MarkerBrush.ImageSize.Y, 1.0));
			const FGeometry Arrow = AllottedGeometry.MakeChild(FVector2f(ArrowSize),
				FSlateLayoutTransform(FVector2f(Spot - ArrowSize * 0.5)), FSlateRenderTransform(FQuat2f(Angle)), FVector2f(0.5f, 0.5f));
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 5, Arrow.ToPaintGeometry(), &MarkerBrush);
		}
		else
		{
			RotatedBar(&NeedleEdge, FVector2D(9.0, 28.0), LayerId + 3);
			RotatedBar(&Needle, FVector2D(5.0, 26.0), LayerId + 4);
			Box(&Dot, FVector2D(18.0, 18.0), LayerId + 5);
		}

		TopLayer = LayerId + 5;
	}

	// Thin gold frame around the view
	const FVector2f Size(ViewSize);
	TArray<FVector2f> Frame = { FVector2f(0.5f, 0.5f), FVector2f(Size.X - 0.5f, 0.5f), FVector2f(Size.X - 0.5f, Size.Y - 0.5f),
		FVector2f(0.5f, Size.Y - 0.5f), FVector2f(0.5f, 0.5f) };
	FSlateDrawElement::MakeLines(OutDrawElements, TopLayer + 1, AllottedGeometry.ToPaintGeometry(), Frame,
		ESlateDrawEffect::None, Gold() * FLinearColor(1.f, 1.f, 1.f, 0.6f), true, 1.f);
	return TopLayer + 1;
}

FReply SCIRLMapPage::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::SpaceBar || Key == EKeys::Gamepad_FaceButton_Top)
	{
		CenterOnPlayer();
		return FReply::Handled();
	}
	if (IsMapKey(Key))
	{
		HeldKeys.Add(Key);
		return FReply::Handled();
	}
	// Everything else (Q / E tabs, Esc, M...) is for the menu
	return FReply::Unhandled();
}

FReply SCIRLMapPage::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (HeldKeys.Remove(InKeyEvent.GetKey()) > 0)
	{
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SCIRLMapPage::OnAnalogValueChanged(const FGeometry& MyGeometry, const FAnalogInputEvent& InAnalogInputEvent)
{
	const FKey Key = InAnalogInputEvent.GetKey();
	const float Value = InAnalogInputEvent.GetAnalogValue();
	const float Dead = 0.2f;
	auto Shaped = [Dead](float V) { return FMath::Abs(V) < Dead ? 0.f : FMath::Sign(V) * (FMath::Abs(V) - Dead) / (1.f - Dead); };

	if (Key == EKeys::Gamepad_LeftX)
	{
		StickPan.X = Shaped(Value);
	}
	else if (Key == EKeys::Gamepad_LeftY)
	{
		StickPan.Y = Shaped(Value);
	}
	else if (Key == EKeys::Gamepad_RightTriggerAxis)
	{
		TriggerZoomIn = Shaped(Value);
	}
	else if (Key == EKeys::Gamepad_LeftTriggerAxis)
	{
		TriggerZoomOut = Shaped(Value);
	}
	else
	{
		return FReply::Unhandled();
	}
	// The stick moves the map here, not the menu's focus
	return FReply::Handled();
}

void SCIRLMapPage::OnFocusLost(const FFocusEvent& InFocusEvent)
{
	HeldKeys.Reset();
	StickPan = FVector2D::ZeroVector;
	TriggerZoomIn = TriggerZoomOut = 0.f;
}

FReply SCIRLMapPage::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bDragging = true;
		LastMouse = FVector2D(MouseEvent.GetScreenSpacePosition());
		return FReply::Handled().CaptureMouse(SharedThis(this)).SetUserFocus(SharedThis(this), EFocusCause::Mouse);
	}
	return FReply::Unhandled();
}

FReply SCIRLMapPage::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bDragging && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bDragging = false;
		return FReply::Handled().ReleaseMouseCapture();
	}
	return FReply::Unhandled();
}

FReply SCIRLMapPage::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!bDragging)
	{
		return FReply::Unhandled();
	}
	// Drag the paper: the map follows the mouse (screen movement converted to this widget's scale)
	const FVector2D Mouse = FVector2D(MouseEvent.GetScreenSpacePosition());
	const FVector2D Delta = (Mouse - LastMouse) / FMath::Max(MyGeometry.Scale, 0.01f);
	LastMouse = Mouse;
	ViewCenter -= Delta / MapScreenSize();
	ClampView();
	return FReply::Handled();
}

FReply SCIRLMapPage::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2D Local = FVector2D(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()));
	ZoomAt(FMath::Pow(1.25f, MouseEvent.GetWheelDelta()), Local);
	return FReply::Handled();
}

FCursorReply SCIRLMapPage::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	return FCursorReply::Cursor(bDragging ? EMouseCursor::GrabHandClosed : EMouseCursor::GrabHand);
}

#undef LOCTEXT_NAMESPACE
