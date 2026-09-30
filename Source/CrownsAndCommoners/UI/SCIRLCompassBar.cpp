// Crowns & Commoners

#include "UI/SCIRLCompassBar.h"
#include "UI/CIRLUIStyle.h"
#include "Animals/Horse.h"
#include "Characters/BallCharacter.h"
#include "Characters/AI/BallFighterController.h"
#include "Camera/PlayerCameraManager.h"
#include "EngineUtils.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

using namespace CIRLUIStyle;

namespace
{
	/** A horse further than this (and not the one you rode last) doesn't show */
	constexpr float NearbyHorseRange = 6000.f;

	/** Closer than this, no distance is written under a marker */
	constexpr float ShowDistanceBeyond = 800.f;

	constexpr float MarkerSize = 30.f;

	void DrawText(FSlateWindowElementList& Out, int32 Layer, const FGeometry& Geometry, const FString& Text, const FSlateFontInfo& Font,
		const FVector2D& Center, const FLinearColor& Color)
	{
		const FVector2D Size = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(Text, Font);
		FSlateDrawElement::MakeText(Out, Layer, Geometry.ToPaintGeometry(Size, FSlateLayoutTransform(Center - Size * 0.5f)), Text, Font,
			ESlateDrawEffect::None, Color);
	}

	void DrawBox(FSlateWindowElementList& Out, int32 Layer, const FGeometry& Geometry, const FSlateBrush* Brush, const FVector2D& TopLeft,
		const FVector2D& Size, const FLinearColor& Tint)
	{
		FSlateDrawElement::MakeBox(Out, Layer, Geometry.ToPaintGeometry(Size, FSlateLayoutTransform(TopLeft)), Brush, ESlateDrawEffect::None, Tint);
	}
}

void SCIRLCompassBar::Construct(const FArguments& InArgs)
{
	Owner = InArgs._Owner;
	SetVisibility(EVisibility::HitTestInvisible);
}

FVector2D SCIRLCompassBar::ComputeDesiredSize(float) const
{
	// Room under the strip for the markers' distances
	return FVector2D(Width, StripHeight + 34.f);
}

void SCIRLCompassBar::GatherMarkers(TArray<FMarker>& OutMarkers) const
{
	const APlayerController* PC = Owner.Get();
	const ABallCharacter* Player = PC ? Cast<ABallCharacter>(PC->GetPawn()) : nullptr;
	if (!Player || Player->IsDead())
	{
		return;
	}
	UWorld* World = PC->GetWorld();

	// Your horse: the one you rode last, or else the nearest free one close by (not while you are on it)
	const AHorse* Horse = Player->GetLastMount();
	if (!Horse)
	{
		float Best = NearbyHorseRange;
		for (TActorIterator<AHorse> It(World); It; ++It)
		{
			const float Distance = FVector::Dist2D(It->GetActorLocation(), Player->GetActorLocation());
			if (It->CanBeMounted() && Distance < Best)
			{
				Horse = *It;
				Best = Distance;
			}
		}
	}
	if (Horse && Horse != Player->GetMount())
	{
		FMarker& Marker = OutMarkers.AddDefaulted_GetRef();
		Marker.Icon = MapMarker(TEXT("marker_horse"));
		Marker.Location = Horse->GetActorLocation();
		Marker.bShowDistance = true;
	}

	// Anyone fighting you
	for (TActorIterator<ABallFighterController> It(World); It; ++It)
	{
		const ABallCharacter* Fighter = Cast<ABallCharacter>(It->GetPawn());
		if (Fighter && !Fighter->IsDead() && It->GetTarget() == Player)
		{
			FMarker& Marker = OutMarkers.AddDefaulted_GetRef();
			Marker.Icon = MapMarker(TEXT("marker_enemy"));
			Marker.Location = Fighter->GetActorLocation();
		}
	}
}

