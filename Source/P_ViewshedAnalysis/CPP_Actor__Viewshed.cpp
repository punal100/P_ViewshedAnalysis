/*
 * @Author: Punal Manalan
 * @Description: ViewShed Analysis Plugin.
 * @Date: 04/10/2025
 */

#include "CPP_Actor__Viewshed.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"

/**
 * Constructor - Initialize default values and create components
 */
ACPP_Actor__Viewshed::ACPP_Actor__Viewshed()
{
    // Enable tick for this actor so we can update analysis over time
    PrimaryActorTick.bCanEverTick = true;

    // Create the root scene component to anchor everything
    USceneComponent *RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootComp;

    // Create Debug Instanced Static Mesh Component for visible points
    Debug_VisiblePointsISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("VisiblePointsISMC"));
    // Attach to root so it moves with the actor
    Debug_VisiblePointsISMC->SetupAttachment(RootComponent);
    // Disable collision since these are just visualization
    Debug_VisiblePointsISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    // Disable shadows for better performance
    Debug_VisiblePointsISMC->SetCastShadow(false);
    // Set Component Mobility to Movable
    Debug_VisiblePointsISMC->SetMobility(EComponentMobility::Movable);
    // Set Translation to World Absolute
    Debug_VisiblePointsISMC->SetUsingAbsoluteLocation(true);
    // Exclude from receiving decals (so our own decal doesn't paint our markers)
    Debug_VisiblePointsISMC->SetReceivesDecals(false);
    // Set Rotation to World Absolute
    Debug_VisiblePointsISMC->SetUsingAbsoluteRotation(true);

    // Create Debug Instanced Static Mesh Component for hidden points
    Debug_HiddenPointsISMC = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("HiddenPointsISMC"));
    // Attach to root so it moves with the actor
    Debug_HiddenPointsISMC->SetupAttachment(RootComponent);
    // Disable collision since these are just visualization
    Debug_HiddenPointsISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    // Disable shadows for better performance
    Debug_HiddenPointsISMC->SetCastShadow(false);
    // Set Component Mobility to Movable
    Debug_HiddenPointsISMC->SetMobility(EComponentMobility::Movable);
    // Set Translation to World Absolute
    Debug_HiddenPointsISMC->SetUsingAbsoluteLocation(true);
    // Exclude from receiving decals
    Debug_HiddenPointsISMC->SetReceivesDecals(false);
    // Set Rotation to World Absolute
    Debug_HiddenPointsISMC->SetUsingAbsoluteRotation(true);

    // Create Debug procedural mesh component, attach to root
    Debug_ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMeshComponent"));
    // Attach to root so it moves with the actor
    Debug_ProceduralMeshComponent->SetupAttachment(RootComponent);
    // Disable shadows for better performance
    Debug_ProceduralMeshComponent->SetCastShadow(false);
    // Use async cooking for performance
    Debug_ProceduralMeshComponent->bUseAsyncCooking = true;
    // Set Component Mobility to Movable
    Debug_ProceduralMeshComponent->SetMobility(EComponentMobility::Movable);
    // Set Translation to World Absolute
    Debug_ProceduralMeshComponent->SetUsingAbsoluteLocation(true);
    // Exclude from receiving decals
    Debug_ProceduralMeshComponent->SetReceivesDecals(false);

    // Create the hidden visualization decal component
    HiddenVisualizationDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("HiddenVisualizationDecal"));
    HiddenVisualizationDecalComponent->SetupAttachment(RootComponent);
    HiddenVisualizationDecalComponent->SetUsingAbsoluteLocation(true);
    HiddenVisualizationDecalComponent->SetUsingAbsoluteRotation(true);
    HiddenVisualizationDecalComponent->SetUsingAbsoluteScale(true);
    HiddenVisualizationDecalComponent->SetVisibility(true);
    // Ensure decal does not fade away on distance and renders above others by default
    HiddenVisualizationDecalComponent->FadeScreenSize = 0.0f;
    HiddenVisualizationDecalComponent->SortOrder = 100;
    // Reasonable default size; will be overridden each tick from FOV/MaxDistance
    HiddenVisualizationDecalComponent->DecalSize = FVector(1000.f, 500.f, 500.f);
    // Set Rotation to World Absolute
    Debug_ProceduralMeshComponent->SetUsingAbsoluteRotation(true);

    // Create Debug procedural mesh component, attach to root
    VisibleVisualization_ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("VisibleVisualization_ProceduralMeshComponent"));
    // Attach to root so it moves with the actor
    VisibleVisualization_ProceduralMeshComponent->SetupAttachment(RootComponent);
    // Disable shadows for better performance
    VisibleVisualization_ProceduralMeshComponent->SetCastShadow(false);
    // Use async cooking for performance
    VisibleVisualization_ProceduralMeshComponent->bUseAsyncCooking = true;
    // Set Component Mobility to Movable
    VisibleVisualization_ProceduralMeshComponent->SetMobility(EComponentMobility::Movable);
    // Set Translation to World Absolute
    VisibleVisualization_ProceduralMeshComponent->SetUsingAbsoluteLocation(true);
    // Exclude from receiving decals
    VisibleVisualization_ProceduralMeshComponent->SetReceivesDecals(false);

    // Initialize default property values
    // ViewDirection = FVector::ForwardVector; // Point forward along X-axis
    MaxDistance = 5000.0f;   // 50 meter range
    VerticalFOV = 60.0f;     // 60 degree vertical view
    HorizontalFOV = 90.0f;   // 90 degree horizontal view
    ObserverHeight = 150.0f; // 1.5 meter eye height

    // Set reasonable sampling resolution defaults
    DistanceSteps = 5; // 5 distance layers

    // Debug Visualization defaults
    Debug_PointScale = 1.0f;         // Normal size points
    bDebug_ShowVisiblePoints = true; // Show visible by default
    bDebug_ShowHiddenPoints = false; // Hide occluded by default
    bDebug_ShowLines = false;        // Debug lines off by default
    bDebug_LineDuration = 5.0f;      // 5 second debug lines
    bDebug_ShowPyramidBounds = true; // Show bounds by default

    // Analysis control defaults
    bAutoUpdate = true;     // Auto-update enabled
    UpdateInterval = 2.0f;  // Update every 2 seconds
    MaxTracesPerFrame = 50; // Max 50 traces per frame

    CachedHorizontalSampleCount = 0;
    CachedDistanceBandCount = 0;
}

