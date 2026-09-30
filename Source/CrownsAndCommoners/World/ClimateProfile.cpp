// Crowns & Commoners

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

	// Weather, from Met Office averages for Linton-on-Ouse (15 km from York, 1991-2020, approximate):
	// days with >= 1 mm of rain, cloudiness derived from sunshine hours (31 h in December, 186 h in July),
	// share of rain falling as showers (mostly summer), mean wind (the Vale of York is fairly sheltered)
	static const float RainDays[12] = { 11.4f, 9.8f, 9.f, 8.8f, 8.2f, 8.9f, 9.1f, 9.9f, 8.9f, 10.8f, 11.8f, 11.6f };
	static const float Cloud[12] = { 0.83f, 0.73f, 0.68f, 0.55f, 0.52f, 0.55f, 0.53f, 0.57f, 0.6f, 0.62f, 0.69f, 0.82f };
	static const float Showers[12] = { 0.15f, 0.15f, 0.25f, 0.35f, 0.45f, 0.5f, 0.55f, 0.5f, 0.35f, 0.25f, 0.15f, 0.15f };
	static const float Wind[12] = { 4.6f, 4.5f, 4.4f, 4.f, 3.7f, 3.4f, 3.3f, 3.3f, 3.5f, 3.9f, 4.2f, 4.4f };

	Months.SetNum(12);
	for (int32 Index = 0; Index < 12; ++Index)
	{
		FMonthlyClimate& Month = Months[Index];
		Month.MeanTemperature = Rows[Index].Mean;
		Month.DailyRange = Rows[Index].Range;
		Month.LeafAmount = Rows[Index].Leaves;
		Month.LeafTint = FLinearColor::FromSRGBColor(Rows[Index].Leaf);
		Month.GrassTint = FLinearColor::FromSRGBColor(Rows[Index].Grass);
		Month.RainDays = RainDays[Index];
		Month.MeanCloudCover = Cloud[Index];
		Month.ShowerShare = Showers[Index];
		Month.MeanWind = Wind[Index];
	}

	// The cold years after 1453: tree rings show Northern Hemisphere summers 0.5-2.5 C colder for about
	// 15 years; England was hit less than the far north, so these are gentle game estimates
	const float Cold[] = { -1.5f, -1.2f, -1.f, -0.9f, -0.8f, -0.8f, -0.7f, -0.5f, -0.4f, -0.4f, -0.3f, -0.3f, -0.3f, -0.2f, -0.2f, -0.2f };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Cold); ++Index)
	{
		YearAnomalies.Add(1453 + Index, Cold[Index]);
	}
}

float UClimateProfile::GetYearAnomaly(int32 Year, int32 Month) const
{
	const float* Anomaly = YearAnomalies.Find(Year);
	if (!Anomaly)
	{
		return 0.f;
	}
	// Volcanic cooling mostly shows in summer
	const bool bSummerHalf = Month >= 5 && Month <= 9;
	return *Anomaly * (bSummerHalf ? 1.f : 0.5f);
}
