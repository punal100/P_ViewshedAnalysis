/*
 * @Author: Punal Manalan
 * @Description: ViewShed Analysis Plugin.
 * @Date: 04/10/2025
 */
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/EngineTypes.h"
#include "CPP_BPL__CustomTrace.generated.h"

USTRUCT(BlueprintType)
struct FS__Points_1DArray
{
	GENERATED_BODY()

public:
	// Let this be Row
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Punal|Point|Array")
	TArray<FVector> Points_1D_Array;
};

USTRUCT(BlueprintType)
struct FS__Points_2DArray
{
	GENERATED_BODY()
public:
	// Let this be Column
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Punal|Point\Array")
	TArray<FS__Points_1DArray> Points_2D_Array;
};

USTRUCT(BlueprintType)
struct FS__Points_3DArray
{
	GENERATED_BODY()
public:
	// Let this be Layer or Slice
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Punal|Point|Array")
	TArray<FS__Points_2DArray> Points_3D_Array;
};

/**
 *
 */
UCLASS()
class P_VIEWSHEDANALYSIS_API UCPP_BPL__CustomTrace : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Punal|Sphere")
	static TArray<FVector> Pack_Spheres_In_Sphere(
		float Arg_BigSphere_Radius,
		float Arg_SmallSphere_Radius,
		int32 MaxIterations = 1000);
	UFUNCTION(BlueprintCallable, Category = "Punal|Sphere")
	static FS__Points_3DArray Arrange_Sphere_In_Grid_With_Overlap(
		float Arg_BigSphere_Radius,
		float Arg_SmallSphere_Radius);

	UFUNCTION(BlueprintCallable, Category = "Punal|Sphere")
	static FS__Points_3DArray Arrange_Sphere_In_Grid_With_Cone_Overlap(
		float Arg_BigSphere_Radius,
		float Arg_SmallSphere_Radius,
		FVector Arg_ConeDirection,
		float Arg_ConeAngle);

	UFUNCTION(BlueprintCallable, Category = "Punal|Sphere")
	static FS__Points_3DArray Arrange_Sphere_In_Grid_With_Pyramid_Overlap(
		float Arg_BigSphere_Radius,
		float Arg_SmallSphere_Radius,
		FVector Arg_Pyramid_Direction,
		float Arg_Pyramid_Vertical_Angle,
		float Arg_Pyramid_Horizontal_Angle,
		bool Arg_Include_Sphere_Sector_Dome);

	UFUNCTION(BlueprintCallable, Category = "Punal|Debug Draw", meta = (WorldContext = "WorldContextObject"))
	static void Debug_Custom_Draw_Pyramid(
		UObject *WorldContextObject,
		FVector Arg_Start_Location,
		FVector Arg_End_Location,
		float Arg_Pyramid_Vertical_Angle,
		float Arg_Pyramid_Horizontal_Angle,
		bool Arg_Include_Sphere_Sector_Dome,
		float DebugDuration = 0.0f,
		float LineThickness = 2.0f,
		FColor PyramidColor = FColor::Yellow,
		FColor SphereColor = FColor::White);

	UFUNCTION(BlueprintCallable, Category = "Punal|Debug Draw", meta = (WorldContext = "WorldContextObject"))
	static void Debug_Custom_Draw_Cone(
		UObject *WorldContextObject,
		FVector Arg_Start_Location,
		FVector Arg_End_Location,
		float Arg_Cone_Angle,
		int32 Arg_Cone_Samples,
		float DebugDuration = 0.0f,
		float LineThickness = 2.0f,
		FColor ConeColor = FColor::Green);

	UFUNCTION(BlueprintCallable, Category = "Punal|CustomTrace", meta = (WorldContext = "WorldContextObject"))
	static void Custom_Shape_Pyramid_Sphere_Trace(
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
		TArray<FHitResult> &Out_Hits);

	// Helper functions
	static bool IsSphereValid(const FVector &Position, float Radius, const TArray<FVector> &ExistingSpheres, float SmallRadius, float BigRadius);
	static FVector FindBestPosition(const TArray<FVector> &ExistingSpheres, float SmallRadius, float BigRadius, int32 GridResolution = 20);
	static bool DoesSphereOverlapCone(
		const FVector &SphereCenter,
		float SphereRadius,
		const FVector &ConeDirection,
		float ConeAngleCos,
		float BigSphereRadius);
	static FVector GetClosestPointOnLineSegment(
		const FVector &Point,
		const FVector &LineStart,
		const FVector &LineEnd);
	static bool DoesSphereOverlapPyramid(
		const FVector &SphereCenter,
		float SphereRadius,
		const FVector &PyramidDirection,
		float PyramidVerticalAngleCos,
		float PyramidHorizontalAngleCos,
		float BigSphereRadius,
		bool IncludeSphereSectorDome);
	static bool IsPointInPyramid(
		const FVector &Point,
		const FVector &PyramidDirection,
		float VerticalAngleCos,
		float HorizontalAngleCos);
};
