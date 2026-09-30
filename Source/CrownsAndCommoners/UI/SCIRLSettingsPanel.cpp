// Crowns & Commoners

#include "UI/SCIRLSettingsPanel.h"
#include "UI/SCIRLButton.h"
#include "UI/SCIRLSoundSettings.h"
#include "UI/CIRLUIStyle.h"
#include "Audio/CIRLAudioSubsystem.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "CIRLSettings"

using namespace CIRLUIStyle;

namespace
{
	FText CategoryName(ECIRLSettingsCategory Category)
	{
		switch (Category)
		{
		case ECIRLSettingsCategory::Audio:	return LOCTEXT("Audio", "Audio");
		default:							return FText::GetEmpty();
		}
	}
}

void SCIRLSettingsPanel::Construct(const FArguments& InArgs)
{
	const float FontSize = InArgs._FontSize;

	// One page per section, in the order of ECIRLSettingsCategory
	SAssignNew(Sections, SWidgetSwitcher)
	+ SWidgetSwitcher::Slot()
	[
		SAssignNew(SoundSettings, SCIRLSoundSettings)
		.Audio(InArgs._Audio)
		.FontSize(FontSize)
		.SliderWidth(InArgs._SliderWidth)
		.LabelWidth(InArgs._LabelWidth)
	];

	TSharedRef<SVerticalBox> List = SNew(SVerticalBox);
	for (int32 Index = 0; Index < static_cast<int32>(ECIRLSettingsCategory::Count); ++Index)
	{
		List->AddSlot().AutoHeight().HAlign(HAlign_Left).Padding(FMargin(0.f, 2.f))
		[
			MakeCategoryButton(static_cast<ECIRLSettingsCategory>(Index), FontSize)
		];
	}

	ChildSlot
	[
		SNew(SHorizontalBox)

		// Sections
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SBox).MinDesiredWidth(FontSize * 7.f)
			[
				List
			]
		]

		// Thin gold line between the list and the options
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(18.f, 4.f, 28.f, 4.f))
		[
			SNew(SBox).WidthOverride(1.f)
			[
				SNew(SImage).Image(GoldFill()).ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.45f))
			]
		]

		// The open section: its name, then its options
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 0.f, 0.f, 12.f))
			[
				SNew(STextBlock)
				.Text_Lambda([this]() { return CategoryName(Current); })
				.Font(Font(EFont::BodyItalic, FontSize + 2.f))
				.ColorAndOpacity(TextMuted())
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				Sections.ToSharedRef()
			]
		]
	];
}

TSharedRef<SWidget> SCIRLSettingsPanel::MakeCategoryButton(ECIRLSettingsCategory Category, float FontSize)
{
	TSharedPtr<SCIRLButton> Button;
	SAssignNew(Button, SCIRLButton)
	.ButtonStyle(&PlainButtonStyle())
	.OnClicked_Lambda([this, Category]()
	{
		Current = Category;
		Sections->SetActiveWidgetIndex(static_cast<int32>(Category));
		return FReply::Handled();
	});

	// The open section is gold in a framed box; hovered: parchment; others: faded (like the menu's tabs)
	TWeakPtr<SCIRLButton> Weak = Button;
	Button->SetContent(
		SNew(SBorder)
		.BorderImage_Lambda([this, Category]() { return Current == Category ? TabActive() : NoBrush(); })
		.Padding(FMargin(16.f, 5.f))
		[
			SNew(STextBlock)
			.Text(CategoryName(Category))
			.Font(Font(EFont::Title, FontSize))
			.ColorAndOpacity_Lambda([this, Category, Weak]()
			{
				if (Current == Category)
				{
					return FSlateColor(GoldBright());
				}
				const TSharedPtr<SCIRLButton> Pinned = Weak.Pin();
				return FSlateColor(Pinned.IsValid() && Pinned->IsHighlighted() ? Text() : TextMuted());
			})
		]);
	return Button.ToSharedRef();
}

TSharedPtr<SWidget> SCIRLSettingsPanel::GetFirstFocus() const
{
	switch (Current)
	{
	case ECIRLSettingsCategory::Audio:	return SoundSettings.IsValid() ? SoundSettings->GetFirstFocus() : nullptr;
	default:							return nullptr;
	}
}

#undef LOCTEXT_NAMESPACE
