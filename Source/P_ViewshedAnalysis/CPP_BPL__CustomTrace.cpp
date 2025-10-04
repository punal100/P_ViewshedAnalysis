/*
 * @Author: Punal Manalan
 * @Description: ViewShed Analysis Plugin.
 * @Date: 04/10/2025
 */

#include "CPP_BPL__CustomTrace.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

TArray<FVector> UCPP_BPL__CustomTrace::Pack_Spheres_In_Sphere(
	float Arg_BigSphere_Radius,
	float Arg_SmallSphere_Radius,
	int32 MaxIterations)
{
	TArray<FVector> SpherePositions;

	// Early exit if small sphere is too big
	if (Arg_SmallSphere_Radius >= Arg_BigSphere_Radius)
	{
		return SpherePositions;
	}

	// Start with center sphere
	SpherePositions.Add(FVector::ZeroVector);

	// Calculate maximum possible spheres (rough estimate)
	float VolumeRatio = FMath::Pow(Arg_BigSphere_Radius / Arg_SmallSphere_Radius, 3);
	int32 EstimatedMaxSpheres = FMath::FloorToInt(VolumeRatio * 0.64f); // ~64% packing efficiency

	// Grid-based sphere placement with collision detection
	int32 Iterations = 0;
	int32 FailedAttempts = 0;
	const int32 MaxFailedAttempts = 100;

	while (Iterations < MaxIterations && FailedAttempts < MaxFailedAttempts && SpherePositions.Num() < EstimatedMaxSpheres)
	{
		FVector BestPosition = FindBestPosition(SpherePositions, Arg_SmallSphere_Radius, Arg_BigSphere_Radius);

		if (BestPosition != FVector::ZeroVector || SpherePositions.Num() == 1)
		{
			if (SpherePositions.Num() > 1) // Don't re-add center
			{
				SpherePositions.Add(BestPosition);
				FailedAttempts = 0;
			}
		}
		else
		{
			FailedAttempts++;
		}

		Iterations++;
	}

	// Secondary pass: Fill gaps with smaller grid resolution
	const int32 SecondaryIterations = 200;
	for (int32 i = 0; i < SecondaryIterations; ++i)
	{
		FVector GapPosition = FindBestPosition(SpherePositions, Arg_SmallSphere_Radius, Arg_BigSphere_Radius, 30);
		if (GapPosition != FVector::ZeroVector)
		{
			SpherePositions.Add(GapPosition);
		}
		else
		{
			break; // No more valid positions found
		}
	}

	return SpherePositions;
}