int32 SCIRLCompassBar::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const APlayerController* PC = Owner.Get();
	if (!PC || !PC->PlayerCameraManager)
	{
		return LayerId;
	}

	// Which way the camera looks: yaw 0 is north (+X), 90 is east (+Y)
	const FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	const float Heading = PC->PlayerCameraManager->GetCameraRotation().Yaw;
	const float Half = Width * 0.5f;
	const float PixelsPerDegree = Width / FieldOfView;
	// Everything fades out towards the strip's ends
	auto EdgeFade = [Half](float X) { return FMath::Clamp(1.f - FMath::Pow(FMath::Abs(X - Half) / Half, 3.f), 0.f, 1.f); };
	auto ToX = [Half, PixelsPerDegree, Heading](float Bearing) { return Half + FMath::FindDeltaAngleDegrees(Heading, Bearing) * PixelsPerDegree; };

	const FSlateBrush* White = FCoreStyle::Get().GetBrush("WhiteBrush");
	const FLinearColor Shade(0.03f, 0.028f, 0.024f, 0.55f);
	const FLinearColor Line = Gold();

	// The strip: a dark band fading at both ends, with thin gold lines above and below
	constexpr int32 Slices = 32;
	const float SliceWidth = Width / Slices;
	for (int32 Slice = 0; Slice < Slices; ++Slice)
	{
		const float X = Slice * SliceWidth;
		const float Fade = EdgeFade(X + SliceWidth * 0.5f);
		DrawBox(OutDrawElements, LayerId, AllottedGeometry, White, FVector2D(X, 0.f), FVector2D(SliceWidth + 0.5f, StripHeight), Shade * FLinearColor(1.f, 1.f, 1.f, Fade));
		const FLinearColor LineColor = Line * FLinearColor(1.f, 1.f, 1.f, 0.8f * Fade);
		DrawBox(OutDrawElements, LayerId + 1, AllottedGeometry, White, FVector2D(X, 0.f), FVector2D(SliceWidth + 0.5f, 1.f), LineColor);
		DrawBox(OutDrawElements, LayerId + 1, AllottedGeometry, White, FVector2D(X, StripHeight - 1.f), FVector2D(SliceWidth + 0.5f, 1.f), LineColor);
	}

	// Ticks every 15 degrees; the directions every 45, the four great ones larger
	static const TCHAR* Names[] = { TEXT("N"), TEXT("NE"), TEXT("E"), TEXT("SE"), TEXT("S"), TEXT("SW"), TEXT("W"), TEXT("NW") };
	const FSlateFontInfo Cardinal = Font(EFont::TitleBold, 17.f);
	const FSlateFontInfo Between = Font(EFont::Title, 12.f);
	for (int32 Degrees = 0; Degrees < 360; Degrees += 15)
	{
		const float X = ToX(static_cast<float>(Degrees));
		const float Fade = EdgeFade(X);
		if (X < 0.f || X > Width || Fade <= 0.01f)
		{
			continue;
		}
		if (Degrees % 45 != 0)
		{
			DrawBox(OutDrawElements, LayerId + 1, AllottedGeometry, White, FVector2D(X - 0.5f, StripHeight - 8.f), FVector2D(1.f, 5.f),
				TextMuted() * FLinearColor(1.f, 1.f, 1.f, Fade));
			continue;
		}
		const int32 Index = Degrees / 45;
		const bool bGreat = Index % 2 == 0;
		// North stands out in bright gold
		const FLinearColor Color = (Index == 0 ? GoldBright() : (bGreat ? Text() : TextMuted())) * FLinearColor(1.f, 1.f, 1.f, Fade);
		DrawText(OutDrawElements, LayerId + 2, AllottedGeometry, Names[Index], bGreat ? Cardinal : Between, FVector2D(X, StripHeight * 0.5f), Color);
	}

	// Markers
	TArray<FMarker> Markers;
	GatherMarkers(Markers);
	const FSlateFontInfo DistanceFont = Font(EFont::BodySemiBold, 13.f);
	const float Edge = MarkerSize * 0.5f + 4.f;
	for (const FMarker& Marker : Markers)
	{
		if (!Marker.Icon)
		{
			continue;
		}
		const FVector To = Marker.Location - CameraLocation;
		float X = ToX(To.Rotation().Yaw);
		const bool bOutside = X < Edge || X > Width - Edge;
		if (bOutside && !Marker.bClampToEnds)
		{
			continue;
		}
		X = FMath::Clamp(X, Edge, Width - Edge);
		// Waiting at the end of the strip: a little fainter
		const FLinearColor Tint(1.f, 1.f, 1.f, bOutside ? 0.65f : 1.f);
		DrawBox(OutDrawElements, LayerId + 3, AllottedGeometry, Marker.Icon, FVector2D(X - MarkerSize * 0.5f, (StripHeight - MarkerSize) * 0.5f),
			FVector2D(MarkerSize), Tint);

		const float Distance = To.Size2D();
		if (Marker.bShowDistance && Distance > ShowDistanceBeyond)
		{
			DrawText(OutDrawElements, LayerId + 3, AllottedGeometry, FString::Printf(TEXT("%d m"), FMath::RoundToInt(Distance / 100.f)), DistanceFont,
				FVector2D(X, StripHeight + 11.f), Text() * Tint);
		}
	}

	// Where you look: a small gold notch in the middle, under the strip
	DrawBox(OutDrawElements, LayerId + 4, AllottedGeometry, White, FVector2D(Half - 1.f, StripHeight - 3.f), FVector2D(2.f, 9.f), GoldBright());
	DrawBox(OutDrawElements, LayerId + 4, AllottedGeometry, White, FVector2D(Half - 4.f, StripHeight + 5.f), FVector2D(8.f, 2.f), GoldBright());

	return LayerId + 4;
}
