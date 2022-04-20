// Fill out your copyright notice in the Description page of Project Settings.

#include "MappingPrivatePCH.h"
#include "MapSourceVolume.h"
#include "SceneMapComponent.h"
#include "SceneCaptureComponentMap.h"
#include "UnrealNetwork.h"

AMapSourceVolume::AMapSourceVolume()
	: bAutoIgnoreActorsWithSceneMapComponents(true)
	, bAutoIgnoreNonStaticActors(true)
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EditorCameraMesh"));
	MeshComp->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	MeshComp->bHiddenInGame = true;
	MeshComp->CastShadow = false;
	MeshComp->PostPhysicsComponentTick.bCanEverTick = false;
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 200.0f), FRotator(-90.0f, 0.0f, 0.0f).Quaternion());
	
	DrawFrustum = CreateDefaultSubobject<UDrawFrustumComponent>(TEXT("EditorDrawFrustum"));
	DrawFrustum->bVisible = true;
	DrawFrustum->bIsEditorOnly = true;
	DrawFrustum->SetupAttachment(MeshComp);

	MapCaptureComponent = CreateDefaultSubobject<USceneCaptureComponentMap>(TEXT("MapCapture"));
	MapCaptureComponent->SetupAttachment(MeshComp);
	GetBrushComponent()->OnComponentEndOverlap.AddDynamic(this, &AMapSourceVolume::OnComponentEndOverlap);
	GetBrushComponent()->OnComponentBeginOverlap.AddDynamic(this, &AMapSourceVolume::OnComponentBeginOverlap);

	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MeshComp->SetIsReplicated(false);
	DrawFrustum->SetIsReplicated(false);

	MapCaptureComponent->SetIsReplicated(true);
}

void AMapSourceVolume::AppendShouldCaptureActors(const TArray<AActor*>& ShouldCapture)
{
	if (MapCaptureComponent)
	{
		MapCaptureComponent->ShowOnlyActors.Append(ShouldCapture);
	}
}

void AMapSourceVolume::AppendShouldIgnoreActors(const TArray<AActor*>& ShouldIgnore)
{
	if (MapCaptureComponent)
	{
		MapCaptureComponent->HiddenActors.Append(ShouldIgnore);
	}
}

bool AMapSourceVolume::ShouldTrackEnteredActor(AActor* EnteredActor)
{
	return ShouldMapCaptureTrackEnteredActor(EnteredActor);
}

void AMapSourceVolume::NotifyActorEntered(AActor* EnteredActor, UPrimitiveComponent* EnteredComponent)
{
	OnActorEntered.Broadcast(this, EnteredActor, EnteredComponent);
}

void AMapSourceVolume::NotifyActorExited(AActor* ExitedActor, UPrimitiveComponent* ExitedComponent)
{
	OnActorExited.Broadcast(this, ExitedActor, ExitedComponent);
}

void AMapSourceVolume::NotifyContainedComponentsUpdated()
{
	ContainedComponentsUpdated.Broadcast(this);
}

void AMapSourceVolume::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	OnActorExitedMapVolume(OtherActor, OtherComp);
}

void AMapSourceVolume::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	OnActorEnteredMapVolume(OtherActor, OtherComp);
}

void AMapSourceVolume::OnActorEnteredMapVolume_Implementation(AActor* EnteredActor, UPrimitiveComponent* EnteredComponent)
{
	USceneMapComponent* PossibleMapComponent = Cast<USceneMapComponent>(EnteredActor->GetComponentByClass(USceneMapComponent::StaticClass()));
	if (PossibleMapComponent)
	{
		ContainedMapComponents.AddUnique(PossibleMapComponent);
		NotifyContainedComponentsUpdated();
	}
	NotifyActorEntered(EnteredActor, EnteredComponent);
	if (ShouldTrackEnteredActor(EnteredActor))
	{
		SetTrackedActor(EnteredActor);
	}
}

