// Fill out your copyright notice in the Description page of Project Settings.

#include "MappingPrivatePCH.h"
#include "GameFramework/Actor.h"
#include "SceneCaptureComponentMap.h"

USceneCaptureComponentMap::USceneCaptureComponentMap(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCaptureEveryFrame = true;
	bCaptureOnMovement = true;
	bWantsInitializeComponent = true;
	ProjectionType = ECameraProjectionMode::Orthographic;
	CaptureSource = ESceneCaptureSource::SCS_SceneColorSceneDepth;
	MaterialParameterName = TEXT("MapTexture");
	FOVAngle = 120.0f;
}

FVector2D USceneCaptureComponentMap::GetViewToTextureScale() const
{
	if (TextureTarget)
	{
		const float OrthoHeight = OrthoWidth / (FOVAngle * (float)PI / 360.0f);
		return FVector2D(TextureTarget->SizeX, TextureTarget->SizeY) / FVector2D(OrthoWidth, OrthoHeight);
	}
	else
	{
		return FVector2D(1.0f, 1.0f);
	}
	 
}

FVector USceneCaptureComponentMap::ProjectActorLocationToTextureLocation(AActor* Actor) const
{
	if (Actor)
	{
		return ProjectLocationToTextureLocation(Actor->GetActorLocation());
	}
	else
	{
		return FVector::ZeroVector;
	}
}

FVector USceneCaptureComponentMap::ProjectSceneComponentLocationToTextureLocation(USceneComponent* Component) const
{
	if (Component)
	{
		return ProjectLocationToTextureLocation(Component->GetComponentLocation());
	}
	else
	{
		return FVector::ZeroVector;
	}
}

FVector USceneCaptureComponentMap::ProjectLocationToTextureLocation(const FVector& WorldLocation) const
{
	UTextureRenderTarget2D* Target = TextureTarget;
	if (Target)
	{
		const float OrthoHeight = OrthoWidth / (FOVAngle * (float)PI / 360.0f);
		const FIntPoint CaptureSize(OrthoWidth, OrthoHeight);
		const FIntRect ViewRect(0, 0, OrthoWidth, OrthoHeight);

		FVector2D Result;
		if (FSceneView::ProjectWorldToScreen(WorldLocation,
			ViewRect,
			GetViewProjectionMatrix(),
			Result))
		{
			const FVector2D ScaleVector = GetViewToTextureScale();
			return FVector(Result.X, Result.Y, WorldLocation.Z) * FVector(ScaleVector.X, ScaleVector.Y, 1.0f);
		}
	}
	return FVector::ZeroVector;
}

FMatrix USceneCaptureComponentMap::GetViewProjectionMatrix() const
{
	FTransform Transform = GetComponentToWorld();
	FMatrix ViewMatrix = Transform.ToInverseMatrixWithScale();

	// swap axis st. x=z,y=x,z=y (unreal coord space) so that z is up
	ViewMatrix = ViewMatrix * FMatrix(
		FPlane(0, 0, 1, 0),
		FPlane(1, 0, 0, 0),
		FPlane(0, 1, 0, 0),
		FPlane(0, 0, 0, 1));
	const float FOV = FOVAngle * (float)PI / 360.0f;

	const float OrthoHeight = OrthoWidth / FOV;
	const FIntPoint CaptureSize(OrthoWidth, OrthoHeight);
	const FIntRect ViewRect(0, 0, OrthoWidth, OrthoHeight);

	float XAxisMultiplier;
	float YAxisMultiplier;

	if (CaptureSize.X > CaptureSize.Y)
	{
		// if the viewport is wider than it is tall
		XAxisMultiplier = 1.0f;
		YAxisMultiplier = CaptureSize.X / (float)CaptureSize.Y;
	}
	else
	{
		// if the viewport is taller than it is wide
		XAxisMultiplier = CaptureSize.Y / (float)CaptureSize.X;
		YAxisMultiplier = 1.0f;
	}

	FMatrix ProjectionMatrix;
	if (ProjectionType == ECameraProjectionMode::Orthographic)
	{
		check((int32)ERHIZBuffer::IsInverted);
		const float ProjOrthoWidth = OrthoWidth / 2.0f;
		const float ProjOrthoHeight = OrthoWidth / 2.0f * YAxisMultiplier;

		const float NearPlane = 0;
		const float FarPlane = WORLD_MAX / 8.0f;

		const float ZScale = 1.0f / (FarPlane - NearPlane);
		const float ZOffset = -NearPlane;

		ProjectionMatrix = FReversedZOrthoMatrix(
			ProjOrthoWidth,
			ProjOrthoHeight,
			ZScale,
			ZOffset
		);
	}
	else
	{
		if ((int32)ERHIZBuffer::IsInverted)
		{
			ProjectionMatrix = FReversedZPerspectiveMatrix(
				FOV,
				FOV,
				XAxisMultiplier,
				YAxisMultiplier,
				GNearClippingPlane,
				GNearClippingPlane
			);
		}
		else
		{
			ProjectionMatrix = FPerspectiveMatrix(
				FOV,
				FOV,
				XAxisMultiplier,
				YAxisMultiplier,
				GNearClippingPlane,
				GNearClippingPlane
			);
		}
	}
	return ViewMatrix * ProjectionMatrix;
}

FVector2D USceneCaptureComponentMap::ProjectActorLocationToTextureLocation2D(AActor* Actor) const
{
	return FVector2D(ProjectActorLocationToTextureLocation(Actor));
}

FVector2D USceneCaptureComponentMap::ProjectSceneComponentLocationToTextureLocation2D(USceneComponent* Component) const
{
	return FVector2D(ProjectSceneComponentLocationToTextureLocation(Component));
}

FVector2D USceneCaptureComponentMap::ProjectLocationToTextureLocation2D(const FVector& WorldLocation) const
{
	return FVector2D(ProjectLocationToTextureLocation(WorldLocation));
}

void USceneCaptureComponentMap::GoToWorldPosition(const FVector& WorldLocation, FVector ClampAxis)
{
	const FVector GoToLocation = WorldLocation * ClampAxis;
	SetWorldLocation(GoToLocation);
}

void USceneCaptureComponentMap::Activate(bool bReset)
{
	Super::Activate(bReset);
	if (ParentMaterial)
	{
		RenderToMaterial = UMaterialInstanceDynamic::Create(ParentMaterial, nullptr);
		if (RenderToMaterial)
		{
			RenderToMaterial->SetTextureParameterValue(MaterialParameterName, TextureTarget);
		}
	}
}