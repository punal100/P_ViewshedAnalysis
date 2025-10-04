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
        HitActor = nullptr;                  // No actor hit
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

    /** Direction the viewshed is pointing (normalized automatically) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewShed Configuration",
              meta = (DisplayName = "View Direction"))
    FVector ViewDirection = FVector::ForwardVector;

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

    /** Number of horizontal samples across the FOV */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sampling Resolution",
              meta = (DisplayName = "Horizontal Samples", ClampMin = "3", ClampMax = "200", UIMax = "100"))
    int32 HorizontalSamples = 20;

    /** Number of vertical samples across the FOV */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sampling Resolution",
              meta = (DisplayName = "Vertical Samples", ClampMin = "3", ClampMax = "200", UIMax = "100"))
    int32 VerticalSamples = 15;

    /** Number of distance steps from observer to max distance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sampling Resolution",
              meta = (DisplayName = "Distance Steps", ClampMin = "1", ClampMax = "50", UIMax = "20"))
    int32 DistanceSteps = 5;

    //////////////////////////////////////////////////////////////////////////
    // VISUALIZATION PROPERTIES
    //////////////////////////////////////////////////////////////////////////

    // Toggle property: use merged mesh or ISMC
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
    bool bUseProceduralMesh = false;

    /** Static mesh to use for visible points (sphere recommended) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization",
              meta = (DisplayName = "Visible Point Mesh"))
    UStaticMesh *VisiblePointMesh;

    /** Material for visible points (green recommended) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization",
              meta = (DisplayName = "Visible Material"))
    UMaterialInterface *VisibleMaterial;

    /** Material for hidden/occluded points (red recommended) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization",
              meta = (DisplayName = "Hidden Material"))
    UMaterialInterface *HiddenMaterial;

    /** Scale multiplier for visualization points */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization",
              meta = (DisplayName = "Point Scale", ClampMin = "0.1", UIMax = "5.0"))
    float PointScale = 1.0f;

    /** Whether to show visible points */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization",
              meta = (DisplayName = "Show Visible Points"))
    bool bShowVisiblePoints = true;

    /** Whether to show hidden/occluded points */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization",
              meta = (DisplayName = "Show Hidden Points"))
    bool bShowHiddenPoints = false;

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
    // DEBUG PROPERTIES
    //////////////////////////////////////////////////////////////////////////

    /** Whether to show debug lines for line traces */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization",
              meta = (DisplayName = "Show Debug Lines"))
    bool bShowDebugLines = false;

    /** How long debug lines should persist (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization",
              meta = (DisplayName = "Debug Line Duration", ClampMin = "0.1", UIMax = "30.0"))
    float DebugLineDuration = 5.0f;

    /** Whether to show the viewshed pyramid bounds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug Visualization",
              meta = (DisplayName = "Show Pyramid Bounds"))
    bool bShowPyramidBounds = true;

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

    /** Component for rendering visible point instances */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInstancedStaticMeshComponent *VisiblePointsISMC;

    /** Component for rendering hidden point instances */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInstancedStaticMeshComponent *HiddenPointsISMC;

    // Procedural mesh component to show merged mesh
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UProceduralMeshComponent *ProceduralMeshComponent;

private:
    //////////////////////////////////////////////////////////////////////////
    // INTERNAL DATA
    //////////////////////////////////////////////////////////////////////////

    /** Array storing all analysis point results */
    TArray<FS__ViewShedPoint> AnalysisResults;

    /** Array of world positions to trace to */
    TArray<FVector> TraceEndpoints;

    /** Current state of analysis processing */
    bool bAnalysisInProgress = false;

    /** Index of current trace being processed */
    int32 CurrentTraceIndex = 0;

    /** Time when last analysis update occurred */
    float LastUpdateTime = 0.0f;

    //////////////////////////////////////////////////////////////////////////
    // INTERNAL FUNCTIONS
    //////////////////////////////////////////////////////////////////////////

    /** Generate all trace endpoints in pyramid pattern */
    void GenerateTraceEndpoints();

    /** Process a single line trace by index */
    void ProcessSingleTrace(int32 TraceIndex);

    /** Build Procedural Merged Mesh */
    void BuildProceduralMergedMesh();

    /** Update visualization based on current results */
    void UpdateVisualization();

    /** Get the world position of the observer (actor + height offset) */
    FVector GetObserverLocation() const;

    /** Draw debug visualization for the viewshed pyramid */
    void DrawDebugPyramid() const;

    /** Calculate a world direction vector from horizontal/vertical angles */
    FVector CalculateDirectionFromAngles(float HorizontalAngle, float VerticalAngle) const;

    /** Check if analysis should be updated based on time */
    bool ShouldUpdateAnalysis() const;
};
