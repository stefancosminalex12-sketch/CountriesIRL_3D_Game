// CountriesIRL 3D Game

#include "Core/CIRLInputConfig.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"

namespace
{
	UInputAction* MakeAction(UObject* Outer, const TCHAR* Name, EInputActionValueType Type)
	{
		UInputAction* Action = NewObject<UInputAction>(Outer, Name);
		Action->ValueType = Type;
		return Action;
	}

	template<typename TModifier>
	TModifier* AddModifier(UInputMappingContext* Context, FEnhancedActionKeyMapping& Mapping)
	{
		TModifier* Modifier = NewObject<TModifier>(Context);
		Mapping.Modifiers.Add(Modifier);
		return Modifier;
	}
}

void UCIRLInputConfig::Build()
{
	Move = MakeAction(this, TEXT("IA_Move"), EInputActionValueType::Axis2D);
	Look = MakeAction(this, TEXT("IA_Look"), EInputActionValueType::Axis2D);
	Jump = MakeAction(this, TEXT("IA_Jump"), EInputActionValueType::Boolean);
	Sprint = MakeAction(this, TEXT("IA_Sprint"), EInputActionValueType::Boolean);
	ToggleView = MakeAction(this, TEXT("IA_ToggleView"), EInputActionValueType::Boolean);
	Attack = MakeAction(this, TEXT("IA_Attack"), EInputActionValueType::Boolean);
	Guard = MakeAction(this, TEXT("IA_Guard"), EInputActionValueType::Boolean);
	CycleEmotion = MakeAction(this, TEXT("IA_CycleEmotion"), EInputActionValueType::Boolean);
	Interact = MakeAction(this, TEXT("IA_Interact"), EInputActionValueType::Boolean);
	GameMenu = MakeAction(this, TEXT("IA_GameMenu"), EInputActionValueType::Boolean);
	Equipment = MakeAction(this, TEXT("IA_Equipment"), EInputActionValueType::Boolean);
	Map = MakeAction(this, TEXT("IA_Map"), EInputActionValueType::Boolean);
	NextSong = MakeAction(this, TEXT("IA_NextSong"), EInputActionValueType::Boolean);

	UInputMappingContext* C = NewObject<UInputMappingContext>(this, TEXT("IMC_Default"));
	DefaultContext = C;

	// Move: WASD (X = right, Y = forward) and left stick
	AddModifier<UInputModifierSwizzleAxis>(C, C->MapKey(Move, EKeys::W));
	{
		FEnhancedActionKeyMapping& S = C->MapKey(Move, EKeys::S);
		AddModifier<UInputModifierSwizzleAxis>(C, S);
		AddModifier<UInputModifierNegate>(C, S);
	}
	AddModifier<UInputModifierNegate>(C, C->MapKey(Move, EKeys::A));
	C->MapKey(Move, EKeys::D);
	AddModifier<UInputModifierDeadZone>(C, C->MapKey(Move, EKeys::Gamepad_Left2D));

	// Look: mouse and right stick. Y is negated here because the project's legacy input scales
	// (DefaultInput.ini bEnableLegacyInputScales) apply a negative pitch scale in the controller.
	{
		FEnhancedActionKeyMapping& Mouse = C->MapKey(Look, EKeys::Mouse2D);
		UInputModifierNegate* Negate = AddModifier<UInputModifierNegate>(C, Mouse);
		Negate->bX = false;
		Negate->bZ = false;
	}
	{
		FEnhancedActionKeyMapping& Stick = C->MapKey(Look, EKeys::Gamepad_Right2D);
		AddModifier<UInputModifierDeadZone>(C, Stick);
		UInputModifierNegate* Negate = AddModifier<UInputModifierNegate>(C, Stick);
		Negate->bX = false;
		Negate->bZ = false;
	}

	C->MapKey(Jump, EKeys::SpaceBar);
	C->MapKey(Jump, EKeys::Gamepad_FaceButton_Bottom);

	C->MapKey(Sprint, EKeys::LeftShift);
	C->MapKey(Sprint, EKeys::Gamepad_LeftThumbstick);

	C->MapKey(ToggleView, EKeys::V);
	C->MapKey(ToggleView, EKeys::Gamepad_DPad_Up);

	C->MapKey(Attack, EKeys::LeftMouseButton);
	C->MapKey(Attack, EKeys::Gamepad_RightTrigger);

	C->MapKey(Guard, EKeys::RightMouseButton);
	C->MapKey(Guard, EKeys::Gamepad_LeftTrigger);

	C->MapKey(Interact, EKeys::E);
	C->MapKey(Interact, EKeys::Gamepad_FaceButton_Left);

	// Menus. In the editor's Play mode Esc stops the game, so Tab/I is the way to test the menu there
	C->MapKey(GameMenu, EKeys::Escape);
	C->MapKey(GameMenu, EKeys::Gamepad_Special_Right);
	C->MapKey(Equipment, EKeys::Tab);
	C->MapKey(Equipment, EKeys::I);
	C->MapKey(Equipment, EKeys::Gamepad_Special_Left);
	C->MapKey(Map, EKeys::M);
	C->MapKey(Map, EKeys::Gamepad_DPad_Right);
	C->MapKey(NextSong, EKeys::N);
	C->MapKey(NextSong, EKeys::Gamepad_FaceButton_Top);

	C->MapKey(CycleEmotion, EKeys::T);
}