FS__Points_3DArray UCPP_BPL__CustomTrace::Arrange_Sphere_In_Grid_With_Overlap(
	float Arg_BigSphere_Radius,
	float Arg_SmallSphere_Radius)
{
	FVector Temp_Vector = FVector::ZeroVector;
	FS__Points_3DArray Temp_Points_3D_Array;
	int Temp_Val = 0;

	// Early exit if small sphere is too big
	if (Arg_SmallSphere_Radius >= Arg_BigSphere_Radius)
	{
		return Temp_Points_3D_Array;
	}

	// Exit If Sphere is Zero or Negative Size
	if ((Arg_BigSphere_Radius <= 0) || (Arg_SmallSphere_Radius <= 0))
	{
		return Temp_Points_3D_Array;
	}

	// Create Square 2 Times the Radius of the Big Sphere and Arrange Small Spheres them inside the
	float SmallSphere_Diameter = Arg_SmallSphere_Radius * 2.0f;
	float BigSphere_Diameter = Arg_BigSphere_Radius * 2.0f;
	Temp_Val = FMath::CeilToInt((Arg_BigSphere_Radius / Arg_SmallSphere_Radius));
	// Check if Not Even
	if (((Temp_Val % 2) != 0))
	{
		Temp_Val += 1;
	}
	int Max_Row = (Temp_Val * 2) - 1; // This is X-Axis, Adding Rows In Between Each Row, Example if 2 Row make it 3, if 4 Row Make it 7 Row
	int Max_Col = Max_Row;			  // This is Y-Axys
	int Max_Layer = Max_Row;		  // This is Z-Axis
	FVector StartPosition = FVector::ZeroVector;
	StartPosition.X = -(Arg_SmallSphere_Radius * ((Max_Row - 1) / 2));
	StartPosition.Y = -(Arg_SmallSphere_Radius * ((Max_Row - 1) / 2));
	StartPosition.Z = -(Arg_SmallSphere_Radius * ((Max_Row - 1) / 2));
	// FVector Center_Location = FVector::ZeroVector;//NOTE For Max_Row = 3, Then Center would be Row = 2, Col = 2, Layer = 2

	Temp_Points_3D_Array.Points_3D_Array.AddZeroed(Max_Layer);
	for (int i = 0; i < Max_Layer; ++i)
	{
		Temp_Vector.Z = StartPosition.Z + (Arg_SmallSphere_Radius * i);
		Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array.AddZeroed(Max_Col);
		for (int j = 0; j < Max_Col; ++j)
		{
			Temp_Vector.Y = StartPosition.Y + (Arg_SmallSphere_Radius * j);
			Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array[j].Points_1D_Array.AddZeroed(Max_Row);
			for (int k = 0; k < Max_Row; ++k)
			{
				Temp_Vector.X = StartPosition.X + (Arg_SmallSphere_Radius * k);
				Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array[j].Points_1D_Array[k] = Temp_Vector;
			}
		}
	}
	for (int i = Max_Layer - 1; i >= 0; --i)
	{
		for (int j = Max_Col - 1; j >= 0; --j)
		{
			for (int k = Max_Row - 1; k >= 0; --k)
			{
				// If Small Sphere is Not Overlapping then Remove this Point
				if (Arg_BigSphere_Radius < (Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array[j].Points_1D_Array[k].Length() - Arg_SmallSphere_Radius))
				{
					Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array[j].Points_1D_Array.RemoveAt(k);
				}
			}
			if (Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array[j].Points_1D_Array.Num() <= 0)
			{
				Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array.RemoveAt(j);
			}
		}
		if (Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array.Num() <= 0)
		{
			Temp_Points_3D_Array.Points_3D_Array.RemoveAt(i);
		}
	}

	return Temp_Points_3D_Array;
}

FS__Points_3DArray UCPP_BPL__CustomTrace::Arrange_Sphere_In_Grid_With_Cone_Overlap(
	float Arg_BigSphere_Radius,
	float Arg_SmallSphere_Radius,
	FVector Arg_ConeDirection,
	float Arg_ConeAngle)
{
	FVector Temp_Vector = FVector::ZeroVector;
	FS__Points_3DArray Temp_Points_3D_Array;
	int Temp_Val = 0;

	// Early exit if small sphere is too big
	if (Arg_SmallSphere_Radius >= Arg_BigSphere_Radius)
	{
		return Temp_Points_3D_Array;
	}

	// Exit If Sphere is Zero or Negative Size
	if ((Arg_BigSphere_Radius <= 0) || (Arg_SmallSphere_Radius <= 0))
	{
		return Temp_Points_3D_Array;
	}

	// Exit if cone angle is invalid
	if (Arg_ConeAngle <= 0 || Arg_ConeAngle >= 180.0f)
	{
		return Temp_Points_3D_Array;
	}

	// Normalize cone direction
	FVector ConeDir = Arg_ConeDirection.GetSafeNormal();
	if (ConeDir.IsNearlyZero())
	{
		return Temp_Points_3D_Array;
	}

	// Calculate cone angle cosine for dot product comparison
	float ConeAngleRad = FMath::DegreesToRadians(Arg_ConeAngle * 0.5f);
	float ConeAngleCos = FMath::Cos(ConeAngleRad);

	// Create grid calculation
	Temp_Val = FMath::CeilToInt((Arg_BigSphere_Radius / Arg_SmallSphere_Radius));
	if (((Temp_Val % 2) != 0))
	{
		Temp_Val += 1;
	}

	int Max_Row = (Temp_Val * 2) - 1;
	int Max_Col = Max_Row;
	int Max_Layer = Max_Row;

	FVector StartPosition = FVector::ZeroVector;
	StartPosition.X = -(Arg_SmallSphere_Radius * ((Max_Row - 1) / 2));
	StartPosition.Y = -(Arg_SmallSphere_Radius * ((Max_Row - 1) / 2));
	StartPosition.Z = -(Arg_SmallSphere_Radius * ((Max_Row - 1) / 2));

	// Initialize and populate 3D array
	Temp_Points_3D_Array.Points_3D_Array.AddZeroed(Max_Layer);
	for (int i = 0; i < Max_Layer; ++i)
	{
		Temp_Vector.Z = StartPosition.Z + (Arg_SmallSphere_Radius * i);
		Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array.AddZeroed(Max_Col);
		for (int j = 0; j < Max_Col; ++j)
		{
			Temp_Vector.Y = StartPosition.Y + (Arg_SmallSphere_Radius * j);
			Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array[j].Points_1D_Array.AddZeroed(Max_Row);
			for (int k = 0; k < Max_Row; ++k)
			{
				Temp_Vector.X = StartPosition.X + (Arg_SmallSphere_Radius * k);
				Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array[j].Points_1D_Array[k] = Temp_Vector;
			}
		}
	}

	// Filter spheres that don't overlap with cone (iterate backwards)
	for (int i = Max_Layer - 1; i >= 0; --i)
	{
		for (int j = Max_Col - 1; j >= 0; --j)
		{
			for (int k = Max_Row - 1; k >= 0; --k)
			{
				FVector SpherePos = Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array[j].Points_1D_Array[k];

				// Use helper function to check cone overlap
				if (!DoesSphereOverlapCone(SpherePos, Arg_SmallSphere_Radius, ConeDir, ConeAngleCos, Arg_BigSphere_Radius))
				{
					Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array[j].Points_1D_Array.RemoveAt(k);
				}
			}

			if (Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array[j].Points_1D_Array.Num() <= 0)
			{
				Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array.RemoveAt(j);
			}
		}

		if (Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array.Num() <= 0)
		{
			Temp_Points_3D_Array.Points_3D_Array.RemoveAt(i);
		}
	}

	return Temp_Points_3D_Array;
}