/**
 * Called when the actor begins play
 * Set up components and start initial analysis if auto-update is enabled
 */
void ACPP_Actor__Viewshed::BeginPlay()
{
    // Call parent implementation
    Super::BeginPlay();

    // If no mesh is assigned, try to load default sphere mesh
    if (!Debug_VisiblePointMesh)
    {
        // Load the default engine sphere mesh for visualization
        Debug_VisiblePointMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    }

    // Apply the mesh to both ISMC components
    if (Debug_VisiblePointMesh)
    {
        // Set the mesh for visible points
        Debug_VisiblePointsISMC->SetStaticMesh(Debug_VisiblePointMesh);
        // Set the mesh for hidden points (same mesh, different material)
        Debug_HiddenPointsISMC->SetStaticMesh(Debug_VisiblePointMesh);
    }

    // Apply materials if they are assigned
    if (VisibleMaterial)
    {
        // Set material for visible points component
        Debug_VisiblePointsISMC->SetMaterial(0, VisibleMaterial);
    }

    if (HiddenMaterial)
    {
        // Set material for hidden points component
        Debug_HiddenPointsISMC->SetMaterial(0, HiddenMaterial);
    }

    // Initialize decal MID if a decal base material is provided
    if (HiddenVisualizationDecalComponent && HiddenVisualizationDecalMaterial)
    {
        HiddenVisualizationDecalMID = UMaterialInstanceDynamic::Create(HiddenVisualizationDecalMaterial, this);
        HiddenVisualizationDecalComponent->SetDecalMaterial(HiddenVisualizationDecalMID);
    }

    // Start initial analysis if auto-update is enabled
    if (bAutoUpdate)
    {
        // Begin the first analysis cycle
        StartAnalysis();
        // Record the current time as last update time
        LastUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

/**
 * Called every frame to update analysis progress and handle auto-updates
 */
void ACPP_Actor__Viewshed::Tick(float DeltaTime)
{
    // Call parent implementation first
    Super::Tick(DeltaTime);

    // Draw debug pyramid bounds if enabled
    if (bDebug_ShowPyramidBounds)
    {
        DrawDebugPyramid();
    }

    // Check if we should start a new analysis cycle
    if (bAutoUpdate && ShouldUpdateAnalysis())
    {
        // Start new analysis if not currently running
        if (!bAnalysisInProgress)
        {
            StartAnalysis();
        }
        // Update the last update time
        LastUpdateTime = GetWorld()->GetTimeSeconds();
    }

    // Process ongoing analysis if in progress
    if (bAnalysisInProgress)
    {
        // Track how many traces we've processed this frame
        int32 TracesProcessedThisFrame = 0;

        // Process traces up to the frame limit or until complete
        while (CurrentTraceIndex < TracePointQueue.Num() &&
               TracesProcessedThisFrame < MaxTracesPerFrame)
        {
            // Process the next trace in the queue
            ProcessSingleTrace(CurrentTraceIndex);
            // Move to next trace
            CurrentTraceIndex++;
            // Increment frame counter
            TracesProcessedThisFrame++;
        }

        // Check if analysis is complete
        if (CurrentTraceIndex >= TracePointQueue.Num())
        {
            // Mark analysis as complete
            bAnalysisInProgress = false;
            // Update visualization with new results
            UpdateVisualization();
            // Broadcast completion event to any listeners
            OnAnalysisComplete.Broadcast(AnalysisResults);
        }
    }

    // Keep decal aligned with the viewshed origin and frustum parameters every frame
    UpdateHiddenVisualizationDecal();
}

/**
 * Start a new viewshed analysis
 * Generates trace endpoints and begins processing
 */
void ACPP_Actor__Viewshed::StartAnalysis()
{
    // Don't start if already in progress
    if (bAnalysisInProgress)
    {
        return;
    }

    // Clear any existing results
    ClearResults();

    // Generate all trace start/end pairs based on the current sampling configuration
    GenerateTraceEndpoints();

    // If no traces were produced (e.g. degenerate sampling parameters) there is nothing to process
    if (TracePointQueue.IsEmpty())
    {
        return;
    }

    // Initialize the analysis results array to match the number of traces we will execute
    AnalysisResults.SetNum(TracePointQueue.Num());

    // Initialize each result with default values
    for (int32 i = 0; i < AnalysisResults.Num(); ++i)
    {
        const FS__ViewShedTracePoint &TracePoint = TracePointQueue[i];

        // Cache the endpoint so visualisation updates have the final sample position available
        AnalysisResults[i].WorldPosition = TracePoint.TraceEnd;
        // Calculate distance from observer to this point
        AnalysisResults[i].Distance = FVector::Dist(TracePoint.TraceStart, TracePoint.TraceEnd);
        // Initialize as not visible (will be updated during trace)
        AnalysisResults[i].bIsVisible = false;
        // Initialize hit location to endpoint (will be updated if hit occurs)
        AnalysisResults[i].HitLocation = TracePoint.TraceEnd;
        // Initialize hit normal from ground support if available (updated during trace if occluded)
        AnalysisResults[i].HitNormal = TracePoint.GroundNormal;
        // Initialize hit actor as null
        AnalysisResults[i].HitActor = nullptr;
    }

    // Mark analysis as in progress and reset trace index
    bAnalysisInProgress = true;
    CurrentTraceIndex = 0;
}

/**
 * Stop the current analysis if running
 */
void ACPP_Actor__Viewshed::StopAnalysis()
{
    // Simply mark analysis as not in progress
    bAnalysisInProgress = false;
    // Reset trace index for next analysis
    CurrentTraceIndex = 0;
}

/**
 * Clear all analysis results and visualization
 */
void ACPP_Actor__Viewshed::ClearResults()
{
    // Clear the results array
    AnalysisResults.Empty();
    // Clear hierarchical trace layout and flattened queue
    TraceSections.Empty();
    TracePointQueue.Empty();
    CachedHorizontalSampleCount = 0;
    CachedDistanceBandCount = 0;
    CachedVerticalSampleCount = 0;
    // Clear all visible point instances
    if (Debug_VisiblePointsISMC)
    {
        Debug_VisiblePointsISMC->ClearInstances();
    }
    // Clear all hidden point instances
    if (Debug_HiddenPointsISMC)
    {
        Debug_HiddenPointsISMC->ClearInstances();
    }
    // Clear Procedural Mesh
    if (Debug_ProceduralMeshComponent)
    {
        Debug_ProceduralMeshComponent->ClearAllMeshSections();
    }
}

/**
 * Generate all trace endpoints in a pyramid sampling pattern
 * Creates a grid of points within the FOV bounds at multiple distances
 */
void ACPP_Actor__Viewshed::GenerateTraceEndpoints()
{
    // Reset previously generated sections and flattened queue
    TraceSections.Empty();
    TracePointQueue.Empty();
    CachedHorizontalSampleCount = 0;
    CachedDistanceBandCount = 0;
    CachedVerticalSampleCount = 0;

    UWorld *World = GetWorld();
    if (!World)
    {
        return;
    }

    const FVector ObserverLoc = GetObserverLocation();
    const FVector UpVector = GetActorUpVector().GetSafeNormal();
    const FVector ForwardVector = GetActorForwardVector().GetSafeNormal();
    FVector RightVector = GetActorRightVector().GetSafeNormal();
    // Ensure basis is orthonormal
    RightVector = FVector::CrossProduct(UpVector, ForwardVector).GetSafeNormal();
    const FVector TrueForward = FVector::CrossProduct(RightVector, UpVector).GetSafeNormal();

    // Convert half-angle FOV values now to avoid repeated work inside the loops
    const float HalfHorizontalRad = FMath::DegreesToRadians(FMath::Max(1e-3f, HorizontalFOV * 0.5f));
    const float HalfVerticalRad = FMath::DegreesToRadians(FMath::Max(1e-3f, VerticalFOV * 0.5f));

    // Derive section counts from user-provided ratios
    const float SafeHRatio = FMath::Clamp(Horizontal_Sample_Section_Ratio, 0.01f, 1.0f);
    const float SafeVRatio = FMath::Clamp(Vertical_Sample_Section_Ratio, 0.01f, 1.0f);
    const int32 HorizontalSectionCount = FMath::Max(1, FMath::CeilToInt(1.0f / SafeHRatio));
    const int32 VerticalSectionCount = FMath::Max(1, FMath::CeilToInt(1.0f / SafeVRatio));

    const int32 EffectiveDistanceSteps = FMath::Max(1, DistanceSteps);

    // Determine a consistent horizontal sample count based on the far-plane arc length and desired spacing
    const float MaxArcWidth = 2.0f * MaxDistance * FMath::Tan(HalfHorizontalRad);
    const float DesiredSpacing = FMath::Max(1.0f, Maximum_Distance_Between_Samples);
    int32 HorizontalSampleCount = FMath::CeilToInt(MaxArcWidth / DesiredSpacing) + 1;
    // Ensure at least Minimum_Samples_Per_Section per horizontal section
    HorizontalSampleCount = FMath::Max(HorizontalSampleCount, HorizontalSectionCount * FMath::Max(1, Minimum_Samples_Per_Section));
    // Round up to a multiple of sections so each section has (roughly) equal columns
    if (HorizontalSampleCount % HorizontalSectionCount != 0)
    {
        HorizontalSampleCount += HorizontalSectionCount - (HorizontalSampleCount % HorizontalSectionCount);
    }
    CachedHorizontalSampleCount = HorizontalSampleCount;
    CachedDistanceBandCount = EffectiveDistanceSteps;

    // Determine vertical sample count similar to horizontal, based on far-plane height
    const float MaxArcHeight = 2.0f * MaxDistance * FMath::Tan(HalfVerticalRad);
    int32 VerticalSampleCount = FMath::CeilToInt(MaxArcHeight / DesiredSpacing) + 1;
    // For vertical we keep a modest minimum per section to avoid exploding sample counts
    VerticalSampleCount = FMath::Max(VerticalSampleCount, VerticalSectionCount * 1);
    if (VerticalSampleCount % VerticalSectionCount != 0)
    {
        VerticalSampleCount += VerticalSectionCount - (VerticalSampleCount % VerticalSectionCount);
    }

    // Cache counts
    TraceSections.SetNum(EffectiveDistanceSteps);
    CachedVerticalSampleCount = VerticalSampleCount;
    // Reserve for all vertical rows; first rows will be the central vertical slice to keep existing mesh assumptions intact
    TracePointQueue.Reserve(EffectiveDistanceSteps * HorizontalSampleCount * VerticalSampleCount);

    // No ground probing (ground-hugging removed)

    for (int32 DistStep = 0; DistStep < EffectiveDistanceSteps; ++DistStep)
    {
        const float StepFraction = float(DistStep + 1) / float(EffectiveDistanceSteps);
        const float CurrentDistance = MaxDistance * StepFraction;

        FS__ViewShedTraceSection &DistanceSection = TraceSections[DistStep];
        DistanceSection.HorizontalSectionCount = HorizontalSectionCount;
        DistanceSection.VerticalSectionCount = VerticalSectionCount;
        DistanceSection.TraceSections.SetNum(1);

        FS__ViewShedTraceEndPoints &SectionPoints = DistanceSection.TraceSections[0];
        SectionPoints.HorizontalSampleCount = HorizontalSampleCount;
        SectionPoints.VerticalSampleCount = VerticalSampleCount;
        SectionPoints.TraceEndPoints.Reset(HorizontalSampleCount * VerticalSampleCount);

        // Pass 1: Generate the central vertical slice first (pitch = 0) so existing visible blanket (which assumes 2D grid) stays correct.
        const int32 CentralVerticalIndex = FMath::Clamp(VerticalSampleCount / 2, 0, FMath::Max(0, VerticalSampleCount - 1));
        const float CentralVerticalAngle = 0.0f; // middle row (no pitch)

        for (int32 HorizontalIndex = 0; HorizontalIndex < HorizontalSampleCount; ++HorizontalIndex)
        {
            const float HorizontalAlpha = (HorizontalSampleCount <= 1)
                                              ? 0.5f
                                              : float(HorizontalIndex) / float(HorizontalSampleCount - 1);
            const float HorizontalAngle = FMath::Lerp(-HalfHorizontalRad, HalfHorizontalRad, HorizontalAlpha);

            // Build direction using yaw (horizontal) and pitch (central vertical row)
            const FQuat Yaw(UpVector, HorizontalAngle);
            const FQuat Pitch(RightVector, CentralVerticalAngle);
            const FVector Direction = (Yaw * Pitch).RotateVector(TrueForward).GetSafeNormal();

            const FVector PointOnFrustum = ObserverLoc + Direction * CurrentDistance;
            FVector TargetLocation = PointOnFrustum; // visibility determined by occluder before reaching this point
            const bool bGroundHit = true;            // treat as supported; we no longer depend on ground probing
            const FVector GroundNormal = FVector::ZeroVector;

            FS__ViewShedTracePoint TracePoint;
            TracePoint.TraceStart = ObserverLoc;
            TracePoint.TraceEnd = TargetLocation;
            TracePoint.DistanceBandIndex = DistStep;
            TracePoint.HorizontalSampleIndex = HorizontalIndex;
            TracePoint.VerticalSampleIndex = CentralVerticalIndex;
            TracePoint.bHasGroundSupport = bGroundHit;
            TracePoint.GroundNormal = GroundNormal;

            SectionPoints.TraceEndPoints.Add(TracePoint);
            TracePointQueue.Add(TracePoint);
        }

        // Pass 2: Generate remaining vertical rows (bottom to top), skipping the central index already added
        for (int32 VerticalIndex = 0; VerticalIndex < VerticalSampleCount; ++VerticalIndex)
        {
            if (VerticalIndex == CentralVerticalIndex)
            {
                continue;
            }
            const float VerticalAlpha = (VerticalSampleCount <= 1) ? 0.5f : float(VerticalIndex) / float(VerticalSampleCount - 1);
            const float VerticalAngle = FMath::Lerp(-HalfVerticalRad, HalfVerticalRad, VerticalAlpha);

            for (int32 HorizontalIndex = 0; HorizontalIndex < HorizontalSampleCount; ++HorizontalIndex)
            {
                const float HorizontalAlpha = (HorizontalSampleCount <= 1)
                                                  ? 0.5f
                                                  : float(HorizontalIndex) / float(HorizontalSampleCount - 1);
                const float HorizontalAngle = FMath::Lerp(-HalfHorizontalRad, HalfHorizontalRad, HorizontalAlpha);

                const FQuat Yaw(UpVector, HorizontalAngle);
                const FQuat Pitch(RightVector, VerticalAngle);
                const FVector Direction = (Yaw * Pitch).RotateVector(TrueForward).GetSafeNormal();

                const FVector PointOnFrustum = ObserverLoc + Direction * CurrentDistance;
                FVector TargetLocation = PointOnFrustum;

                FS__ViewShedTracePoint TracePoint;
                TracePoint.TraceStart = ObserverLoc;
                TracePoint.TraceEnd = TargetLocation;
                TracePoint.DistanceBandIndex = DistStep;
                TracePoint.HorizontalSampleIndex = HorizontalIndex;
                TracePoint.VerticalSampleIndex = VerticalIndex;
                TracePoint.bHasGroundSupport = true;
                TracePoint.GroundNormal = FVector::ZeroVector;

                SectionPoints.TraceEndPoints.Add(TracePoint);
                TracePointQueue.Add(TracePoint);
            }
        }
    }
}

/**
 * Process a single line trace by index
 * Performs collision detection and updates result data
 */
void ACPP_Actor__Viewshed::ProcessSingleTrace(int32 TraceIndex)
{
    // Validate input parameters
    if (!IsValid(GetWorld()) || TraceIndex >= TracePointQueue.Num() || TraceIndex >= AnalysisResults.Num())
    {
        return;
    }

    // Get observer and target locations
    const FS__ViewShedTracePoint &TracePoint = TracePointQueue[TraceIndex];
    const FVector ObserverLoc = TracePoint.TraceStart;
    const FVector TargetLoc = TracePoint.TraceEnd;

    // Set up collision query parameters
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this); // Ignore self to avoid self-collision
    QueryParams.bTraceComplex = false; // Use simple collision for performance

    const FVector TraceVector = TargetLoc - ObserverLoc;
    const float TraceLength = TraceVector.Size();

    // Perform the line trace
    FHitResult HitResult;
    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,      // Output hit result
        ObserverLoc,    // Start location
        TargetLoc,      // End location
        ECC_Visibility, // Collision channel (visibility)
        QueryParams     // Query parameters
    );

    const float DistanceTolerance = 5.0f;

    if (!bHit)
    {
        // Nothing blocked the view all the way to the intended ground position
        AnalysisResults[TraceIndex].bIsVisible = true;
        AnalysisResults[TraceIndex].HitLocation = TargetLoc;
        AnalysisResults[TraceIndex].HitNormal = TracePoint.GroundNormal;
        AnalysisResults[TraceIndex].HitActor = nullptr;
    }
    else if (TraceLength <= KINDA_SMALL_NUMBER)
    {
        // Degenerate trace (observer origin) - treat as visible anchor
        AnalysisResults[TraceIndex].bIsVisible = true;
        AnalysisResults[TraceIndex].HitLocation = TargetLoc;
        AnalysisResults[TraceIndex].HitNormal = TracePoint.GroundNormal;
        AnalysisResults[TraceIndex].HitActor = HitResult.GetActor();
    }
    else
    {
        const float HitDistance = (HitResult.Location - ObserverLoc).Size();
        const bool bReachedTarget = FMath::IsNearlyEqual(HitDistance, TraceLength, DistanceTolerance) || HitDistance > TraceLength;

        if (bReachedTarget)
        {
            // Reached near the intended endpoint, but we still have a concrete surface from the trace
            AnalysisResults[TraceIndex].bIsVisible = true;
            AnalysisResults[TraceIndex].HitLocation = HitResult.Location; // use the actual surface contact point
            AnalysisResults[TraceIndex].HitNormal = TracePoint.GroundNormal.IsNearlyZero() ? HitResult.Normal : TracePoint.GroundNormal;
            AnalysisResults[TraceIndex].HitActor = HitResult.GetActor();
        }
        else
        {
            // Something obstructed the path before reaching the target
            AnalysisResults[TraceIndex].bIsVisible = false;
            AnalysisResults[TraceIndex].HitLocation = HitResult.Location;
            AnalysisResults[TraceIndex].HitNormal = HitResult.Normal;
            AnalysisResults[TraceIndex].HitActor = HitResult.GetActor();
        }
    }

    // Ground support no longer affects visibility; only occluder hits vs reaching the target matters

    // Draw debug line if enabled
    if (bDebug_ShowLines)
    {
        // Choose color based on visibility
        FColor LineColor = AnalysisResults[TraceIndex].bIsVisible ? FColor::Green : FColor::Red;
        // Draw line from observer to hit location (not necessarily endpoint)
        DrawDebugLine(GetWorld(), ObserverLoc, AnalysisResults[TraceIndex].HitLocation,
                      LineColor, false, bDebug_LineDuration, 0, 2.0f);
    }
}

