// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SWidgetSwitcher;
class SCIRLSoundSettings;
class UCIRLAudioSubsystem;

/** The settings' sections, top to bottom. Later: Graphics, Controls, Gameplay... */
enum class ECIRLSettingsCategory : uint8
{
	Audio,
	Count
};

/**
 *  The game's settings, the same on the title screen and in the Esc menu: the sections listed on the left,
 *  the open section's options on the right. The host adds its own Back button.
 *  Keyboard/controller: up/down in the list, right into the options.
 */
class SCIRLSettingsPanel : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SCIRLSettingsPanel)
		: _FontSize(22.f)
		, _SliderWidth(260.f)
		, _LabelWidth(190.f)
	{}
		SLATE_ARGUMENT(TWeakObjectPtr<UCIRLAudioSubsystem>, Audio)
		SLATE_ARGUMENT(float, FontSize)
		SLATE_ARGUMENT(float, SliderWidth)
		SLATE_ARGUMENT(float, LabelWidth)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Where keyboard/controller focus goes when the settings open: the open section's first option */
	TSharedPtr<SWidget> GetFirstFocus() const;

private:

	TSharedRef<SWidget> MakeCategoryButton(ECIRLSettingsCategory Category, float FontSize);

	ECIRLSettingsCategory Current = ECIRLSettingsCategory::Audio;

	TSharedPtr<SWidgetSwitcher> Sections;
	TSharedPtr<SCIRLSoundSettings> SoundSettings;
};