FS__Points_3DArray UCPP_BPL__CustomTrace::Arrange_Sphere_In_Grid_With_Pyramid_Overlap(
	float Arg_BigSphere_Radius,
	float Arg_SmallSphere_Radius,
	FVector Arg_Pyramid_Direction,
	float Arg_Pyramid_Vertical_Angle,
	float Arg_Pyramid_Horizontal_Angle,
	bool Arg_Include_Sphere_Sector_Dome)
{
	FVector Temp_Vector = FVector::ZeroVector;
	FS__Points_3DArray Temp_Points_3D_Array;
	int Temp_Val = 0;

	// Early exit if small sphere is too big
	if (Arg_SmallSphere_Radius >= Arg_BigSphere_Radius)
	{
		return Temp_Points_3D_Array;
	}

	// Exit If Sphere is Zero or Negative Size
	if ((Arg_BigSphere_Radius <= 0) || (Arg_SmallSphere_Radius <= 0))
	{
		return Temp_Points_3D_Array;
	}

	// Exit if pyramid angles are invalid
	if (Arg_Pyramid_Vertical_Angle <= 0 || Arg_Pyramid_Vertical_Angle >= 180.0f ||
		Arg_Pyramid_Horizontal_Angle <= 0 || Arg_Pyramid_Horizontal_Angle >= 180.0f)
	{
		return Temp_Points_3D_Array;
	}

	// Normalize pyramid direction
	FVector PyramidDir = Arg_Pyramid_Direction.GetSafeNormal();
	if (PyramidDir.IsNearlyZero())
	{
		return Temp_Points_3D_Array;
	}

	// Calculate pyramid angle cosines for dot product comparison
	float VerticalAngleRad = FMath::DegreesToRadians(Arg_Pyramid_Vertical_Angle * 0.5f);
	float HorizontalAngleRad = FMath::DegreesToRadians(Arg_Pyramid_Horizontal_Angle * 0.5f);
	float VerticalAngleCos = FMath::Cos(VerticalAngleRad);
	float HorizontalAngleCos = FMath::Cos(HorizontalAngleRad);

	// Create grid calculation
	Temp_Val = FMath::CeilToInt((Arg_BigSphere_Radius / Arg_SmallSphere_Radius));
	if (((Temp_Val % 2) != 0))
	{
		Temp_Val += 1;
	}

	int Max_Row = (Temp_Val * 2) - 1;
	int Max_Col = Max_Row;
	int Max_Layer = Max_Row;

	FVector StartPosition = FVector::ZeroVector;
	StartPosition.X = -(Arg_SmallSphere_Radius * ((Max_Row - 1) / 2));
	StartPosition.Y = -(Arg_SmallSphere_Radius * ((Max_Row - 1) / 2));
	StartPosition.Z = -(Arg_SmallSphere_Radius * ((Max_Row - 1) / 2));

	// Initialize and populate 3D array
	Temp_Points_3D_Array.Points_3D_Array.AddZeroed(Max_Layer);
	for (int i = 0; i < Max_Layer; ++i)
	{
		Temp_Vector.Z = StartPosition.Z + (Arg_SmallSphere_Radius * i);
		Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array.AddZeroed(Max_Col);
		for (int j = 0; j < Max_Col; ++j)
		{
			Temp_Vector.Y = StartPosition.Y + (Arg_SmallSphere_Radius * j);
			Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array[j].Points_1D_Array.AddZeroed(Max_Row);
			for (int k = 0; k < Max_Row; ++k)
			{
				Temp_Vector.X = StartPosition.X + (Arg_SmallSphere_Radius * k);
				Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array[j].Points_1D_Array[k] = Temp_Vector;
			}
		}
	}

	// Filter spheres that don't overlap with pyramid (iterate backwards)
	for (int i = Max_Layer - 1; i >= 0; --i)
	{
		for (int j = Max_Col - 1; j >= 0; --j)
		{
			for (int k = Max_Row - 1; k >= 0; --k)
			{
				FVector SpherePos = Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array[j].Points_1D_Array[k];

				// Use helper function to check pyramid overlap
				if (!DoesSphereOverlapPyramid(SpherePos, Arg_SmallSphere_Radius, PyramidDir,
											  VerticalAngleCos, HorizontalAngleCos, Arg_BigSphere_Radius, Arg_Include_Sphere_Sector_Dome))
				{
					Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array[j].Points_1D_Array.RemoveAt(k);
				}
			}

			if (Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array[j].Points_1D_Array.Num() <= 0)
			{
				Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array.RemoveAt(j);
			}
		}

		if (Temp_Points_3D_Array.Points_3D_Array[i].Points_2D_Array.Num() <= 0)
		{
			Temp_Points_3D_Array.Points_3D_Array.RemoveAt(i);
		}
	}

	return Temp_Points_3D_Array;
}