/**
 * Build Debug Point Mesh
 */
void ACPP_Actor__Viewshed::
    BuildDebug_PointMesh()
{
    const FVector ObserverLoc = GetObserverLocation();
    // Clear existing debug points (defensive)
    if (Debug_VisiblePointsISMC)
    {
        Debug_VisiblePointsISMC->ClearInstances();
    }
    if (Debug_HiddenPointsISMC)
    {
        Debug_HiddenPointsISMC->ClearInstances();
    }

    // Use instanced mesh visualization as before
    // Process each analysis result
    for (const FS__ViewShedPoint &Point : AnalysisResults)
    {
        // Create transform for this instance
        FTransform InstanceTransform;
        // Position relative to the viewshed origin (slightly offset upward for visibility)
        InstanceTransform.SetLocation((Point.WorldPosition - ObserverLoc) + FVector(0, 0, 10));
        // Set uniform scale based on point scale setting
        InstanceTransform.SetScale3D(FVector(Debug_PointScale, Debug_PointScale, Debug_PointScale));
        // Use default rotation (no rotation needed for spheres)
        InstanceTransform.SetRotation(FQuat::Identity);

        // Add to appropriate component based on visibility and display settings
        if (Point.bIsVisible && bDebug_ShowVisiblePoints)
        {
            // Add to visible points component (defensive)
            if (Debug_VisiblePointsISMC)
            {
                Debug_VisiblePointsISMC->AddInstance(InstanceTransform);
            }
        }
        else if (!Point.bIsVisible && bDebug_ShowHiddenPoints)
        {
            // Add to hidden points component (defensive)
            if (Debug_HiddenPointsISMC)
            {
                Debug_HiddenPointsISMC->AddInstance(InstanceTransform);
            }
        }
    }
}