void AMapSourceVolume::OnActorExitedMapVolume_Implementation(AActor* ExitedActor, UPrimitiveComponent* ExitedComponent)
{
	USceneMapComponent* PossibleMapComponent = Cast<USceneMapComponent>(ExitedActor->GetComponentByClass(USceneMapComponent::StaticClass()));
	if (PossibleMapComponent)
	{
		ContainedMapComponents.Remove(PossibleMapComponent);
		NotifyContainedComponentsUpdated();
	}
	NotifyActorExited(ExitedActor, ExitedComponent);
	if (TrackedActor == ExitedActor)
	{
		SetTrackedActor(nullptr);
	}
}

bool AMapSourceVolume::ShouldMapCaptureTrackEnteredActor_Implementation(AActor* EnteredActor)
{
	return false;
}

void AMapSourceVolume::BeginPlay()
{
	Super::BeginPlay();
	if(bAutoIgnoreActorsWithSceneMapComponents ||
		bAutoIgnoreNonStaticActors)
	{
		for (TActorIterator<AActor> Iter(GetWorld()); Iter; ++Iter)
		{
			AActor* Actor = *Iter;
			if (Actor)
			{
				if (Actor == this) continue;

				if (bAutoIgnoreActorsWithSceneMapComponents)
				{
					if (Actor->GetComponentByClass(USceneMapComponent::StaticClass()))
					{
						MapCaptureComponent->HiddenActors.Add(Actor);
					}
				}

				if (bAutoIgnoreNonStaticActors)
				{
					USceneComponent* Root = Actor->GetRootComponent();
					if (Root)
					{
						if (Root->Mobility != EComponentMobility::Static)
						{
							MapCaptureComponent->HiddenActors.Add(Actor);
						}
					}
				}
			}
		}
	}
}

/* Copy past of SceneCaptureComponent.cpp definitions for ASceneCapture2D*/
void AMapSourceVolume::OnInterpToggle(bool bEnable)
{
	MapCaptureComponent->SetVisibility(bEnable);
}

void AMapSourceVolume::UpdateDrawFrustum()
{
	if (DrawFrustum && MapCaptureComponent)
	{
		DrawFrustum->FrustumStartDist = GNearClippingPlane;

		// 1000 is the default frustum distance, ideally this would be infinite but that might cause rendering issues
		DrawFrustum->FrustumEndDist = (MapCaptureComponent->MaxViewDistanceOverride > DrawFrustum->FrustumStartDist)
			? MapCaptureComponent->MaxViewDistanceOverride : 1000.0f;

		DrawFrustum->FrustumAngle = MapCaptureComponent->FOVAngle;
		//DrawFrustum->FrustumAspectRatio = CaptureComponent2D->AspectRatio;
	}
}

void AMapSourceVolume::PostActorCreated()
{
	Super::PostActorCreated();

	// no need load the editor mesh when there is no editor
#if WITH_EDITOR
	if (GetMeshComp())
	{
		if (!IsRunningCommandlet())
		{
			if (!GetMeshComp()->GetStaticMesh())
			{
				UStaticMesh* CamMesh = LoadObject<UStaticMesh>(NULL, TEXT("/Engine/EditorMeshes/MatineeCam_SM.MatineeCam_SM"), NULL, LOAD_None, NULL);
				GetMeshComp()->SetStaticMesh(CamMesh);
			}
		}
	}
#endif

	// Sync component with CameraActor frustum settings.
	UpdateDrawFrustum();
}

void AMapSourceVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (TrackedActor && MapCaptureComponent)
	{
		MapCaptureComponent->GoToWorldPosition(TrackedActor->GetActorLocation());
	}
}

void AMapSourceVolume::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMapSourceVolume, ContainedMapComponents);
	DOREPLIFETIME(AMapSourceVolume, TrackedActor);
}

void AMapSourceVolume::SetTrackedActor(AActor* Actor)
{
	TrackedActor = Actor;
	if (!TrackedActor)
	{
		MapCaptureComponent->GoToWorldPosition(MeshComp->GetComponentLocation());
	}
}