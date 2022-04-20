// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Volume.h"
#include "MapSourceVolume.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorEnter, class AMapSourceVolume*, Volume, AActor*, EnteredActor, UPrimitiveComponent*, EnteredComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorExit, class AMapSourceVolume*, Volume, AActor*, ExitedActor, UPrimitiveComponent*, ExitedComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FContainedComponentsUpdated, class AMapSourceVolume*, Volume);

/**
 * Class for setting up a mapped environment. A Mapped source volume contains the components for sending to the map UI for having area specific maps.
 */
UCLASS(Blueprintable)
class MAPPING_API AMapSourceVolume : public AVolume
{
	GENERATED_BODY()
public:
	AMapSourceVolume();

	FORCEINLINE class UStaticMeshComponent* GetMeshComp() const { return MeshComp; }

	FORCEINLINE class UDrawFrustumComponent* GetDrawFrustum() const { return DrawFrustum; }

	FORCEINLINE class USceneCaptureComponentMap* GetMapCaptureComponent() const { return MapCaptureComponent; }

	FORCEINLINE TArray<class USceneMapComponent*> GetContainedMapComponents() const { return ContainedMapComponents; }

	/* Multicast Delegate that is fired when an Actor enters the volume*/
	UPROPERTY(BlueprintAssignable, Category = "MapSourceVolume")
	FOnActorEnter OnActorEntered;

	/* Multicast Delegate that is fired whan an Actor leaves the volume*/
	UPROPERTY(BlueprintAssignable, Category = "MapSourceVolume")
	FOnActorExit OnActorExited;

	/* Multicast Delegate that is fired when a SceneMapComponent is added or removed from the list*/
	UPROPERTY(BlueprintAssignable, Category = "MapSourceVolume")
	FContainedComponentsUpdated ContainedComponentsUpdated;
	
	/* Add Actors to the ShouldCapture array of the SceneCaptureComponentMap*/
	UFUNCTION(BlueprintCallable, Category = "MapSourceVolume")
	void AppendShouldCaptureActors(const TArray<AActor*>& ShouldCapture);

	/* Add Actors to the ShouldIgnore array of the SceneCaptureComponentMap*/
	UFUNCTION(BlueprintCallable, Category = "MapSourceVolume")
	void AppendShouldIgnoreActors(const TArray<AActor*>& ShouldIgnore);

	/* Query as to whether the volume should track a given actor with the camera*/
	UFUNCTION(BlueprintCallable, Category = "MapSourceVolume")
	bool ShouldTrackEnteredActor(AActor* EnteredActor);

	/* Define the camera tracked Actor*/
	UFUNCTION(BlueprintCallable, Category = "MapSourceVolume")
	void SetTrackedActor(AActor* Actor);

	/*Beg Actor Interface*/
	virtual void BeginPlay() override;
	virtual void PostActorCreated() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	/*End Actor Interface*/

	/** Used to synchronize the DrawFrustumComponent with the SceneCaptureComponentMap settings. */
	void UpdateDrawFrustum();

	/* Toggle Visibility of the SceneCaptureComponentMap Component */
	UFUNCTION(BlueprintCallable, Category = "Rendering")
	void OnInterpToggle(bool bEnable);

protected:
	/* Handler for when an actor enters the volume and the component that caused the overlap*/
	UFUNCTION(BlueprintNativeEvent, Category = "MapSourceVolume")
	void OnActorEnteredMapVolume(AActor* EnteredActor, UPrimitiveComponent* EnteredComponent);

	/* Handler for when an actor leaves the volume and the component that caused the overlap */
	UFUNCTION(BlueprintNativeEvent, Category = "MapSourceVolume")
	void OnActorExitedMapVolume(AActor* ExitedActor, UPrimitiveComponent* ExitedComponent);

	/* Implementation of ShouldTrackEnteredActor*/
	UFUNCTION(BlueprintNativeEvent, Category = "MapSourceVolume")
	bool ShouldMapCaptureTrackEnteredActor(AActor* EnteredActor);

	/* Broadcast the OnActorEntered Event*/
	UFUNCTION(BlueprintCallable, Category = "MapSourceVolume")
	void NotifyActorEntered(AActor* EnteredActor, UPrimitiveComponent* EnteredComponent);

	/* Broadcast the OnActorExited Event*/
	UFUNCTION(BlueprintCallable, Category = "MapSourceVolume")
	void NotifyActorExited(AActor* ExitedActor, UPrimitiveComponent* ExitedComponent);

	/* Broadcast the ContainedComponentsUpdated Event*/
	UFUNCTION(BlueprintCallable, Category = "MapSourceVolume")
	void NotifyContainedComponentsUpdated();

	/* Only render static actors within the volume on Begin Play*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MapSourceVolume")
	bool bAutoIgnoreNonStaticActors;

	/* If an Actor has a SceneMapComponent, should it be ignored for SceneCapture and instead use the Icon?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MapSourceVolume")
	bool bAutoIgnoreActorsWithSceneMapComponents;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
	class USceneCaptureComponentMap* MapCaptureComponent;

	UPROPERTY(EditAnywhere, Meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(Meta = (AllowPrivateAccess = "true"))
	class UDrawFrustumComponent* DrawFrustum;

	UPROPERTY(Replicated)
	class AActor* TrackedActor;
	
	UPROPERTY(Replicated, BlueprintReadOnly, VisibleAnywhere, Category = "MapSourceVolume", Meta = (AllowPrivateAccess = "true"))
	TArray<class USceneMapComponent*> ContainedMapComponents;

	UFUNCTION()
	void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
};