/**
 * Build Debug Procedural Merged Mesh
 */
void ACPP_Actor__Viewshed::BuildDebug_ProceduralMergedMesh()
{
    if (!Debug_ProceduralMeshComponent)
    {
        return;
    }

    Debug_ProceduralMeshComponent->ClearAllMeshSections();

    if (AnalysisResults.IsEmpty())
    {
        return;
    }
    // TODO Invisible points should have HiddenMaterial
}

/**
 * Build Visible Visualization Procedural Merged Mesh
 */
void ACPP_Actor__Viewshed::BuildVisibleVisualization_ProceduralMergedMesh()
{
    if (!VisibleVisualization_ProceduralMeshComponent)
    {
        return;
    }

    VisibleVisualization_ProceduralMeshComponent->ClearAllMeshSections();

    if (AnalysisResults.IsEmpty())
    {
        return;
    }

    const FVector ObserverLoc = GetObserverLocation();
    const float QuadHalfSize = FMath::Max(1.0f, VisibleVisualization_QuadHalfSize);
    const FTransform ComponentTransform = VisibleVisualization_ProceduralMeshComponent->GetComponentTransform();
    const FTransform WorldToComponent = ComponentTransform.Inverse();

    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FLinearColor> VertexColors;
    TArray<FProcMeshTangent> Tangents;
    Vertices.Reserve(AnalysisResults.Num() * 8);
    Normals.Reserve(AnalysisResults.Num() * 8);
    UVs.Reserve(AnalysisResults.Num() * 8);
    VertexColors.Reserve(AnalysisResults.Num() * 8);
    Tangents.Reserve(AnalysisResults.Num() * 8);
    Triangles.Reserve(AnalysisResults.Num() * 12);

    const FLinearColor VisibleColor(0.0f, 1.0f, 0.0f, 1.0f);
    const FLinearColor HiddenColor(1.0f, 0.0f, 0.0f, 1.0f);

    const FVector2D QuadUVs[4] = {
        FVector2D(1.0f, 1.0f),
        FVector2D(0.0f, 1.0f),
        FVector2D(0.0f, 0.0f),
        FVector2D(1.0f, 0.0f)};

    for (const FS__ViewShedPoint &Point : AnalysisResults)
    {
        // Only place geometry where we actually hit a surface (visible or occluded)
        const bool bHasRealHit = (Point.HitActor != nullptr) || (!Point.HitNormal.IsNearlyZero());
        if (!bHasRealHit)
        {
            continue;
        }

        // Choose surface normal from hit; fallback points back to observer
        FVector SurfaceNormal = Point.HitNormal;
        if (!SurfaceNormal.Normalize())
        {
            SurfaceNormal = (ObserverLoc - Point.HitLocation).GetSafeNormal();
            if (!SurfaceNormal.Normalize())
            {
                SurfaceNormal = FVector::UpVector;
            }
        }

        // Lift slightly off the surface along the surface normal
        const FVector BaseWorldPosition = Point.HitLocation + SurfaceNormal * VisibleVisualization_SurfaceOffset;

        // Build a tangent frame on the surface
        FVector TangentX, TangentY;
        SurfaceNormal.FindBestAxisVectors(TangentX, TangentY);
        const FVector TangentDir = TangentX.GetSafeNormal();
        TangentX = TangentX.GetSafeNormal() * QuadHalfSize;
        TangentY = TangentY.GetSafeNormal() * QuadHalfSize;

        FVector LocalNormal = WorldToComponent.TransformVectorNoScale(SurfaceNormal).GetSafeNormal();
        if (LocalNormal.IsNearlyZero())
        {
            LocalNormal = FVector::UpVector;
        }

        FVector LocalTangentDir = WorldToComponent.TransformVectorNoScale(TangentDir).GetSafeNormal();
        if (LocalTangentDir.IsNearlyZero())
        {
            LocalTangentDir = FVector::ForwardVector;
        }

        FVector OffsetCorners[4] = {
            TangentX + TangentY,
            -TangentX + TangentY,
            -TangentX - TangentY,
            TangentX - TangentY};

        const int32 FrontBaseIndex = Vertices.Num();
        for (int32 CornerIdx = 0; CornerIdx < 4; ++CornerIdx)
        {
            const FVector WorldPosition = BaseWorldPosition + OffsetCorners[CornerIdx];
            Vertices.Add(WorldToComponent.TransformPosition(WorldPosition));
            Normals.Add(LocalNormal);
            UVs.Add(QuadUVs[CornerIdx]);
            VertexColors.Add(Point.bIsVisible ? VisibleColor : HiddenColor);
            Tangents.Add(FProcMeshTangent(LocalTangentDir, false));
        }

        const int32 BackBaseIndex = Vertices.Num();
        for (int32 CornerIdx = 0; CornerIdx < 4; ++CornerIdx)
        {
            const FVector WorldPosition = BaseWorldPosition + OffsetCorners[CornerIdx];
            Vertices.Add(WorldToComponent.TransformPosition(WorldPosition));
            Normals.Add(-LocalNormal);
            UVs.Add(QuadUVs[CornerIdx]);
            VertexColors.Add(Point.bIsVisible ? VisibleColor : HiddenColor);
            Tangents.Add(FProcMeshTangent(-LocalTangentDir, false));
        }

        // Front face
        Triangles.Add(FrontBaseIndex + 0);
        Triangles.Add(FrontBaseIndex + 1);
        Triangles.Add(FrontBaseIndex + 2);

        Triangles.Add(FrontBaseIndex + 0);
        Triangles.Add(FrontBaseIndex + 2);
        Triangles.Add(FrontBaseIndex + 3);

        // Back face (reverse winding)
        Triangles.Add(BackBaseIndex + 0);
        Triangles.Add(BackBaseIndex + 2);
        Triangles.Add(BackBaseIndex + 1);

        Triangles.Add(BackBaseIndex + 0);
        Triangles.Add(BackBaseIndex + 3);
        Triangles.Add(BackBaseIndex + 2);
    }

    if (Triangles.Num() == 0 || Vertices.Num() == 0)
    {
        return;
    }

    VisibleVisualization_ProceduralMeshComponent->CreateMeshSection_LinearColor(
        0,
        Vertices,
        Triangles,
        Normals,
        UVs,
        {},
        {},
        {},
        VertexColors,
        Tangents,
        false,
        false);

    if (VisibleMaterial)
    {
        VisibleVisualization_ProceduralMeshComponent->SetMaterial(0, VisibleMaterial);
    }
}

