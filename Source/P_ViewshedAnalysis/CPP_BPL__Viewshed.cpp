/*
 * @Author: Punal Manalan
 * @Description: ViewShed Analysis Plugin.
 * @Date: 04/10/2025
 */

#include "CPP_BPL__Viewshed.h"
#include "Engine/World.h"

/**
 * Check if a single point is visible from a location using line trace
 */
bool UCPP_BPL__Viewshed::IsPointVisibleFromLocation(
    UWorld *World,
    FVector ViewerLocation,
    FVector TargetLocation,
    AActor *IgnoreActor)
{
    // Validate world pointer
    if (!World)
    {
        return false;
    }

    // Set up collision query parameters
    FCollisionQueryParams QueryParams;
    if (IgnoreActor)
    {
        // Add actor to ignore list if provided
        QueryParams.AddIgnoredActor(IgnoreActor);
    }
    // Use simple collision for better performance
    QueryParams.bTraceComplex = false;

    // Perform line trace
    FHitResult HitResult;
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,      // Output hit result
        ViewerLocation, // Start location
        TargetLocation, // End location
        ECC_Visibility, // Visibility collision channel
        QueryParams     // Query parameters
    );

    // Return true if no hit occurred (point is visible)
    return !bHit;
}

/**
 * Calculate visibility percentage from viewshed results
 */
float UCPP_BPL__Viewshed::CalculateVisibilityPercentage(const TArray<FS__ViewShedPoint> &ViewShedPoints)
{
    // Handle empty array case
    if (ViewShedPoints.Num() == 0)
    {
        return 0.0f;
    }

    // Count visible points
    int32 VisibleCount = 0;
    for (const FS__ViewShedPoint &Point : ViewShedPoints)
    {
        if (Point.bIsVisible)
        {
            VisibleCount++;
        }
    }

    // Calculate and return percentage
    return (float(VisibleCount) / float(ViewShedPoints.Num())) * 100.0f;
}

/**
 * Filter points by distance range
 */
TArray<FS__ViewShedPoint> UCPP_BPL__Viewshed::FilterPointsByDistance(
    const TArray<FS__ViewShedPoint> &ViewShedPoints,
    float MinDistance,
    float MaxDistance)
{
    TArray<FS__ViewShedPoint> FilteredPoints;

    // Iterate through all points and filter by distance
    for (const FS__ViewShedPoint &Point : ViewShedPoints)
    {
        // Check if point distance is within specified range
        if (Point.Distance >= MinDistance && Point.Distance <= MaxDistance)
        {
            // Add to filtered results
            FilteredPoints.Add(Point);
        }
    }

    return FilteredPoints;
}

/**
 * Extract only visible points from results
 */
TArray<FS__ViewShedPoint> UCPP_BPL__Viewshed::GetVisiblePoints(const TArray<FS__ViewShedPoint> &ViewShedPoints)
{
    TArray<FS__ViewShedPoint> VisiblePoints;

    // Filter for visible points only
    for (const FS__ViewShedPoint &Point : ViewShedPoints)
    {
        if (Point.bIsVisible)
        {
            VisiblePoints.Add(Point);
        }
    }

    return VisiblePoints;
}

/**
 * Extract only hidden points from results
 */
TArray<FS__ViewShedPoint> UCPP_BPL__Viewshed::GetHiddenPoints(const TArray<FS__ViewShedPoint> &ViewShedPoints)
{
    TArray<FS__ViewShedPoint> HiddenPoints;

    // Filter for hidden points only
    for (const FS__ViewShedPoint &Point : ViewShedPoints)
    {
        if (!Point.bIsVisible)
        {
            HiddenPoints.Add(Point);
        }
    }

    return HiddenPoints;
}

/**
 * Find the closest visible point to a reference location
 */
bool UCPP_BPL__Viewshed::FindClosestVisiblePoint(
    const TArray<FS__ViewShedPoint> &ViewShedPoints,
    FVector Location,
    FS__ViewShedPoint &FoundPoint)
{
    // Variables to track closest point
    bool bFoundAny = false;
    float ClosestDistanceSquared = FLT_MAX; // Use squared distance for performance

    // Search through all points
    for (const FS__ViewShedPoint &Point : ViewShedPoints)
    {
        // Only consider visible points
        if (Point.bIsVisible)
        {
            // Calculate squared distance to reference location
            float DistanceSquared = FVector::DistSquared(Point.WorldPosition, Location);

            // Check if this is the closest so far
            if (DistanceSquared < ClosestDistanceSquared)
            {
                // Update closest point tracking
                ClosestDistanceSquared = DistanceSquared;
                FoundPoint = Point;
                bFoundAny = true;
            }
        }
    }

    return bFoundAny;
}