void UCPP_BPL__CustomTrace::Debug_Custom_Draw_Pyramid(
	UObject *WorldContextObject,
	FVector Arg_Start_Location,
	FVector Arg_End_Location,
	float Arg_Pyramid_Vertical_Angle,
	float Arg_Pyramid_Horizontal_Angle,
	bool Arg_Include_Sphere_Sector_Dome,
	float DebugDuration,
	float LineThickness,
	FColor PyramidColor,
	FColor SphereColor)
{
	if (!WorldContextObject)
	{
		return;
	}

	UWorld *World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);

	FVector Forward = (Arg_End_Location - Arg_Start_Location).GetSafeNormal();
	float Distance = FVector::Dist(Arg_Start_Location, Arg_End_Location);

	// Create orthogonal basis vectors
	FVector Up = FVector::UpVector;
	FVector Right = FVector::CrossProduct(Forward, Up).GetSafeNormal();
	if (Right.IsNearlyZero())
	{
		Up = FVector::RightVector;
		Right = FVector::CrossProduct(Forward, Up).GetSafeNormal();
	}
	Up = FVector::CrossProduct(Right, Forward).GetSafeNormal();

	// Calculate pyramid corner directions
	float HalfVertAngle = Arg_Pyramid_Vertical_Angle * 0.5f;
	float HalfHorzAngle = Arg_Pyramid_Horizontal_Angle * 0.5f;

	FVector BaseCorners[4];
	for (int i = 0; i < 4; ++i)
	{
		float VertAngle = (i < 2) ? HalfVertAngle : -HalfVertAngle;
		float HorzAngle = (i % 2 == 0) ? HalfHorzAngle : -HalfHorzAngle;

		// Create rotation quaternions for proper angle handling
		FQuat VerticalRot = FQuat(Right, FMath::DegreesToRadians(VertAngle));
		FQuat HorizontalRot = FQuat(Up, FMath::DegreesToRadians(HorzAngle));
		FQuat CombinedRot = VerticalRot * HorizontalRot;

		FVector CornerDirection = CombinedRot.RotateVector(Forward);
		BaseCorners[i] = Arg_Start_Location + CornerDirection * Distance;
	}

	// Draw pyramid edges (apex to base corners)
	for (int i = 0; i < 4; ++i)
	{
		DrawDebugLine(World, Arg_Start_Location, BaseCorners[i], PyramidColor, false, DebugDuration, 0, LineThickness);
	}

	// Draw pyramid base (connecting base corners)
	for (int i = 0; i < 4; ++i)
	{
		DrawDebugLine(World, BaseCorners[i], BaseCorners[(i + 1) % 4], PyramidColor, false, DebugDuration, 0, LineThickness);
	}

	// Draw center line for reference
	DrawDebugLine(World, Arg_Start_Location, Arg_End_Location, FColor::White, false, DebugDuration, 0, LineThickness * 0.5f);

	// If Include Sphere Sector Dome is true, draw the sphere
	if (Arg_Include_Sphere_Sector_Dome)
	{
		DrawDebugSphere(World, Arg_Start_Location, Distance, 24, SphereColor, false, DebugDuration, 0, LineThickness);

		// Draw additional dome sector lines for visual clarity
		int NumSectorLines = 8;
		for (int i = 0; i < NumSectorLines; ++i)
		{
			float Angle = (i * 360.0f) / NumSectorLines;
			FQuat RotationQuat = FQuat(Forward, FMath::DegreesToRadians(Angle));
			FVector RadialDirection = RotationQuat.RotateVector(Up);
			FVector SpherePoint = Arg_Start_Location + RadialDirection * Distance;

			// Only draw if within pyramid bounds (simplified check)
			FVector ToPoint = (SpherePoint - Arg_Start_Location).GetSafeNormal();
			float DotProduct = FVector::DotProduct(ToPoint, Forward);
			if (DotProduct > FMath::Cos(FMath::DegreesToRadians(FMath::Max(HalfVertAngle, HalfHorzAngle))))
			{
				DrawDebugLine(World, Arg_Start_Location, SpherePoint, SphereColor, false, DebugDuration, 0, LineThickness * 0.7f);
			}
		}
	}
}