/**
 * Update visualization based on current analysis results
 * Clears existing instances and creates new ones based on visibility
 */
void ACPP_Actor__Viewshed::UpdateVisualization()
{
    // Anchor components at the viewshed origin so they rotate around the correct pivot
    const FVector ObserverLoc = GetObserverLocation();
    const FVector ActorLocation = GetActorLocation();
    const FRotator IdentityRot = FRotator::ZeroRotator;
    const FRotator ActorRotation = GetActorRotation();
    if (Debug_VisiblePointsISMC)
    {
        Debug_VisiblePointsISMC->SetUsingAbsoluteLocation(true);
        Debug_VisiblePointsISMC->SetUsingAbsoluteRotation(true);
        Debug_VisiblePointsISMC->SetWorldLocation(ObserverLoc);
        Debug_VisiblePointsISMC->SetWorldRotation(IdentityRot);
    }
    if (Debug_HiddenPointsISMC)
    {
        Debug_HiddenPointsISMC->SetUsingAbsoluteLocation(true);
        Debug_HiddenPointsISMC->SetUsingAbsoluteRotation(true);
        Debug_HiddenPointsISMC->SetWorldLocation(ObserverLoc);
        Debug_HiddenPointsISMC->SetWorldRotation(IdentityRot);
    }
    if (Debug_ProceduralMeshComponent)
    {
        Debug_ProceduralMeshComponent->SetUsingAbsoluteLocation(true);
        Debug_ProceduralMeshComponent->SetUsingAbsoluteRotation(true);
        Debug_ProceduralMeshComponent->SetWorldLocation(ObserverLoc);
        Debug_ProceduralMeshComponent->SetWorldRotation(IdentityRot);
    }
    if (VisibleVisualization_ProceduralMeshComponent)
    {
        VisibleVisualization_ProceduralMeshComponent->SetUsingAbsoluteLocation(true);
        VisibleVisualization_ProceduralMeshComponent->SetUsingAbsoluteRotation(true);
        VisibleVisualization_ProceduralMeshComponent->SetWorldLocation(ActorLocation);
        VisibleVisualization_ProceduralMeshComponent->SetWorldRotation(ActorRotation);
    }

    // Clear existing instances from both components (defensive checks)
    if (Debug_VisiblePointsISMC)
        Debug_VisiblePointsISMC->ClearInstances();
    if (Debug_HiddenPointsISMC)
        Debug_HiddenPointsISMC->ClearInstances();
    if (Debug_ProceduralMeshComponent)
        Debug_ProceduralMeshComponent->ClearAllMeshSections();
    if (VisibleVisualization_ProceduralMeshComponent)
        VisibleVisualization_ProceduralMeshComponent->ClearAllMeshSections();

    if (bDebug_ShowDebugVisualization)
    {
        if (bDebug_UseProceduralMesh)
        {
            // Build and update the merged procedural mesh from current points
            BuildDebug_ProceduralMergedMesh();
        }
        else
        {
            // Build and update the point mesh from current points
            BuildDebug_PointMesh();
        }
    }

    BuildVisibleVisualization_ProceduralMergedMesh();
}

