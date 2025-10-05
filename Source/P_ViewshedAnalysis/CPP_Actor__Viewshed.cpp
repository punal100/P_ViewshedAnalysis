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
    // Set Rotation to World Absolute
    Debug_ProceduralMeshComponent->SetUsingAbsoluteRotation(true);

    // Initialize default property values
    // ViewDirection = FVector::ForwardVector; // Point forward along X-axis
    MaxDistance = 5000.0f;   // 50 meter range
    VerticalFOV = 60.0f;     // 60 degree vertical view
    HorizontalFOV = 90.0f;   // 90 degree horizontal view
    ObserverHeight = 150.0f; // 1.5 meter eye height

    // Set reasonable sampling resolution
    HorizontalSamples = 20; // 20 samples horizontally
    VerticalSamples = 15;   // 15 samples vertically
    DistanceSteps = 5;      // 5 distance layers

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
        while (CurrentTraceIndex < TraceEndpoints.Num() &&
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
        if (CurrentTraceIndex >= TraceEndpoints.Num())
        {
            // Mark analysis as complete
            bAnalysisInProgress = false;
            // Update visualization with new results
            UpdateVisualization();
            // Broadcast completion event to any listeners
            OnAnalysisComplete.Broadcast(AnalysisResults);
        }
    }
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

    // Generate all the trace endpoints in pyramid pattern
    GenerateTraceEndpoints();

    // Initialize the analysis results array
    AnalysisResults.SetNum(TraceEndpoints.Num());

    // Initialize each result with default values
    for (int32 i = 0; i < AnalysisResults.Num(); ++i)
    {
        // Set the world position from trace endpoints
        AnalysisResults[i].WorldPosition = TraceEndpoints[i];
        // Calculate distance from observer to this point
        AnalysisResults[i].Distance = FVector::Dist(GetObserverLocation(), TraceEndpoints[i]);
        // Initialize as not visible (will be updated during trace)
        AnalysisResults[i].bIsVisible = false;
        // Initialize hit location to endpoint (will be updated if hit occurs)
        AnalysisResults[i].HitLocation = TraceEndpoints[i];
        // Initialize hit normal to zero until a hit provides a value
        AnalysisResults[i].HitNormal = FVector::ZeroVector;
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
    // Clear trace endpoints array
    TraceEndpoints.Empty();
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
    // Clear existing endpoints
    TraceEndpoints.Empty();

    // Get observer location as starting point
    FVector ObserverLoc = GetObserverLocation();

    // Find a valid up vector for the basis.
    // If ViewDirection is close to UpVector (i.e., looking straight up or down), use ForwardVector as up instead.
    FVector ForwardDir = GetActorForwardVector();
    // FVector ArbitraryUp = (FMath::Abs(ForwardDir.Z) > 0.99f) ? FVector::ForwardVector : FVector::UpVector;
    //  Create orthogonal basis vectors for the pyramid
    FVector RightDir = GetActorRightVector();
    FVector UpDir = GetActorUpVector();
    // Recalculate up to ensure orthogonality
    UpDir = FVector::CrossProduct(RightDir, ForwardDir).GetSafeNormal();

    // Convert FOV from degrees to radians for calculations
    float HalfVerticalRad = FMath::DegreesToRadians(VerticalFOV * 0.5f);
    float HalfHorizontalRad = FMath::DegreesToRadians(HorizontalFOV * 0.5f);

    // Generate endpoints for each distance step
    for (int32 DistStep = 0; DistStep < DistanceSteps; ++DistStep)
    {
        // Calculate distance for this step (evenly distributed)
        float CurrentDistance = MaxDistance * (float(DistStep + 1) / float(DistanceSteps));

        // Generate grid of points at this distance
        for (int32 VertIndex = 0; VertIndex < VerticalSamples; ++VertIndex)
        {
            for (int32 HorzIndex = 0; HorzIndex < HorizontalSamples; ++HorzIndex)
            {
                // Calculate normalized coordinates (-1 to +1) for this grid position
                float NormalizedHorizontal = -1.0f + (2.0f * float(HorzIndex) / float(HorizontalSamples - 1));
                float NormalizedVertical = -1.0f + (2.0f * float(VertIndex) / float(VerticalSamples - 1));

                // Convert to angular offsets from center
                float HorizontalAngle = NormalizedHorizontal * HalfHorizontalRad;
                float VerticalAngle = NormalizedVertical * HalfVerticalRad;

                // Calculate direction vector for this sample point
                FVector SampleDirection = CalculateDirectionFromAngles(HorizontalAngle, VerticalAngle);

                // Calculate final world position at the desired distance
                FVector EndPoint = ObserverLoc + (SampleDirection * CurrentDistance);

                // Add this endpoint to our array
                TraceEndpoints.Add(EndPoint);
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
    if (!IsValid(GetWorld()) || TraceIndex >= TraceEndpoints.Num() || TraceIndex >= AnalysisResults.Num())
    {
        return;
    }

    // Get observer and target locations
    FVector ObserverLoc = GetObserverLocation();
    FVector TargetLoc = TraceEndpoints[TraceIndex];

    // Set up collision query parameters
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this); // Ignore self to avoid self-collision
    QueryParams.bTraceComplex = false; // Use simple collision for performance

    // Perform the line trace
    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,      // Output hit result
        ObserverLoc,    // Start location
        TargetLoc,      // End location
        ECC_Visibility, // Collision channel (visibility)
        QueryParams     // Query parameters
    );

    // Update analysis result based on hit result
    if (bHit)
    {
        // Something blocked the view - point is not visible
        AnalysisResults[TraceIndex].bIsVisible = false;
        // Record where we hit
        AnalysisResults[TraceIndex].HitLocation = HitResult.Location;
        // Record surface normal at hit
        AnalysisResults[TraceIndex].HitNormal = HitResult.Normal;
        // Record what we hit
        AnalysisResults[TraceIndex].HitActor = HitResult.GetActor();
    }
    else
    {
        // No obstruction - point is visible
        AnalysisResults[TraceIndex].bIsVisible = true;
        // Hit location is the target endpoint
        AnalysisResults[TraceIndex].HitLocation = TargetLoc;
        // No surface normal when no obstruction
        AnalysisResults[TraceIndex].HitNormal = FVector::ZeroVector;
        // No actor was hit
        AnalysisResults[TraceIndex].HitActor = nullptr;
    }

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
    if (Debug_ProceduralMeshComponent)
    {
        Debug_ProceduralMeshComponent->ClearAllMeshSections();
    }

    // Build geometry in component-local space relative to the viewshed origin
    const FVector ObserverLoc = GetObserverLocation();

    int32 H = HorizontalSamples;
    int32 V = VerticalSamples;
    int32 NumSteps = DistanceSteps;
    int32 NumPointsPerStep = H * V;

    if (AnalysisResults.Num() != NumSteps * NumPointsPerStep)
        return;

    int SectionIdx = 0;
    for (int32 step = 0; step < NumSteps; ++step)
    {
        // Buffers for visible and hidden faces for this layer
        TArray<FVector> VertVis, VertHid;
        TArray<int32> TrisVis, TrisHid;
        TArray<FVector> NormVis, NormHid;
        TArray<FLinearColor> ColVis, ColHid;
        TArray<FVector2D> UVVis, UVHid;
        TArray<FProcMeshTangent> TgtVis, TgtHid;

        int32 base = step * NumPointsPerStep;
        // Use actor forward as the surface normal basis for procedural faces
        FVector normal = GetActorForwardVector().GetSafeNormal();

        for (int32 v = 0; v < V - 1; ++v)
        {
            for (int32 h = 0; h < H - 1; ++h)
            {
                int idx00 = base + v * H + h;
                int idx01 = base + v * H + h + 1;
                int idx10 = base + (v + 1) * H + h;
                int idx11 = base + (v + 1) * H + h + 1;

                const auto &p00 = AnalysisResults[idx00];
                const auto &p01 = AnalysisResults[idx01];
                const auto &p10 = AnalysisResults[idx10];
                const auto &p11 = AnalysisResults[idx11];

                bool vis00 = p00.bIsVisible, vis01 = p01.bIsVisible, vis10 = p10.bIsVisible, vis11 = p11.bIsVisible;

                // Only process fully visible or fully hidden quads for material separation
                if ((vis00 && vis01 && vis10 && vis11) || (!vis00 && !vis01 && !vis10 && !vis11))
                {
                    bool isVisible = vis00; // all must match, so pick one
                    TArray<FVector> &VertArray = isVisible ? VertVis : VertHid;
                    TArray<int32> &TrisArray = isVisible ? TrisVis : TrisHid;
                    TArray<FVector> &NormArray = isVisible ? NormVis : NormHid;
                    TArray<FLinearColor> &ColArray = isVisible ? ColVis : ColHid;
                    TArray<FVector2D> &UVArray = isVisible ? UVVis : UVHid;
                    TArray<FProcMeshTangent> &TgtArray = isVisible ? TgtVis : TgtHid;

                    // ------- FRONT FACE -------
                    int vi = VertArray.Num();
                    VertArray.Add(p00.WorldPosition - ObserverLoc);
                    NormArray.Add(normal);
                    ColArray.Add(FLinearColor::Green);
                    UVArray.Add(FVector2D((float)h / (H - 1), (float)v / (V - 1)));
                    TgtArray.Add(FProcMeshTangent());
                    VertArray.Add(p10.WorldPosition - ObserverLoc);
                    NormArray.Add(normal);
                    ColArray.Add(FLinearColor::Green);
                    UVArray.Add(FVector2D((float)h / (H - 1), (float)(v + 1) / (V - 1)));
                    TgtArray.Add(FProcMeshTangent());
                    VertArray.Add(p01.WorldPosition - ObserverLoc);
                    NormArray.Add(normal);
                    ColArray.Add(FLinearColor::Green);
                    UVArray.Add(FVector2D((float)(h + 1) / (H - 1), (float)v / (V - 1)));
                    TgtArray.Add(FProcMeshTangent());
                    VertArray.Add(p11.WorldPosition - ObserverLoc);
                    NormArray.Add(normal);
                    ColArray.Add(FLinearColor::Green);
                    UVArray.Add(FVector2D((float)(h + 1) / (H - 1), (float)(v + 1) / (V - 1)));
                    TgtArray.Add(FProcMeshTangent());

                    // Two triangles for front face
                    TrisArray.Add(vi + 0);
                    TrisArray.Add(vi + 1);
                    TrisArray.Add(vi + 2);
                    TrisArray.Add(vi + 2);
                    TrisArray.Add(vi + 1);
                    TrisArray.Add(vi + 3);

                    // ------- BACK FACE -------
                    int vj = VertArray.Num();
                    VertArray.Add(p00.WorldPosition - ObserverLoc);
                    NormArray.Add(-normal);
                    ColArray.Add(FLinearColor::Green);
                    UVArray.Add(FVector2D((float)h / (H - 1), (float)v / (V - 1)));
                    TgtArray.Add(FProcMeshTangent());
                    VertArray.Add(p10.WorldPosition - ObserverLoc);
                    NormArray.Add(-normal);
                    ColArray.Add(FLinearColor::Green);
                    UVArray.Add(FVector2D((float)h / (H - 1), (float)(v + 1) / (V - 1)));
                    TgtArray.Add(FProcMeshTangent());
                    VertArray.Add(p01.WorldPosition - ObserverLoc);
                    NormArray.Add(-normal);
                    ColArray.Add(FLinearColor::Green);
                    UVArray.Add(FVector2D((float)(h + 1) / (H - 1), (float)v / (V - 1)));
                    TgtArray.Add(FProcMeshTangent());
                    VertArray.Add(p11.WorldPosition - ObserverLoc);
                    NormArray.Add(-normal);
                    ColArray.Add(FLinearColor::Green);
                    UVArray.Add(FVector2D((float)(h + 1) / (H - 1), (float)(v + 1) / (V - 1)));
                    TgtArray.Add(FProcMeshTangent());

                    // Two triangles for back face (reverse winding order)
                    TrisArray.Add(vj + 2);
                    TrisArray.Add(vj + 1);
                    TrisArray.Add(vj + 0);
                    TrisArray.Add(vj + 2);
                    TrisArray.Add(vj + 3);
                    TrisArray.Add(vj + 1);
                }
                // (Optional: add mixed quad handling here if required)
            }
        }

        // Create mesh sections for visible and hidden using debug procedural component
        if (Debug_ProceduralMeshComponent)
        {
            Debug_ProceduralMeshComponent->CreateMeshSection_LinearColor(
                SectionIdx, VertVis, TrisVis, NormVis, UVVis, {}, {}, {}, ColVis, TgtVis, false, false); // Use single-sided flag, as manual double sided

            if (VisibleMaterial)
                Debug_ProceduralMeshComponent->SetMaterial(SectionIdx, VisibleMaterial);

            Debug_ProceduralMeshComponent->CreateMeshSection_LinearColor(
                SectionIdx + 1, VertHid, TrisHid, NormHid, UVHid, {}, {}, {}, ColHid, TgtHid, false, false); // Use single-sided flag

            if (HiddenMaterial)
                Debug_ProceduralMeshComponent->SetMaterial(SectionIdx + 1, HiddenMaterial);
        }

        SectionIdx += 2; // two sections per step
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
    const FRotator IdentityRot = FRotator::ZeroRotator;
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

    // Clear existing instances from both components (defensive checks)
    if (Debug_VisiblePointsISMC)
        Debug_VisiblePointsISMC->ClearInstances();
    if (Debug_HiddenPointsISMC)
        Debug_HiddenPointsISMC->ClearInstances();
    if (Debug_ProceduralMeshComponent)
        Debug_ProceduralMeshComponent->ClearAllMeshSections();

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