void UCPP_BPL__CustomTrace::Debug_Custom_Draw_Cone(
	UObject *WorldContextObject,
	FVector Arg_Start_Location,
	FVector Arg_End_Location,
	float Arg_Cone_Angle,
	int32 Arg_Cone_Samples,
	float DebugDuration,
	float LineThickness,
	FColor ConeColor)
{
	if (!WorldContextObject || Arg_Cone_Samples <= 0)
	{
		return;
	}

	UWorld *World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);

	FVector Forward = (Arg_End_Location - Arg_Start_Location).GetSafeNormal();
	float Distance = FVector::Dist(Arg_Start_Location, Arg_End_Location);

	// Create orthogonal basis vectors
	FVector Up = FVector::UpVector;
	FVector Right = FVector::CrossProduct(Forward, Up).GetSafeNormal();
	if (Right.IsNearlyZero())
	{
		Up = FVector::RightVector;
		Right = FVector::CrossProduct(Forward, Up).GetSafeNormal();
	}
	Up = FVector::CrossProduct(Right, Forward).GetSafeNormal();

	// Calculate cone parameters
	float HalfConeAngle = Arg_Cone_Angle * 0.5f;
	float ConeRadius = Distance * FMath::Tan(FMath::DegreesToRadians(HalfConeAngle));

	// Draw cone base circle
	TArray<FVector> CirclePoints;
	for (int32 i = 0; i <= Arg_Cone_Samples; ++i)
	{
		float Angle = (i * 2.0f * PI) / Arg_Cone_Samples;
		FVector CirclePoint = Arg_End_Location +
							  (Up * FMath::Cos(Angle) * ConeRadius) +
							  (Right * FMath::Sin(Angle) * ConeRadius);
		CirclePoints.Add(CirclePoint);

		// Draw lines from apex to circle points
		DrawDebugLine(World, Arg_Start_Location, CirclePoint, ConeColor, false, DebugDuration, 0, LineThickness);
	}

	// Draw cone base circle
	for (int32 i = 0; i < CirclePoints.Num() - 1; ++i)
	{
		DrawDebugLine(World, CirclePoints[i], CirclePoints[i + 1], ConeColor, false, DebugDuration, 0, LineThickness);
	}

	// Draw center line for reference
	DrawDebugLine(World, Arg_Start_Location, Arg_End_Location, FColor::White, false, DebugDuration, 0, LineThickness * 0.5f);

	// Draw additional radial lines for better visualization
	int32 RadialLines = FMath::Min(Arg_Cone_Samples / 2, 8);
	for (int32 i = 0; i < RadialLines; ++i)
	{
		float Angle = (i * 2.0f * PI) / RadialLines;
		FVector RadialPoint = Arg_End_Location +
							  (Up * FMath::Cos(Angle) * ConeRadius) +
							  (Right * FMath::Sin(Angle) * ConeRadius);
		DrawDebugLine(World, Arg_Start_Location, RadialPoint, ConeColor, false, DebugDuration, 0, LineThickness * 0.8f);
	}
}

