// CountriesIRL 3D Game

#include "UI/CIRLUIStyle.h"
#include "Engine/FontFace.h"
#include "Fonts/CompositeFont.h"
#include "Styling/CoreStyle.h"

namespace CIRLUIStyle
{
	namespace
	{
		/** Colours are picked in sRGB (like a paint program) and converted to Slate's linear colour */
		FLinearColor SRGB(uint8 R, uint8 G, uint8 B, float Alpha = 1.f)
		{
			FLinearColor Color(FColor(R, G, B));
			Color.A = Alpha;
			return Color;
		}

		/** A box with slightly rounded corners and an optional border line */
		FSlateBrush MakeBox(const FLinearColor& Fill, const FLinearColor& Outline, float OutlineWidth, float CornerRadius = 3.f)
		{
			FSlateBrush Brush;
			Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
			Brush.TintColor = Fill;
			Brush.OutlineSettings = FSlateBrushOutlineSettings(CornerRadius, Outline, OutlineWidth);
			return Brush;
		}

		TSharedPtr<const FCompositeFont> LoadComposite(const TCHAR* FaceAsset)
		{
			// Font faces stay loaded for the whole game once a menu has used them
			UFontFace* Face = LoadObject<UFontFace>(nullptr, FaceAsset);
			if (!Face)
			{
				return nullptr;
			}
			Face->AddToRoot();

			TSharedRef<FCompositeFont> Composite = MakeShared<FCompositeFont>();
			FTypefaceEntry& Entry = Composite->DefaultTypeface.Fonts.AddDefaulted_GetRef();
			Entry.Name = TEXT("Regular");
			Entry.Font = FFontData(Face);
			return Composite;
		}
	}

	FSlateFontInfo Font(EFont Face, float Size)
	{
		static const TCHAR* Assets[] =
		{
			TEXT("/Game/CountriesIRL/UI/Fonts/FF_Cinzel_Regular.FF_Cinzel_Regular"),
			TEXT("/Game/CountriesIRL/UI/Fonts/FF_Cinzel_SemiBold.FF_Cinzel_SemiBold"),
			TEXT("/Game/CountriesIRL/UI/Fonts/FF_Cinzel_Bold.FF_Cinzel_Bold"),
			TEXT("/Game/CountriesIRL/UI/Fonts/FF_EBGaramond_Regular.FF_EBGaramond_Regular"),
			TEXT("/Game/CountriesIRL/UI/Fonts/FF_EBGaramond_SemiBold.FF_EBGaramond_SemiBold"),
			TEXT("/Game/CountriesIRL/UI/Fonts/FF_EBGaramond_Italic.FF_EBGaramond_Italic"),
		};
		static TSharedPtr<const FCompositeFont> Composites[UE_ARRAY_COUNT(Assets)];

		const int32 Index = static_cast<int32>(Face);
		if (!Composites[Index])
		{
			Composites[Index] = LoadComposite(Assets[Index]);
		}
		if (!Composites[Index])
		{
			// Missing asset: fall back to the engine font rather than drawing nothing
			return FCoreStyle::GetDefaultFontStyle("Regular", Size);
		}
		return FSlateFontInfo(Composites[Index], Size);
	}

	FLinearColor Gold() { return SRGB(150, 122, 74); }
	FLinearColor GoldBright() { return SRGB(227, 194, 127); }
	FLinearColor Text() { return SRGB(217, 205, 180); }
	FLinearColor TextMuted() { return SRGB(140, 131, 112); }

	const FSlateBrush* Panel()
	{
		static const FSlateBrush Brush = MakeBox(SRGB(20, 18, 15, 0.97f), Gold(), 1.5f, 4.f);
		return &Brush;
	}

	const FSlateBrush* Card()
	{
		static const FSlateBrush Brush = MakeBox(SRGB(28, 25, 21, 0.95f), SRGB(150, 122, 74, 0.45f), 1.f);
		return &Brush;
	}

	const FSlateBrush* ButtonNormal()
	{
		static const FSlateBrush Brush = MakeBox(SRGB(28, 25, 21, 0.9f), SRGB(150, 122, 74, 0.35f), 1.f);
		return &Brush;
	}

	const FSlateBrush* ButtonHovered()
	{
		static const FSlateBrush Brush = MakeBox(SRGB(58, 47, 28, 0.95f), GoldBright(), 1.5f);
		return &Brush;
	}

	const FSlateBrush* ButtonPressed()
	{
		static const FSlateBrush Brush = MakeBox(SRGB(80, 64, 36, 0.95f), GoldBright(), 1.5f);
		return &Brush;
	}

	const FSlateBrush* KeyCap()
	{
		static const FSlateBrush Brush = MakeBox(SRGB(14, 13, 11, 0.95f), Text(), 1.f, 2.f);
		return &Brush;
	}

	const FSlateBrush* GoldFill()
	{
		static const FSlateBrush Brush = MakeBox(Gold(), FLinearColor::Transparent, 0.f, 0.f);
		return &Brush;
	}

	const FSlateBrush* ScreenDim()
	{
		static const FSlateBrush Brush = MakeBox(FLinearColor(0.f, 0.f, 0.f, 0.45f), FLinearColor::Transparent, 0.f, 0.f);
		return &Brush;
	}
}
