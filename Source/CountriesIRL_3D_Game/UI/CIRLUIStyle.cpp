// CountriesIRL 3D Game

#include "UI/CIRLUIStyle.h"
#include "Engine/FontFace.h"
#include "Fonts/CompositeFont.h"
#include "Styling/CoreStyle.h"
#include "Engine/Texture2D.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"

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

		/** Brush drawing a texture from /Game/CountriesIRL/UI/Textures (kept loaded for the whole game) */
		FSlateBrush MakeImage(const TCHAR* TextureAsset, const FVector2D& Size, ESlateBrushTileType::Type Tiling = ESlateBrushTileType::NoTile)
		{
			FSlateBrush Brush;
			Brush.DrawAs = ESlateBrushDrawType::Image;
			Brush.ImageSize = Size;
			Brush.Tiling = Tiling;
			if (UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, TextureAsset))
			{
				Texture->AddToRoot();
				Brush.SetResourceObject(Texture);
			}
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

	const FSlateBrush* PanelTexture()
	{
		// One tile covers ~420 px, so the leather grain reads without repeating visibly
		static const FSlateBrush Brush = MakeImage(TEXT("/Game/CountriesIRL/UI/Textures/T_panel_texture.T_panel_texture"), FVector2D(420.f), ESlateBrushTileType::Both);
		return &Brush;
	}

	const FSlateBrush* CornerOrnament()
	{
		static const FSlateBrush Brush = MakeImage(TEXT("/Game/CountriesIRL/UI/Textures/T_corner_ornament.T_corner_ornament"), FVector2D(118.f));
		return &Brush;
	}

	const FSlateBrush* TitleBackground()
	{
		static const FSlateBrush Brush = MakeImage(TEXT("/Game/CountriesIRL/UI/Textures/T_menu_background.T_menu_background"), FVector2D(1536.f, 1024.f));
		return &Brush;
	}

	int32 LoadingPaintingCount()
	{
		return 4;
	}

	const FSlateBrush* LoadingPainting(int32 Index)
	{
		static const FSlateBrush Brushes[] =
		{
			MakeImage(TEXT("/Game/CountriesIRL/UI/Textures/T_loading_01.T_loading_01"), FVector2D(1536.f, 1024.f)),
			MakeImage(TEXT("/Game/CountriesIRL/UI/Textures/T_loading_02.T_loading_02"), FVector2D(1536.f, 1024.f)),
			MakeImage(TEXT("/Game/CountriesIRL/UI/Textures/T_loading_03.T_loading_03"), FVector2D(1536.f, 1024.f)),
			MakeImage(TEXT("/Game/CountriesIRL/UI/Textures/T_loading_04.T_loading_04"), FVector2D(1536.f, 1024.f)),
		};
		static_assert(UE_ARRAY_COUNT(Brushes) == 4, "Keep LoadingPaintingCount() in step");
		return &Brushes[FMath::Clamp(Index, 0, 3)];
	}

	const FSlateBrush* GradientLeft()
	{
		static const FSlateBrush Brush = MakeImage(TEXT("/Game/CountriesIRL/UI/Textures/T_gradient_left.T_gradient_left"), FVector2D(512.f, 4.f));
		return &Brush;
	}

	const FSlateBrush* GradientBottom()
	{
		static const FSlateBrush Brush = MakeImage(TEXT("/Game/CountriesIRL/UI/Textures/T_gradient_bottom.T_gradient_bottom"), FVector2D(4.f, 512.f));
		return &Brush;
	}

	TSharedRef<SWidget> MakeOrnatePanel(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		// The ornament's own gold lines run ~10% in from its edges: pulling it out by that much
		// lays its lines over the panel's border, so the corners grow out of the frame
		const float Size = CornerOrnament()->ImageSize.X;
		const float Overhang = Size * 0.1f;
		auto Corner = [Size](float ScaleX, float ScaleY)
		{
			return SNew(SBox)
				.WidthOverride(Size)
				.HeightOverride(Size)
				[
					SNew(SImage)
					.Image(CornerOrnament())
					.RenderTransform(FSlateRenderTransform(FScale2D(ScaleX, ScaleY)))
					.RenderTransformPivot(FVector2D(0.5f, 0.5f))
				];
		};

		static const FSlateBrush BorderOnly = MakeBox(FLinearColor::Transparent, Gold(), 1.5f, 4.f);

		return SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage).Image(PanelTexture()).ColorAndOpacity(FLinearColor(0.85f, 0.85f, 0.85f, 0.97f))
			]
			+ SOverlay::Slot()
			[
				SNew(SBorder).BorderImage(&BorderOnly).Padding(Padding)[Content]
			]
			+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(FMargin(-Overhang, -Overhang, 0.f, 0.f))[Corner(1.f, 1.f)]
			+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(FMargin(0.f, -Overhang, -Overhang, 0.f))[Corner(-1.f, 1.f)]
			+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(FMargin(-Overhang, 0.f, 0.f, -Overhang))[Corner(1.f, -1.f)]
			+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom).Padding(FMargin(0.f, 0.f, -Overhang, -Overhang))[Corner(-1.f, -1.f)];
	}
}