/**
 * Get the world position of the observer (actor location + height offset)
 */
FVector ACPP_Actor__Viewshed::GetObserverLocation() const
{
    // Return actor location with vertical offset for observer height
    return GetActorLocation() + FVector(0, 0, ObserverHeight);
}

/**
 * Draw debug visualization showing the viewshed pyramid bounds
 */
void ACPP_Actor__Viewshed::DrawDebugPyramid() const
{
    // Skip if no world available
    if (!GetWorld())
    {
        return;
    }

    // Get observer location and view parameters
    FVector ObserverLoc = GetObserverLocation();
    // Find a valid up vector for the basis.
    // If ViewDirection is close to UpVector (i.e., looking straight up or down), use ForwardVector as up instead.
    FVector ForwardDir = GetActorForwardVector();
    // FVector ArbitraryUp = (FMath::Abs(ForwardDir.Z) > 0.99f) ? FVector::ForwardVector : FVector::UpVector;
    //  Create orthogonal basis vectors for the pyramid
    FVector RightDir = GetActorRightVector();
    FVector UpDir = GetActorUpVector();

    // Convert FOV to radians
    float HalfVertRad = FMath::DegreesToRadians(VerticalFOV * 0.5f);
    float HalfHorzRad = FMath::DegreesToRadians(HorizontalFOV * 0.5f);

    // Calculate pyramid corner directions
    TArray<FVector> CornerDirections;
    CornerDirections.Add(CalculateDirectionFromAngles(-HalfHorzRad, -HalfVertRad)); // Bottom left
    CornerDirections.Add(CalculateDirectionFromAngles(HalfHorzRad, -HalfVertRad));  // Bottom right
    CornerDirections.Add(CalculateDirectionFromAngles(HalfHorzRad, HalfVertRad));   // Top right
    CornerDirections.Add(CalculateDirectionFromAngles(-HalfHorzRad, HalfVertRad));  // Top left

    // Calculate corner positions at max distance
    TArray<FVector> CornerPositions;
    for (const FVector &Direction : CornerDirections)
    {
        CornerPositions.Add(ObserverLoc + Direction * MaxDistance);
    }

    // Draw pyramid edges (from observer to corners)
    for (const FVector &CornerPos : CornerPositions)
    {
        DrawDebugLine(GetWorld(), ObserverLoc, CornerPos, FColor::Magenta, false, -1, 0, 3.0f);
    }

    // Draw pyramid face edges (connecting corners)
    for (int32 i = 0; i < 4; ++i)
    {
        int32 NextIndex = (i + 1) % 4;
        DrawDebugLine(GetWorld(), CornerPositions[i], CornerPositions[NextIndex],
                      FColor::Magenta, false, -1, 0, 2.0f);
    }

    // Draw center line (view direction)
    FVector CenterEnd = ObserverLoc + ForwardDir * MaxDistance;
    DrawDebugLine(GetWorld(), ObserverLoc, CenterEnd, FColor::Yellow, false, -1, 0, 4.0f);
}