void UCPP_BPL__CustomTrace::Custom_Shape_Pyramid_Sphere_Trace(
	UObject *WorldContextObject,
	FVector Arg_TraceStart,
	FVector Arg_TraceEnd,
	float Arg_SmallSphere_Radius,
	bool Arg_Include_Sphere_Sector_Dome,
	float Arg_Vertical_Angle,
	float Arg_Horizontal_Angle,
	int32 Arg_Cone_Samples,
	TEnumAsByte<ETraceTypeQuery> Arg_TraceChannel,
	const TArray<AActor *> &Arg_ActorsToIgnore,
	bool Arg_DrawDebug,
	float Arg_DebugDuration,
	float Arg_DebugLineThickness,
	TArray<FHitResult> &Out_Hits)
{
	Out_Hits.Empty();

	UWorld *World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	float MaxLength = (Arg_TraceEnd - Arg_TraceStart).Size();
	float BigSphereRadius = MaxLength;

	// Arrange grid points that overlap the pyramid (and optional dome)
	FS__Points_3DArray Grid = Arrange_Sphere_In_Grid_With_Pyramid_Overlap(
		BigSphereRadius,
		Arg_SmallSphere_Radius,
		(Arg_TraceEnd - Arg_TraceStart).GetSafeNormal(),
		Arg_Vertical_Angle,
		Arg_Horizontal_Angle,
		Arg_Include_Sphere_Sector_Dome);

	// Flatten 3D array and perform sphere traces at each point
	for (auto &Layer : Grid.Points_3D_Array)
	{
		for (auto &Row : Layer.Points_2D_Array)
		{
			for (auto &Point : Row.Points_1D_Array)
			{
				FVector WorldPoint = Arg_TraceStart + Point;
				// Trace from start toward that grid point direction
				FVector Dir = (WorldPoint - Arg_TraceStart).GetSafeNormal();
				FVector End = Arg_TraceStart + Dir * MaxLength;

				FHitResult Hit;
				bool bHit = UKismetSystemLibrary::SphereTraceSingle(
					WorldContextObject,
					Arg_TraceStart,
					End,
					Arg_SmallSphere_Radius,
					Arg_TraceChannel,
					false,
					Arg_ActorsToIgnore,
					Arg_DrawDebug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
					Hit,
					true);

				if (Arg_DrawDebug)
				{
					DrawDebugLine(World, Arg_TraceStart, End, bHit ? FColor::Green : FColor::Red,
								  false, Arg_DebugDuration, 0, Arg_DebugLineThickness);
				}

				if (bHit)
				{
					Out_Hits.Add(Hit);
				}
			}
		}
	}

	// Optional: draw pyramid edges and dome as before for debug
	if (Arg_DrawDebug)
	{
		Debug_Custom_Draw_Pyramid(
			WorldContextObject,
			Arg_TraceStart,
			Arg_TraceEnd,
			Arg_Vertical_Angle,
			Arg_Horizontal_Angle,
			Arg_Include_Sphere_Sector_Dome,
			Arg_DebugDuration,
			Arg_DebugLineThickness);
	}
}

