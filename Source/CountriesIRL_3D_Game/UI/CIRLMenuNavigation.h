// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "Framework/Application/NavigationConfig.h"
#include "Framework/Application/SlateApplication.h"

/**
 *  While one of our menus is open, WASD moves between buttons as well as the arrow keys and the controller.
 *  Push() swaps Slate's navigation rules in and returns the old ones; Pop() puts them back.
 */
namespace CIRLMenuNavigation
{
	inline TSharedPtr<FNavigationConfig> Push()
	{
		FSlateApplication& Slate = FSlateApplication::Get();
		TSharedPtr<FNavigationConfig> Previous = Slate.GetNavigationConfig();

		TSharedRef<FNavigationConfig> Navigation = MakeShared<FNavigationConfig>();
		Navigation->KeyEventRules.Emplace(EKeys::W, EUINavigation::Up);
		Navigation->KeyEventRules.Emplace(EKeys::S, EUINavigation::Down);
		Navigation->KeyEventRules.Emplace(EKeys::A, EUINavigation::Left);
		Navigation->KeyEventRules.Emplace(EKeys::D, EUINavigation::Right);
		Slate.SetNavigationConfig(Navigation);
		return Previous;
	}

	inline void Pop(TSharedPtr<FNavigationConfig>& Previous)
	{
		if (Previous.IsValid() && FSlateApplication::IsInitialized())
		{
			FSlateApplication::Get().SetNavigationConfig(Previous.ToSharedRef());
		}
		Previous.Reset();
	}
}
