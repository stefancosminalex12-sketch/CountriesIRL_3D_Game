// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SSlider;
class UCIRLAudioSubsystem;
enum class ECIRLVolume : uint8;

/**
 *  The volume sliders (Master, Music, SFX), used by the title screen's Settings and the Esc menu.
 *  Moving a slider changes the volume at once; the volumes are saved when the player lets go and when the
 *  widget closes. Mouse drag, or with keyboard/controller: up/down picks a slider, left/right moves it.
 */
class SCIRLSoundSettings : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SCIRLSoundSettings)
		: _SliderWidth(320.f)
		, _LabelWidth(190.f)
		, _FontSize(24.f)
	{}
		SLATE_ARGUMENT(TWeakObjectPtr<UCIRLAudioSubsystem>, Audio)
		SLATE_ARGUMENT(float, SliderWidth)
		SLATE_ARGUMENT(float, LabelWidth)
		SLATE_ARGUMENT(float, FontSize)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SCIRLSoundSettings() override;

	/** The first slider, for keyboard/controller focus */
	TSharedPtr<SWidget> GetFirstFocus() const;

private:

	TSharedRef<SWidget> MakeRow(const FText& Label, ECIRLVolume Which, float SliderWidth, float LabelWidth, float FontSize);

	void Save();

	TWeakObjectPtr<UCIRLAudioSubsystem> Audio;
	TSharedPtr<SSlider> FirstSlider;
};
