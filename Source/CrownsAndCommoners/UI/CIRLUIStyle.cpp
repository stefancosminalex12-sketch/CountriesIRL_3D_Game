// Crowns & Commoners

#include "UI/CIRLUIStyle.h"
#include "Engine/FontFace.h"
#include "Fonts/CompositeFont.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "Engine/Texture2D.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "UI/SCIRLButton.h"

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

		/** Brush drawing a texture from /Game/CrownsAndCommoners/UI/Textures (kept loaded for the whole game) */
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

		/** Like MakeImage, for art that may not exist yet: nullptr (quietly) until the texture has been imported */
		const FSlateBrush* OptionalImage(const TCHAR* Name, const FVector2D& Size)
		{
			static TMap<FString, TUniquePtr<FSlateBrush>> Brushes;
			if (const TUniquePtr<FSlateBrush>* Found = Brushes.Find(Name))
			{
				return Found->Get();
			}
			const FString Path = FString::Printf(TEXT("/Game/CrownsAndCommoners/UI/Textures/%s.%s"), Name, Name);
			TUniquePtr<FSlateBrush> Brush;
			if (UTexture2D* Texture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet)))
			{
				Texture->AddToRoot();
				Brush = MakeUnique<FSlateBrush>();
				Brush->DrawAs = ESlateBrushDrawType::Image;
				Brush->ImageSize = Size;
				Brush->SetResourceObject(Texture);
			}
			return Brushes.Add(Name, MoveTemp(Brush)).Get();
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
			TEXT("/Game/CrownsAndCommoners/UI/Fonts/FF_Cinzel_Regular.FF_Cinzel_Regular"),
			TEXT("/Game/CrownsAndCommoners/UI/Fonts/FF_Cinzel_SemiBold.FF_Cinzel_SemiBold"),
			TEXT("/Game/CrownsAndCommoners/UI/Fonts/FF_Cinzel_Bold.FF_Cinzel_Bold"),
			TEXT("/Game/CrownsAndCommoners/UI/Fonts/FF_EBGaramond_Regular.FF_EBGaramond_Regular"),
			TEXT("/Game/CrownsAndCommoners/UI/Fonts/FF_EBGaramond_SemiBold.FF_EBGaramond_SemiBold"),
			TEXT("/Game/CrownsAndCommoners/UI/Fonts/FF_EBGaramond_Italic.FF_EBGaramond_Italic"),
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

	const FSlateBrush* SlotSelected()
	{
		static const FSlateBrush Brush = MakeBox(SRGB(28, 25, 21, 0.9f), SRGB(227, 194, 127, 0.7f), 1.f);
		return &Brush;
	}

	const FSlateBrush* TabActive()
	{
		static const FSlateBrush Brush = MakeBox(SRGB(52, 43, 28, 0.85f), SRGB(150, 122, 74, 0.8f), 1.f, 2.f);
		return &Brush;
	}

	const FSlateBrush* NoBrush()
	{
		static const FSlateNoResource Brush;
		return &Brush;
	}

	const FSlateBrush* DollRing()
	{
		// White ring image (Art/UI/doll_ring.png, a double gold line), tinted worn gold
		static const FSlateBrush Brush = []
		{
			FSlateBrush B = MakeImage(TEXT("/Game/CrownsAndCommoners/UI/Textures/T_doll_ring.T_doll_ring"), FVector2D(512.f));
			B.TintColor = SRGB(150, 122, 74, 0.55f);
			return B;
		}();
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
		static const FSlateBrush Brush = MakeImage(TEXT("/Game/CrownsAndCommoners/UI/Textures/T_panel_texture.T_panel_texture"), FVector2D(420.f), ESlateBrushTileType::Both);
		return &Brush;
	}

	const FSlateBrush* CornerOrnament()
	{
		static const FSlateBrush Brush = MakeImage(TEXT("/Game/CrownsAndCommoners/UI/Textures/T_corner_ornament.T_corner_ornament"), FVector2D(118.f));
		return &Brush;
	}

	const FSlateBrush* TitleBackground()
	{
		static const FSlateBrush Brush = MakeImage(TEXT("/Game/CrownsAndCommoners/UI/Textures/T_menu_background.T_menu_background"), FVector2D(1536.f, 1024.f));
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
			MakeImage(TEXT("/Game/CrownsAndCommoners/UI/Textures/T_loading_01.T_loading_01"), FVector2D(1536.f, 1024.f)),
			MakeImage(TEXT("/Game/CrownsAndCommoners/UI/Textures/T_loading_02.T_loading_02"), FVector2D(1536.f, 1024.f)),
			MakeImage(TEXT("/Game/CrownsAndCommoners/UI/Textures/T_loading_03.T_loading_03"), FVector2D(1536.f, 1024.f)),
			MakeImage(TEXT("/Game/CrownsAndCommoners/UI/Textures/T_loading_04.T_loading_04"), FVector2D(1536.f, 1024.f)),
		};
		static_assert(UE_ARRAY_COUNT(Brushes) == 4, "Keep LoadingPaintingCount() in step");
		return &Brushes[FMath::Clamp(Index, 0, 3)];
	}

	const FSlateBrush* Vignette()
	{
		static const FSlateBrush Brush = MakeImage(TEXT("/Game/CrownsAndCommoners/UI/Textures/T_vignette.T_vignette"), FVector2D(1024.f, 1024.f));
		return &Brush;
	}

	const FSlateBrush* DeathBackground()
	{
		return OptionalImage(TEXT("T_death_background"), FVector2D(1536.f, 1024.f));
	}

	const FSlateBrush* DeathEmblem()
	{
		return OptionalImage(TEXT("T_death_emblem"), FVector2D(1024.f, 1024.f));
	}

	const FSlateBrush* DeathBlood()
	{
		return OptionalImage(TEXT("T_death_blood"), FVector2D(1536.f, 1024.f));
	}

	const FSlateBrush* DeathBloodDrips()
	{
		return OptionalImage(TEXT("T_death_blood_drips"), FVector2D(1536.f, 1024.f));
	}

	const FSlateBrush* GradientLeft()
	{
		static const FSlateBrush Brush = MakeImage(TEXT("/Game/CrownsAndCommoners/UI/Textures/T_gradient_left.T_gradient_left"), FVector2D(512.f, 4.f));
		return &Brush;
	}

	const FSlateBrush* GradientBottom()
	{
		static const FSlateBrush Brush = MakeImage(TEXT("/Game/CrownsAndCommoners/UI/Textures/T_gradient_bottom.T_gradient_bottom"), FVector2D(4.f, 512.f));
		return &Brush;
	}

	const FSlateBrush* MapMarker(FName Name)
	{
		static TMap<FName, TUniquePtr<FSlateBrush>> Markers;
		if (const TUniquePtr<FSlateBrush>* Found = Markers.Find(Name))
		{
			return Found->Get();
		}
		const FString Path = FString::Printf(TEXT("/Game/CrownsAndCommoners/UI/Markers/T_%s.T_%s"), *Name.ToString(), *Name.ToString());
		TUniquePtr<FSlateBrush> Brush;
		if (UTexture2D* Texture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet)))
		{
			Texture->AddToRoot();
			Brush = MakeUnique<FSlateBrush>();
			Brush->DrawAs = ESlateBrushDrawType::Image;
			Brush->ImageSize = FVector2D(256.f);
			Brush->SetResourceObject(Texture);
		}
		return Markers.Add(Name, MoveTemp(Brush)).Get();
	}

	const FSlateBrush* ItemIcon(FName Name)
	{
		// Loaded on first use; icons that don't exist yet are remembered as missing (no repeated lookups)
		static TMap<FName, TUniquePtr<FSlateBrush>> Icons;
		if (const TUniquePtr<FSlateBrush>* Found = Icons.Find(Name))
		{
			return Found->Get();
		}

		const FString Path = FString::Printf(TEXT("/Game/CrownsAndCommoners/UI/Icons/T_%s.T_%s"), *Name.ToString(), *Name.ToString());
		UTexture2D* Texture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet));
		TUniquePtr<FSlateBrush> Brush;
		if (Texture)
		{
			Texture->AddToRoot();
			Brush = MakeUnique<FSlateBrush>();
			Brush->DrawAs = ESlateBrushDrawType::Image;
			Brush->ImageSize = FVector2D(512.f);
			Brush->SetResourceObject(Texture);
		}
		return Icons.Add(Name, MoveTemp(Brush)).Get();
	}

	const FButtonStyle& PlainButtonStyle()
	{
		static const FButtonStyle Style = []
		{
			FButtonStyle S;
			const FSlateNoResource None;
			S.SetNormal(None).SetHovered(None).SetPressed(None).SetDisabled(None);
			S.SetNormalPadding(FMargin(0.f)).SetPressedPadding(FMargin(0.f));
			return S;
		}();
		return Style;
	}

	const FSliderStyle& SliderStyle()
	{
		static const FSliderStyle Style = []
		{
			auto Handle = [](const FLinearColor& Fill)
			{
				FSlateBrush Brush = MakeBox(Fill, SRGB(20, 18, 15), 1.f, 3.f);
				Brush.ImageSize = FVector2D(14.f, 26.f);
				return Brush;
			};
			FSliderStyle S;
			S.SetNormalBarImage(MakeBox(SRGB(46, 41, 33), SRGB(150, 122, 74, 0.5f), 1.f, 2.f))
				.SetHoveredBarImage(MakeBox(SRGB(58, 50, 38), SRGB(227, 194, 127, 0.8f), 1.f, 2.f))
				.SetDisabledBarImage(MakeBox(SRGB(36, 33, 28), SRGB(150, 122, 74, 0.3f), 1.f, 2.f))
				.SetNormalThumbImage(Handle(Gold()))
				.SetHoveredThumbImage(Handle(GoldBright()))
				.SetDisabledThumbImage(Handle(TextMuted()))
				.SetBarThickness(6.f);
			return S;
		}();
		return Style;
	}

	TSharedRef<SWidget> MakeMenuButton(const FText& Label, FOnClicked OnClicked, TSharedPtr<SButton>* OutButton, bool bCentered)
	{
		TSharedPtr<SCIRLButton> Button;
		SAssignNew(Button, SCIRLButton)
		.ButtonStyle(&PlainButtonStyle())
		.OnClicked(OnClicked);

		TWeakPtr<SCIRLButton> Weak = Button;
		Button->SetContent(
			SNew(SBorder)
			.HAlign(bCentered ? HAlign_Center : HAlign_Fill)
			.BorderImage_Lambda([Weak]()
			{
				const TSharedPtr<SCIRLButton> Pinned = Weak.Pin();
				if (!Pinned.IsValid() || !Pinned->IsHighlighted())
				{
					return ButtonNormal();
				}
				return Pinned->IsPressed() ? ButtonPressed() : ButtonHovered();
			})
			.Padding(FMargin(24.f, 10.f))
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(Font(EFont::Title, 21.f))
				.ColorAndOpacity_Lambda([Weak]()
				{
					const TSharedPtr<SCIRLButton> Pinned = Weak.Pin();
					return FSlateColor(Pinned.IsValid() && Pinned->IsHighlighted() ? GoldBright() : Text());
				})
			]);

		if (OutButton)
		{
			*OutButton = Button;
		}
		return SNew(SBox).WidthOverride(360.f)[Button.ToSharedRef()];
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
