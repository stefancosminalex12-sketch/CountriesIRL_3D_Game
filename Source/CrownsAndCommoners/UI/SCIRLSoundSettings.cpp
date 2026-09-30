// Crowns & Commoners

#include "UI/SCIRLSoundSettings.h"
#include "UI/CIRLUIStyle.h"
#include "Audio/CIRLAudioSubsystem.h"
#include "Styling/SlateTypes.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "CIRLSoundSettings"

using namespace CIRLUIStyle;

void SCIRLSoundSettings::Construct(const FArguments& InArgs)
{
	Audio = InArgs._Audio;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 6.f))
		[
			MakeRow(LOCTEXT("Master", "Master"), ECIRLVolume::Master, InArgs._SliderWidth, InArgs._LabelWidth, InArgs._FontSize)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 6.f))
		[
			MakeRow(LOCTEXT("Music", "Music"), ECIRLVolume::Music, InArgs._SliderWidth, InArgs._LabelWidth, InArgs._FontSize)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 6.f))
		[
			MakeRow(LOCTEXT("GameMusic", "In-Game Music"), ECIRLVolume::GameMusic, InArgs._SliderWidth, InArgs._LabelWidth, InArgs._FontSize)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 6.f))
		[
			MakeRow(LOCTEXT("Effects", "SFX"), ECIRLVolume::Effects, InArgs._SliderWidth, InArgs._LabelWidth, InArgs._FontSize)
		]
	];
}

SCIRLSoundSettings::~SCIRLSoundSettings()
{
	Save();
}

TSharedRef<SWidget> SCIRLSoundSettings::MakeRow(const FText& Label, ECIRLVolume Which, float SliderWidth, float LabelWidth, float FontSize)
{
	TWeakObjectPtr<UCIRLAudioSubsystem> WeakAudio = Audio;
	auto Value = [WeakAudio, Which]()
	{
		return WeakAudio.IsValid() ? WeakAudio->GetVolume(Which) : 1.f;
	};

	TSharedPtr<SSlider> Slider;
	SAssignNew(Slider, SSlider)
		.Style(&SliderStyle())
		.Value_Lambda(Value)
		.StepSize(0.05f)
		.IsFocusable(true)
		.RequiresControllerLock(false)	// left/right move the slider straight away, up/down go to the next one
		.OnValueChanged_Lambda([WeakAudio, Which](float NewValue)
		{
			if (WeakAudio.IsValid())
			{
				WeakAudio->SetVolume(Which, NewValue);
			}
		})
		.OnMouseCaptureEnd_Lambda([this]() { Save(); })
		.OnControllerCaptureEnd_Lambda([this]() { Save(); });

	if (!FirstSlider.IsValid())
	{
		FirstSlider = Slider;
	}

	// The row's name lights up while its slider is hovered or has keyboard/controller focus
	TWeakPtr<SSlider> WeakSlider = Slider;
	auto IsLit = [WeakSlider]()
	{
		const TSharedPtr<SSlider> Pinned = WeakSlider.Pin();
		return Pinned.IsValid() && (Pinned->IsHovered() || Pinned->HasKeyboardFocus());
	};

	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(SBox).WidthOverride(LabelWidth)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(Font(EFont::Title, FontSize))
				.ColorAndOpacity_Lambda([IsLit]() { return FSlateColor(IsLit() ? GoldBright() : Text()); })
				.ShadowOffset(FVector2D(1.f, 1.f))
				.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.8f))
			]
		]

		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(SBox).WidthOverride(SliderWidth).HeightOverride(34.f)
			[
				Slider.ToSharedRef()
			]
		]

		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(16.f, 0.f, 0.f, 0.f))
		[
			SNew(SBox).WidthOverride(FontSize * 3.f)
			[
				SNew(STextBlock)
				.Text_Lambda([Value]() { return FText::Format(LOCTEXT("Percent", "{0}%"), FText::AsNumber(FMath::RoundToInt(Value() * 100.f))); })
				.Font(Font(EFont::Body, FontSize * 0.9f))
				.ColorAndOpacity(TextMuted())
			]
		];
}

TSharedPtr<SWidget> SCIRLSoundSettings::GetFirstFocus() const
{
	return FirstSlider;
}

void SCIRLSoundSettings::Save()
{
	if (Audio.IsValid())
	{
		Audio->SaveVolumes();
	}
}

#undef LOCTEXT_NAMESPACE
