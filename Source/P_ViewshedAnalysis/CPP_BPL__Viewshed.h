/*
 * @Author: Punal Manalan
 * @Description: ViewShed Analysis Plugin.
 * @Date: 04/10/2025
 */
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CPP_Actor__Viewshed.h"
#include "CPP_BPL__Viewshed.generated.h"

/**
 * Blueprint function library providing utility functions for ViewShed analysis
 * These functions can be called from Blueprint graphs or C++ code
 */
UCLASS()
class P_VIEWSHEDANALYSIS_API UCPP_BPL__Viewshed : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Check if a single point is visible from a location
     * Performs a simple line trace between two points
     * @param World - World context for line tracing
     * @param ViewerLocation - Location to trace from
     * @param TargetLocation - Location to trace to
     * @param IgnoreActor - Actor to ignore during trace (optional)
     * @return True if target is visible from viewer location
     */
    UFUNCTION(BlueprintCallable, Category = "ViewShed Analysis",
              meta = (Keywords = "viewshed visibility analysis trace"))
    static bool IsPointVisibleFromLocation(
        UWorld *World,
        FVector ViewerLocation,
        FVector TargetLocation,
        AActor *IgnoreActor = nullptr);

    /**
     * Calculate visibility percentage from an array of viewshed points
     * @param ViewShedPoints - Array of analysis points
     * @return Percentage of visible points (0-100)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ViewShed Analysis")
    static float CalculateVisibilityPercentage(const TArray<FS__ViewShedPoint> &ViewShedPoints);

    /**
     * Filter viewshed points by distance range
     * @param ViewShedPoints - Input array of points
     * @param MinDistance - Minimum distance to include
     * @param MaxDistance - Maximum distance to include
     * @return Filtered array of points within distance range
     */
    UFUNCTION(BlueprintCallable, Category = "ViewShed Analysis")
    static TArray<FS__ViewShedPoint> FilterPointsByDistance(
        const TArray<FS__ViewShedPoint> &ViewShedPoints,
        float MinDistance,
        float MaxDistance);

    /**
     * Get only visible points from viewshed results
     * @param ViewShedPoints - Input array of points
     * @return Array containing only visible points
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ViewShed Analysis")
    static TArray<FS__ViewShedPoint> GetVisiblePoints(const TArray<FS__ViewShedPoint> &ViewShedPoints);

    /**
     * Get only hidden points from viewshed results
     * @param ViewShedPoints - Input array of points
     * @return Array containing only hidden points
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ViewShed Analysis")
    static TArray<FS__ViewShedPoint> GetHiddenPoints(const TArray<FS__ViewShedPoint> &ViewShedPoints);

    /**
     * Find the closest visible point to a given location
     * @param ViewShedPoints - Array of analysis points
     * @param Location - Reference location
     * @param FoundPoint - Output parameter for the closest point
     * @return True if a visible point was found
     */
    UFUNCTION(BlueprintCallable, Category = "ViewShed Analysis")
    static bool FindClosestVisiblePoint(
        const TArray<FS__ViewShedPoint> &ViewShedPoints,
        FVector Location,
        FS__ViewShedPoint &FoundPoint);
};
