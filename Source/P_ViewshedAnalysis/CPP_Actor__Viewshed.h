/*
 * @Author: Punal Manalan
 * @Description: ViewShed Analysis Plugin.
 * @Date: 04/10/2025
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "ProceduralMeshComponent.h"
#include "Components/DecalComponent.h"
#include "CPP_Actor__ViewShed.generated.h"

/**
 * Structure representing a single point in the viewshed analysis
 * Contains all information about visibility, position, and hit results
 */
USTRUCT(BlueprintType)
struct P_VIEWSHEDANALYSIS_API FS__ViewShedPoint
{
    GENERATED_BODY()

    /** World position of this analysis point */
    UPROPERTY(BlueprintReadOnly, Category = "ViewShed Point")
    FVector WorldPosition;

    /** Whether this point is visible from the viewshed origin */
    UPROPERTY(BlueprintReadOnly, Category = "ViewShed Point")
    bool bIsVisible;

    /** Distance from viewshed origin to this point */
    UPROPERTY(BlueprintReadOnly, Category = "ViewShed Point")
    float Distance;

    /** World position where the line trace hit something (if any) */
    UPROPERTY(BlueprintReadOnly, Category = "ViewShed Point")
    FVector HitLocation;

    /** World position where the line trace hit something (if any) */
    UPROPERTY(BlueprintReadOnly, Category = "ViewShed Point")
    FVector HitNormal;

    /** Actor that was hit by the line trace (if any) */
    UPROPERTY(BlueprintReadOnly, Category = "ViewShed Point")
    AActor *HitActor;

    /** Default constructor - initializes all values to safe defaults */
    FS__ViewShedPoint()
    {
        WorldPosition = FVector::ZeroVector; // Origin point
        bIsVisible = false;                  // Assume not visible until proven otherwise
        Distance = 0.0f;                     // Zero distance
        HitLocation = FVector::ZeroVector;   // No hit location
        HitNormal = FVector::ZeroVector;     // No hit normal
        HitActor = nullptr;                  // No actor hit
    }
};

/**
 * Structure Containing Trace Start and End Point
 */
USTRUCT(BlueprintType)
struct P_VIEWSHEDANALYSIS_API FS__ViewShedTracePoint
{
    GENERATED_BODY()

    /** World position Trace Start point, Usualy ObserverLocation with Forward Offset Towards the End Point(Mainly for performance Reasons)*/
    UPROPERTY(BlueprintReadOnly, Category = "ViewShed Trace Point")
    FVector TraceStart;

    /** World position Trace End point, Usualy any point Within the ViewShed */
    UPROPERTY(BlueprintReadOnly, Category = "ViewShed Trace Point")
    FVector TraceEnd;

    /** Index of the radial distance band this sample belongs to (0-based from nearest to farthest) */
    UPROPERTY(BlueprintReadOnly, Category = "ViewShed Trace Point")
    int32 DistanceBandIndex;

    /** Index of the horizontal angular sample within the distance band (0-based left to right across FOV) */
    UPROPERTY(BlueprintReadOnly, Category = "ViewShed Trace Point")
    int32 HorizontalSampleIndex;

    /** Index of the vertical angular sample within the distance band (0-based bottom to top across FOV) */
    UPROPERTY(BlueprintReadOnly, Category = "ViewShed Trace Point")
    int32 VerticalSampleIndex;

    /** Whether a supporting ground probe located a surface for this sample */
    UPROPERTY(BlueprintReadOnly, Category = "ViewShed Trace Point")
    bool bHasGroundSupport;

    /** Up-facing surface normal returned by the ground probe (ZeroVector if none) */
    UPROPERTY(BlueprintReadOnly, Category = "ViewShed Trace Point")
    FVector GroundNormal;

    /** Default constructor - initializes all values to safe defaults */
    FS__ViewShedTracePoint()
    {
        TraceStart = FVector::ZeroVector;
        TraceEnd = FVector::ZeroVector;
        DistanceBandIndex = INDEX_NONE;
        HorizontalSampleIndex = INDEX_NONE;
        VerticalSampleIndex = INDEX_NONE;
        bHasGroundSupport = false;
        GroundNormal = FVector::ZeroVector;
    }
};

/**
 * Structure Containing Trace End Points
 */