void ACPP_Actor__Viewshed::UpdateHiddenVisualizationDecal()
{
    if (!HiddenVisualizationDecalComponent)
    {
        return;
    }

    const FVector Origin = GetObserverLocation();
    const FVector Forward = GetActorForwardVector().GetSafeNormal();
    FVector Up = GetActorUpVector().GetSafeNormal();
    const FVector Right = GetActorRightVector().GetSafeNormal();
    // Re-orthonormalize Up in case of near-colinearity with Forward
    Up = FVector::CrossProduct(Right, Forward).GetSafeNormal();

    // Align decal so X+ projects forward
    const FRotator DecalRot = FRotationMatrix::MakeFromXZ(Forward, Up).Rotator();
    // Place the projector mid-way along the frustum depth so the box spans from origin to far plane
    // UE decals use a box centered on the component location; X is depth (half-size), Y/Z are half extents
    const float HalfDepth = MaxDistance * 0.5f;
    HiddenVisualizationDecalComponent->SetWorldLocation(Origin + Forward * HalfDepth);
    HiddenVisualizationDecalComponent->SetWorldRotation(DecalRot);

    // Size decal to encompass the frustum at MaxDistance
    const float HalfH = FMath::DegreesToRadians(HorizontalFOV * 0.5f);
    const float HalfV = FMath::DegreesToRadians(VerticalFOV * 0.5f);
    const float HalfWidthAtFar = MaxDistance * FMath::Tan(HalfH);
    const float HalfHeightAtFar = MaxDistance * FMath::Tan(HalfV);
    // Slightly pad the projector to avoid edge clipping
    const float Pad = 1.02f;
    HiddenVisualizationDecalComponent->DecalSize = FVector(HalfDepth * Pad, HalfWidthAtFar * Pad, HalfHeightAtFar * Pad);

    // Feed runtime parameters to the decal material (shader should test frustum and normal dot)
    if (HiddenVisualizationDecalMID)
    {
        HiddenVisualizationDecalMID->SetScalarParameterValue(TEXT("VS_MaxDistance"), MaxDistance);
        HiddenVisualizationDecalMID->SetScalarParameterValue(TEXT("VS_VertFOVDeg"), VerticalFOV);
        HiddenVisualizationDecalMID->SetScalarParameterValue(TEXT("VS_HorizFOVDeg"), HorizontalFOV);
        HiddenVisualizationDecalMID->SetVectorParameterValue(TEXT("VS_Origin"), FLinearColor(Origin));
        HiddenVisualizationDecalMID->SetVectorParameterValue(TEXT("VS_Forward"), FLinearColor(Forward));
        HiddenVisualizationDecalMID->SetVectorParameterValue(TEXT("VS_Right"), FLinearColor(Right));
        HiddenVisualizationDecalMID->SetVectorParameterValue(TEXT("VS_Up"), FLinearColor(Up));
        // Additional tunables for the decal material
        HiddenVisualizationDecalMID->SetScalarParameterValue(TEXT("VS_NormalThreshold"), VS_NormalThreshold);
        HiddenVisualizationDecalMID->SetScalarParameterValue(TEXT("VS_FrustumFeather"), VS_FrustumFeather);
        HiddenVisualizationDecalMID->SetScalarParameterValue(TEXT("VS_FacingFeather"), VS_FacingFeather);
        HiddenVisualizationDecalMID->SetScalarParameterValue(TEXT("VS_FacingEnabled"), VS_FacingEnabled);
        HiddenVisualizationDecalMID->SetVectorParameterValue(TEXT("VS_ColorInside"), VS_ColorInside);
        HiddenVisualizationDecalMID->SetVectorParameterValue(TEXT("VS_ColorOutside"), VS_ColorOutside);
        HiddenVisualizationDecalMID->SetScalarParameterValue(TEXT("VS_GridIntensity"), VS_GridIntensity);
        HiddenVisualizationDecalMID->SetScalarParameterValue(TEXT("VS_Opacity"), VS_Opacity);
    }
}

