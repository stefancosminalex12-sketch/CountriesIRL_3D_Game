// CountriesIRL 3D Game

#include "World/ClimateProfile.h"

UClimateProfile::UClimateProfile()
{
	// Yorkshire (York area), 1450s. Temperatures: modern York averages minus ~0.5 °C.
	// Leaves: oak/ash come out late April to late May, colour in October, fall through November.
	// Grass: lush May-June, hay-coloured in the dry weeks of late summer, dull through winter.
	struct FRow { float Mean, Range, Leaves; FColor Leaf, Grass; };
	static const FRow Rows[12] =
	{
		{ 3.3f, 6.f,  0.00f, FColor(120, 95, 60),  FColor(92, 98, 62) },	// January
		{ 3.6f, 7.f,  0.00f, FColor(120, 95, 60),  FColor(94, 100, 62) },	// February
		{ 5.4f, 8.f,  0.00f, FColor(125, 100, 62), FColor(92, 108, 56) },	// March
		{ 7.4f, 9.f,  0.10f, FColor(160, 196, 76), FColor(86, 124, 50) },	// April: buds breaking
		{ 10.5f, 10.f, 0.65f, FColor(124, 178, 56), FColor(80, 132, 46) },	// May: fresh green
		{ 13.5f, 10.f, 1.00f, FColor(88, 142, 44),  FColor(76, 126, 44) },	// June
		{ 15.5f, 10.f, 1.00f, FColor(76, 126, 40),  FColor(100, 126, 50) },	// July: haymaking
		{ 15.2f, 9.5f, 1.00f, FColor(78, 120, 40),  FColor(124, 122, 60) },	// August: harvest, dry grass
		{ 12.8f, 9.f,  0.97f, FColor(104, 124, 42), FColor(104, 118, 54) },	// September
		{ 9.4f, 8.f,  0.80f, FColor(204, 132, 40), FColor(96, 108, 54) },	// October: autumn gold
		{ 5.8f, 7.f,  0.25f, FColor(160, 90, 40),  FColor(92, 100, 58) },	// November: leaf fall
		{ 3.9f, 6.f,  0.00f, FColor(120, 80, 45),  FColor(90, 96, 60) },	// December
	};

	Months.SetNum(12);
	for (int32 Index = 0; Index < 12; ++Index)
	{
		FMonthlyClimate& Month = Months[Index];
		Month.MeanTemperature = Rows[Index].Mean;
		Month.DailyRange = Rows[Index].Range;
		Month.LeafAmount = Rows[Index].Leaves;
		Month.LeafTint = FLinearColor::FromSRGBColor(Rows[Index].Leaf);
		Month.GrassTint = FLinearColor::FromSRGBColor(Rows[Index].Grass);
	}
}