USTRUCT(BlueprintType)
struct P_VIEWSHEDANALYSIS_API FS__ViewShedTraceEndPoints
{
    GENERATED_BODY()

    /** Trace Start and End Point */
    UPROPERTY(BlueprintReadOnly, Category = "ViewShed Trace End Point")
    TArray<FS__ViewShedTracePoint> TraceEndPoints;

    /** Number of samples along the horizontal axis within this slice */
    UPROPERTY(BlueprintReadOnly, Category = "ViewShed Trace End Point")
    int32 HorizontalSampleCount;

    /** Number of samples along the vertical axis within this slice */
    UPROPERTY(BlueprintReadOnly, Category = "ViewShed Trace End Point")
    int32 VerticalSampleCount;

    /** Default constructor - initializes all values to safe defaults */
    FS__ViewShedTraceEndPoints()
    {
        TraceEndPoints.Empty();
        HorizontalSampleCount = 0;
        VerticalSampleCount = 0;
    }
};

/**
 * Structure Containing Trace End Points
 */
USTRUCT(BlueprintType)
struct P_VIEWSHEDANALYSIS_API FS__ViewShedTraceSection
{
    GENERATED_BODY()

    /** Trace Start and End Points */
    UPROPERTY(BlueprintReadOnly, Category = "ViewShed Trace End Point")
    TArray<FS__ViewShedTraceEndPoints> TraceSections;

    /** Number of horizontal sub-sections that partition this distance layer */
    UPROPERTY(BlueprintReadOnly, Category = "ViewShed Trace End Point")
    int32 HorizontalSectionCount;

    /** Number of vertical sub-sections that partition this distance layer */
    UPROPERTY(BlueprintReadOnly, Category = "ViewShed Trace End Point")
    int32 VerticalSectionCount;

    /** Default constructor - initializes all values to safe defaults */
    FS__ViewShedTraceSection()
    {
        TraceSections.Empty();
        HorizontalSectionCount = 0;
        VerticalSectionCount = 0;
    }
};

/**
 * Delegate for broadcasting when viewshed analysis is complete
 * Allows other systems to react to finished analysis
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnViewShedComplete, const TArray<FS__ViewShedPoint> &, AnalysisResults);

/**
 * Main ViewShed Actor class
 * Performs pyramid-shaped visibility analysis using line traces
 * Visualizes results using Instanced Static Mesh Components
 */
UCLASS(BlueprintType, Blueprintable, Category = "ViewShed Analysis")
class P_VIEWSHEDANALYSIS_API ACPP_Actor__Viewshed : public AActor
{
    GENERATED_BODY()

public:
    /** Constructor - sets up default values and components */
    ACPP_Actor__Viewshed();

protected:
    /** Called when the game starts or when spawned */
    virtual void BeginPlay() override;

public:
    /** Called every frame to update analysis if needed */
    virtual void Tick(float DeltaTime) override;

    //////////////////////////////////////////////////////////////////////////
    // CORE VIEWSHED PROPERTIES
    //////////////////////////////////////////////////////////////////////////

    // Punal Manalan, NOTE: View Direction is the default forward vector of the actor
    ///** Direction the viewshed is pointing (normalized automatically) */
    // UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewShed Configuration",
    //           meta = (DisplayName = "View Direction"))
    // FVector ViewDirection = FVector::ForwardVector;