/**
 * Calculate a world direction vector from horizontal and vertical angles
 * Accounts for the view direction and creates proper pyramid sampling
 */
FVector ACPP_Actor__Viewshed::CalculateDirectionFromAngles(float HorizontalAngle, float VerticalAngle) const
{
    // Always use a robust basis:
    // Find a valid up vector for the basis.
    // If ViewDirection is close to UpVector (i.e., looking straight up or down), use ForwardVector as up instead.
    FVector ForwardDir = GetActorForwardVector();
    // FVector ArbitraryUp = (FMath::Abs(ForwardDir.Z) > 0.99f) ? FVector::ForwardVector : FVector::UpVector;
    //  Create orthogonal basis vectors for the pyramid
    FVector RightDir = GetActorRightVector();
    FVector UpDir = GetActorUpVector();

    // Start from Forward, rotate horizontally around Up (yaw), then vertically around Right (pitch)
    FVector Direction = ForwardDir;

    FQuat HorizontalRotation = FQuat(UpDir, HorizontalAngle);
    Direction = HorizontalRotation.RotateVector(Direction);

    FVector RotatedRightDir = FVector::CrossProduct(Direction, UpDir).GetSafeNormal();
    FQuat VerticalRotation = FQuat(RotatedRightDir, VerticalAngle);
    Direction = VerticalRotation.RotateVector(Direction);

    return Direction.GetSafeNormal();
}

/**
 * Check if analysis should be updated based on time interval
 */
bool ACPP_Actor__Viewshed::ShouldUpdateAnalysis() const
{
    // Check if enough time has passed since last update
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastUpdateTime) >= UpdateInterval;
}

/**
 * Get number of visible points in current analysis
 */
int32 ACPP_Actor__Viewshed::GetVisiblePointCount() const
{
    int32 Count = 0;
    // Count all visible points
    for (const FS__ViewShedPoint &Point : AnalysisResults)
    {
        if (Point.bIsVisible)
        {
            Count++;
        }
    }
    return Count;
}

/**
 * Get number of hidden points in current analysis
 */
int32 ACPP_Actor__Viewshed::GetHiddenPointCount() const
{
    int32 Count = 0;
    // Count all hidden points
    for (const FS__ViewShedPoint &Point : AnalysisResults)
    {
        if (!Point.bIsVisible)
        {
            Count++;
        }
    }
    return Count;
}

/**
 * Get visibility percentage (0-100)
 */
float ACPP_Actor__Viewshed::GetVisibilityPercentage() const
{
    // Avoid division by zero
    if (AnalysisResults.Num() == 0)
    {
        return 0.0f;
    }

    // Calculate percentage of visible points
    int32 VisibleCount = GetVisiblePointCount();
    return (float(VisibleCount) / float(AnalysisResults.Num())) * 100.0f;
}