bool UCPP_BPL__CustomTrace::IsSphereValid(const FVector &Position, float Radius, const TArray<FVector> &ExistingSpheres, float SmallRadius, float BigRadius)
{
	// Check if sphere is within big sphere boundary
	if (Position.Size() + SmallRadius > BigRadius)
	{
		return false;
	}

	// Check collision with existing spheres
	for (const FVector &ExistingPos : ExistingSpheres)
	{
		float Distance = FVector::Dist(Position, ExistingPos);
		if (Distance < (SmallRadius * 2.0f))
		{
			return false;
		}
	}

	return true;
}

FVector UCPP_BPL__CustomTrace::FindBestPosition(const TArray<FVector> &ExistingSpheres, float SmallRadius, float BigRadius, int32 GridResolution)
{
	FVector BestPosition = FVector::ZeroVector;
	float BestScore = -1.0f;

	// Create 3D grid to sample positions
	float StepSize = (BigRadius * 2.0f) / GridResolution;
	float MaxRadius = BigRadius - SmallRadius;

	for (int32 x = 0; x < GridResolution; ++x)
	{
		for (int32 y = 0; y < GridResolution; ++y)
		{
			for (int32 z = 0; z < GridResolution; ++z)
			{
				FVector GridPos(
					-BigRadius + (x * StepSize),
					-BigRadius + (y * StepSize),
					-BigRadius + (z * StepSize));

				// Skip if outside sphere bounds
				if (GridPos.Size() > MaxRadius)
					continue;

				// Check if position is valid
				if (IsSphereValid(GridPos, SmallRadius, ExistingSpheres, SmallRadius, BigRadius))
				{
					// Calculate score (prefer positions closer to existing spheres for better packing)
					float Score = 0.0f;
					for (const FVector &ExistingPos : ExistingSpheres)
					{
						float Distance = FVector::Dist(GridPos, ExistingPos);
						Score += 1.0f / FMath::Max(Distance, SmallRadius * 2.1f); // Avoid division by zero
					}

					// Prefer positions closer to center for stability
					Score += 1.0f / FMath::Max(GridPos.Size(), 1.0f);

					if (Score > BestScore)
					{
						BestScore = Score;
						BestPosition = GridPos;
					}
				}
			}
		}
	}

	return BestScore > 0.0f ? BestPosition : FVector::ZeroVector;
}

bool UCPP_BPL__CustomTrace::DoesSphereOverlapCone(
	const FVector &SphereCenter,
	float SphereRadius,
	const FVector &ConeDirection,
	float ConeAngleCos,
	float BigSphereRadius)
{
	// Line segment: Start = Big Sphere Center (0,0,0), End = ConeDirection * BigSphereRadius
	FVector LineStart = FVector::ZeroVector;
	FVector LineEnd = ConeDirection * BigSphereRadius;

	// Find closest point on small sphere to the line segment
	FVector ClosestPointOnLine = GetClosestPointOnLineSegment(SphereCenter, LineStart, LineEnd);

	// Get direction from line point to sphere center
	FVector LineToSphere = (SphereCenter - ClosestPointOnLine);
	float DistanceToLine = LineToSphere.Size();

	// If sphere is too far from line, it can't overlap cone
	if (DistanceToLine > SphereRadius)
	{
		return false;
	}

	// Find closest point on small sphere surface to the line
	FVector ClosestPointOnSphere = FVector::ZeroVector;
	if (DistanceToLine > KINDA_SMALL_NUMBER)
	{
		FVector DirectionToSphere = LineToSphere.GetSafeNormal();
		ClosestPointOnSphere = SphereCenter - (DirectionToSphere * SphereRadius);
	}
	else
	{
		ClosestPointOnSphere = SphereCenter;
	}

	// Check if the closest point on sphere is within the cone
	FVector DirectionToPoint = ClosestPointOnSphere.GetSafeNormal();
	float DotProduct = FVector::DotProduct(DirectionToPoint, ConeDirection);

	return DotProduct >= ConeAngleCos;
}