    /** Maximum distance to perform analysis (in Unreal units) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewShed Configuration",
              meta = (DisplayName = "Maximum Range", ClampMin = "100.0", UIMax = "50000.0"))
    float MaxDistance = 5000.0f;

    /** Vertical field of view angle in degrees */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewShed Configuration",
              meta = (DisplayName = "Vertical FOV", ClampMin = "1.0", ClampMax = "179.0", UIMax = "120.0"))
    float VerticalFOV = 60.0f;

    /** Horizontal field of view angle in degrees */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewShed Configuration",
              meta = (DisplayName = "Horizontal FOV", ClampMin = "1.0", ClampMax = "179.0", UIMax = "120.0"))
    float HorizontalFOV = 90.0f;

    /** Height offset above actor location to start analysis from */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewShed Configuration",
              meta = (DisplayName = "Observer Height", UIMax = "500.0"))
    float ObserverHeight = 150.0f;

    //////////////////////////////////////////////////////////////////////////
    // SAMPLING RESOLUTION PROPERTIES
    //////////////////////////////////////////////////////////////////////////

    /** This Refers to the Horizontal Size of the Sample Section.
     *  For Example 0.2 of 90* Horizontal FOV would be equal to 18 degrees.
     *  Sample Section Starts form Top Left and Ends at Bottom Right.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sampling Resolution",
              meta = (DisplayName = "Maximum Horizontal Sample Section Ratio", ClampMin = "0.1", ClampMax = "1", UIMax = "1"))
    float Horizontal_Sample_Section_Ratio = 0.2f;

    /** This Refers to the Vertical Size of the Sample Section.
     * For Example 0.4 of 60* Vertical FOV would be equal to 24 degrees.
     * Sample Section Starts form Top Left and Ends at Bottom Right.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sampling Resolution",
              meta = (DisplayName = "Maximum Vertical Sample Section Ratio", ClampMin = "0.1", ClampMax = "1", UIMax = "1"))
    float Vertical_Sample_Section_Ratio = 0.4f;

    /** Number of distance steps from observer to max distance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sampling Resolution",
              meta = (DisplayName = "Distance Steps", ClampMin = "1", ClampMax = "50", UIMax = "20"))
    int32 DistanceSteps = 5;

    /** Maximum Distance Between Samples.
     *  At each subsequent step Distance between Samples increases.
     *  At the Last Distance Step Distance Between Samples highest, it would be proportional to MaxDistance.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sampling Resolution",
              meta = (DisplayName = "Maximum Distance Between Samples", ClampMin = "1", UIMax = "5000"))
    float Maximum_Distance_Between_Samples = 500;

    /** Maximum Number of Samples Per Section
     *  if a Section too Large, this will divide the section into smaller sections
     *  to keep the number of samples per section under this limit.
     *  this Keeps the Sampling Section more Accurate.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sampling Resolution",
              meta = (DisplayName = "Samples Per Section", ClampMin = "1", UIMax = "5000"))
    int32 Minimum_Samples_Per_Section = 500;

    //////////////////////////////////////////////////////////////////////////
    // VISUALIZATION PROPERTIES
    //////////////////////////////////////////////////////////////////////////

    /** Material for visible points (green recommended) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization",
              meta = (DisplayName = "Visible Material"))
    UMaterialInterface *VisibleMaterial;

    /** Material for hidden/occluded points (red recommended) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization",
              meta = (DisplayName = "Hidden Material"))
    UMaterialInterface *HiddenMaterial;

    /** Height offset applied to the procedural blanket that visualises visible areas */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization",
              meta = (DisplayName = "Visible Blanket Offset", ClampMin = "0.0", UIMax = "50.0"))
    float VisibleVisualization_SurfaceOffset = 5.0f;

    /** Half-size (in cm) of each quad stamped onto visible hit locations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization",
              meta = (DisplayName = "Visible Blanket Quad Half Size", ClampMin = "1.0", UIMax = "500.0"))
    float VisibleVisualization_QuadHalfSize = 25.0f;

    /** Decal material used by HiddenVisualizationDecalComponent (Deferred Decal domain). Should implement frustum tests. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization",
              meta = (DisplayName = "Hidden Visualization Decal Material"))
    UMaterialInterface *HiddenVisualizationDecalMaterial;

    //////////////////////////////////////////////////////////////////////////
    // ANALYSIS CONTROL PROPERTIES
    //////////////////////////////////////////////////////////////////////////

    /** Whether to automatically update analysis over time */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analysis Control",
              meta = (DisplayName = "Auto Update"))
    bool bAutoUpdate = true;

    /** How often to update analysis when auto-update is enabled (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Analysis Control",
              meta = (DisplayName = "Update Interval", ClampMin = "0.1", UIMax = "10.0"))
    float UpdateInterval = 2.0f;

    /** Maximum number of traces to process per frame (for performance) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance",
              meta = (DisplayName = "Max Traces Per Frame", ClampMin = "10", UIMax = "500"))
    int32 MaxTracesPerFrame = 50;

    //////////////////////////////////////////////////////////////////////////
    // HIDDEN VISUALIZATION DECAL MATERIAL PARAMETERS
    //////////////////////////////////////////////////////////////////////////
    /** Surface facing threshold; higher means stricter facing requirement (dot(N, viewDir) >= threshold). Range [-1..1] */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Decal|Material",
              meta = (DisplayName = "Normal Threshold", ClampMin = "-1.0", ClampMax = "1.0"))
    float VS_NormalThreshold = 0.0f;

    /** Feather width for frustum edges and distance falloff */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Decal|Material",
              meta = (DisplayName = "Frustum Feather", ClampMin = "0.0", UIMax = "1.0"))
    float VS_FrustumFeather = 0.05f;

    /** Feather width for facing threshold */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Decal|Material",
              meta = (DisplayName = "Facing Feather", ClampMin = "0.0", UIMax = "1.0"))
    float VS_FacingFeather = 0.1f;

    /** Toggle for facing check (1=enabled, 0=disabled); values in between blend */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Decal|Material",
              meta = (DisplayName = "Facing Enabled", ClampMin = "0.0", ClampMax = "1.0"))
    float VS_FacingEnabled = 0.0f;

    /** Color used inside the viewshed mask */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Decal|Material",
              meta = (DisplayName = "Color Inside"))
    FLinearColor VS_ColorInside = FLinearColor(1.f, 0.f, 0.f, 1.f);

    /** Color used outside the viewshed mask */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Decal|Material",
              meta = (DisplayName = "Color Outside"))
    FLinearColor VS_ColorOutside = FLinearColor(1.f, 1.f, 1.f, 1.f);

    /** Intensity of optional debug grid overlay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Decal|Material",
              meta = (DisplayName = "Grid Intensity", ClampMin = "0.0", UIMax = "1.0"))
    float VS_GridIntensity = 0.2f;

    /** Optional opacity multiplier for the decal material (if implemented in material) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hidden Decal|Material",
              meta = (DisplayName = "Opacity Multiplier", ClampMin = "0.0", ClampMax = "1.0"))
    float VS_Opacity = 0.6f;

    //////////////////////////////////////////////////////////////////////////
    // DEBUG PROPERTIES
    //////////////////////////////////////////////////////////////////////////

    // Toggle property: Show Debug Visualization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
    bool bDebug_ShowDebugVisualization = false;

    // Toggle property: use merged mesh or ISMC
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization")
    bool bDebug_UseProceduralMesh = false;

    /** Static mesh to use for visible points (sphere recommended) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization",
              meta = (DisplayName = "Visible Point Mesh"))
    UStaticMesh *Debug_VisiblePointMesh;

    /** Scale multiplier for visualization points */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization",
              meta = (DisplayName = "Point Scale", ClampMin = "0.1", UIMax = "5.0"))
    float Debug_PointScale = 1.0f;

    /** Whether to show visible points */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization",
              meta = (DisplayName = "Show Visible Points"))
    bool bDebug_ShowVisiblePoints = true;

    /** Whether to show hidden/occluded points */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization",
              meta = (DisplayName = "Show Hidden Points"))
    bool bDebug_ShowHiddenPoints = false;

    /** Whether to show debug lines for line traces */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization",
              meta = (DisplayName = "Show Debug Lines"))
    bool bDebug_ShowLines = false;

    /** How long debug lines should persist (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization",
              meta = (DisplayName = "Debug Line Duration", ClampMin = "0.1", UIMax = "30.0"))
    float bDebug_LineDuration = 5.0f;

    /** Whether to show the viewshed pyramid bounds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization",
              meta = (DisplayName = "Show Pyramid Bounds"))
    bool bDebug_ShowPyramidBounds = true;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    /** Event fired when viewshed analysis completes */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnViewShedComplete OnAnalysisComplete;

    //////////////////////////////////////////////////////////////////////////
    // PUBLIC FUNCTIONS
    //////////////////////////////////////////////////////////////////////////

    /** Manually start a new viewshed analysis */
    UFUNCTION(BlueprintCallable, Category = "ViewShed Analysis")
    void StartAnalysis();

    /** Stop current analysis if running */
    UFUNCTION(BlueprintCallable, Category = "ViewShed Analysis")
    void StopAnalysis();

    /** Clear all current analysis results and visualization */
    UFUNCTION(BlueprintCallable, Category = "ViewShed Analysis")
    void ClearResults();

    /** Get the current analysis results */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ViewShed Analysis")
    TArray<FS__ViewShedPoint> GetAnalysisResults() const { return AnalysisResults; }

    /** Get number of visible points in current analysis */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ViewShed Analysis")
    int32 GetVisiblePointCount() const;

    /** Get number of hidden points in current analysis */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ViewShed Analysis")
    int32 GetHiddenPointCount() const;

    /** Get visibility percentage (0-100) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ViewShed Analysis")
    float GetVisibilityPercentage() const;

protected:
    //////////////////////////////////////////////////////////////////////////
    // COMPONENTS
    //////////////////////////////////////////////////////////////////////////

    // Decal component to Paint everything in size box Red (Except Any ISMC and Procedural Mesh which is part of any ACPP_Actor__Viewshed Actor class)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UDecalComponent *HiddenVisualizationDecalComponent;

    // Procedural mesh component to Visualize Visible mesh
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UProceduralMeshComponent *VisibleVisualization_ProceduralMeshComponent;

    /** Component for rendering visible point instances */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInstancedStaticMeshComponent *Debug_VisiblePointsISMC;

    /** Component for rendering hidden point instances */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInstancedStaticMeshComponent *Debug_HiddenPointsISMC;

    // Procedural mesh component to show Viewshed Step merged mesh
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UProceduralMeshComponent *Debug_ProceduralMeshComponent;

private:
    //////////////////////////////////////////////////////////////////////////
    // INTERNAL DATA
    //////////////////////////////////////////////////////////////////////////

    /** Array storing all analysis point results */
    TArray<FS__ViewShedPoint> AnalysisResults;

    /** Hierarchical layout of traces organised by distance steps and FOV sub-sections */
    TArray<FS__ViewShedTraceSection> TraceSections;

    /** Flattened queue of trace start/end pairs, consumed sequentially during analysis */
    TArray<FS__ViewShedTracePoint> TracePointQueue;

    /** Current state of analysis processing */
    bool bAnalysisInProgress = false;

    /** Index of current trace being processed */
    int32 CurrentTraceIndex = 0;

    /** Time when last analysis update occurred */
    float LastUpdateTime = 0.0f;

    /** Dynamic material instance used by the hidden visualization decal to receive runtime parameters */
    UMaterialInstanceDynamic *HiddenVisualizationDecalMID = nullptr;

    /** Cached number of horizontal samples produced during the last GenerateTraceEndpoints pass */
    int32 CachedHorizontalSampleCount = 0;

    /** Cached number of distance bands (excluding the implicit origin ring) produced during the last GenerateTraceEndpoints pass */
    int32 CachedDistanceBandCount = 0;

    /** Cached number of vertical samples produced during the last GenerateTraceEndpoints pass */
    int32 CachedVerticalSampleCount = 0;

    //////////////////////////////////////////////////////////////////////////
    // INTERNAL FUNCTIONS
    //////////////////////////////////////////////////////////////////////////

    /** Generate all trace endpoints in pyramid pattern */
    void GenerateTraceEndpoints();

    /** Process a single line trace by index */
    void ProcessSingleTrace(int32 TraceIndex);

    /** Build Debug Point Mesh */
    void BuildDebug_PointMesh();

    /** Build Debug Procedural Merged Mesh */
    void BuildDebug_ProceduralMergedMesh();

    /** Build Visible Visualization Procedural Merged Mesh */
    void BuildVisibleVisualization_ProceduralMergedMesh();

    /** Update visualization based on current results */
    void UpdateVisualization();

    /** Update or initialize the hidden visualization decal component transform and material parameters */
    void UpdateHiddenVisualizationDecal();

    /** Get the world position of the observer (actor + height offset) */
    FVector GetObserverLocation() const;

    /** Draw debug visualization for the viewshed pyramid */
    void DrawDebugPyramid() const;

    /** Calculate a world direction vector from horizontal/vertical angles */
    FVector CalculateDirectionFromAngles(float HorizontalAngle, float VerticalAngle) const;

    /** Check if analysis should be updated based on time */
    bool ShouldUpdateAnalysis() const;
};