FVector UCPP_BPL__CustomTrace::GetClosestPointOnLineSegment(
	const FVector &Point,
	const FVector &LineStart,
	const FVector &LineEnd)
{
	FVector LineVec = LineEnd - LineStart;
	FVector PointVec = Point - LineStart;

	float LineLength = LineVec.Size();
	if (LineLength < KINDA_SMALL_NUMBER)
	{
		return LineStart; // Line segment is too short
	}

	FVector LineDir = LineVec / LineLength;
	float ProjectedDistance = FVector::DotProduct(PointVec, LineDir);

	// Clamp to line segment bounds
	ProjectedDistance = FMath::Clamp(ProjectedDistance, 0.0f, LineLength);

	return LineStart + (LineDir * ProjectedDistance);
}
bool UCPP_BPL__CustomTrace::IsPointInPyramid(
	const FVector &Point,
	const FVector &PyramidDirection,
	float VerticalAngleCos,
	float HorizontalAngleCos)
{
	if (Point.IsNearlyZero())
	{
		return true; // Origin is always in pyramid
	}

	FVector PointDirection = Point.GetSafeNormal();

	// Check if point is in front of pyramid (positive dot product)
	float ForwardDot = FVector::DotProduct(PointDirection, PyramidDirection);
	if (ForwardDot <= 0)
	{
		return false; // Point is behind pyramid
	}

	// Create orthogonal basis for pyramid
	FVector Up = FVector::UpVector;
	FVector Right = FVector::CrossProduct(PyramidDirection, Up).GetSafeNormal();
	if (Right.IsNearlyZero())
	{
		Up = FVector::RightVector;
		Right = FVector::CrossProduct(PyramidDirection, Up).GetSafeNormal();
	}
	Up = FVector::CrossProduct(Right, PyramidDirection).GetSafeNormal();

	// Project point direction onto vertical and horizontal planes
	float VerticalDot = FVector::DotProduct(PointDirection, Up);
	float HorizontalDot = FVector::DotProduct(PointDirection, Right);

	// Calculate angles from pyramid axis
	FVector ProjectedVertical = PyramidDirection + (Up * VerticalDot);
	FVector ProjectedHorizontal = PyramidDirection + (Right * HorizontalDot);

	float VerticalAngleCheck = FVector::DotProduct(PointDirection, ProjectedVertical.GetSafeNormal());
	float HorizontalAngleCheck = FVector::DotProduct(PointDirection, ProjectedHorizontal.GetSafeNormal());

	// Check if within pyramid bounds
	return (VerticalAngleCheck >= VerticalAngleCos) && (HorizontalAngleCheck >= HorizontalAngleCos);
}

bool UCPP_BPL__CustomTrace::DoesSphereOverlapPyramid(
	const FVector &SphereCenter,
	float SphereRadius,
	const FVector &PyramidDirection,
	float PyramidVerticalAngleCos,
	float PyramidHorizontalAngleCos,
	float BigSphereRadius,
	bool IncludeSphereSectorDome)
{
	// Check if sphere center is within pyramid
	if (IsPointInPyramid(SphereCenter, PyramidDirection, PyramidVerticalAngleCos, PyramidHorizontalAngleCos))
	{
		return true;
	}

	// Check if any point on sphere surface is within pyramid
	// Sample points around sphere surface
	for (int i = 0; i < 8; ++i)
	{
		float Theta = (i * 2.0f * PI) / 8.0f;
		for (int j = 0; j < 4; ++j)
		{
			float Phi = (j * PI) / 4.0f;

			FVector SpherePoint = SphereCenter + FVector(
													 SphereRadius * FMath::Sin(Phi) * FMath::Cos(Theta),
													 SphereRadius * FMath::Sin(Phi) * FMath::Sin(Theta),
													 SphereRadius * FMath::Cos(Phi));

			if (IsPointInPyramid(SpherePoint, PyramidDirection, PyramidVerticalAngleCos, PyramidHorizontalAngleCos))
			{
				return true;
			}
		}
	}

	// If Arg_Include_Sphere_Sector_Dome is true, check additional conditions
	if (IncludeSphereSectorDome)
	{
		// Check if sphere center is in front of pyramid
		FVector CenterDirection = SphereCenter.GetSafeNormal();
		float ForwardDot = FVector::DotProduct(CenterDirection, PyramidDirection);

		if (ForwardDot > 0) // Point is in front of pyramid
		{
			// Check if any part of sphere is within big sphere radius
			float DistanceFromOrigin = SphereCenter.Size();
			if (DistanceFromOrigin <= BigSphereRadius + SphereRadius)
			{
				return true;
			}
		}
	}

	return false;
}