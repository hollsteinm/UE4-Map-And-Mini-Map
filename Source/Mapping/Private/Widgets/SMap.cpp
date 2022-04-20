#include "MappingPrivatePCH.h"
#include "Widgets/SMap.h"
#include "Widgets/SCanvas.h"
#include "SceneMapComponent.h"

void SMap::Construct(const FArguments& InArgs)
{
	ChildSlot
		[
			SAssignNew(Canvas, SCanvas)
		];

	if (InArgs._CaptureComponent)
	{
		SAssignNew(RenderImage, SImage)
			.Image(&MapBrush);

		SetCaptureComponent(InArgs._CaptureComponent);

		if (Canvas.IsValid() && RenderImage.IsValid())
		{
			MapSlot = &Canvas->AddSlot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Position(MapBrush.ImageSize / 2.0f)
				.Size(MapBrush.ImageSize)
				[
					RenderImage.ToSharedRef()
				];
		}
	}
}

SMap::~SMap()
{

}

void SMap::SetCaptureComponent(USceneCaptureComponentMap* NewMapCaptureComponent)
{
	Map = NewMapCaptureComponent;
	if (Map.IsValid())
	{
		if (Map->TextureTarget && Map->GetMaterialInstance())
		{
			MapBrush.SetResourceObject(Map->GetMaterialInstance());
			MapBrush.DrawAs = ESlateBrushDrawType::Image;
			MapBrush.ImageSize.X = Map->TextureTarget->SizeX;
			MapBrush.ImageSize.Y = Map->TextureTarget->SizeY;
		}
	} 
	else
	{
		MapBrush.SetResourceObject(nullptr);
		MapBrush.ImageSize = FVector2D::ZeroVector;
		MapBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
	}
	if (MapSlot != nullptr)
	{
		MapSlot->Position(MapBrush.ImageSize / 2.0f);
		MapSlot->Size(MapBrush.ImageSize);
	}
	Invalidate(EInvalidateWidget::LayoutAndVolatility);
}

void SMap::Add(USceneMapComponent* Component)
{
	if (Component &&
		!MapIcons.Contains(Component) &&
		Map.IsValid() &&
		Canvas.IsValid())
	{
		TWeakObjectPtr<USceneMapComponent> ToAdd(Component);
		TSharedRef<SWidget> Content = OnGenerateChildIcon(Component, Map.Get());
		MapIcons.Add(ToAdd, Content);
		Canvas->AddSlot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Size(Component->MapIcon.ImageSize)
			.Position(CreateComponentToMapPositionAttribute(Component))
			[
				Content
			];
	}
}

void SMap::Remove(USceneMapComponent* Component)
{
	if (Component &&
		MapIcons.Contains(Component) &&
		Canvas.IsValid())
	{
		TSharedRef<SWidget> ToRemove = MapIcons[Component];
		Canvas->RemoveSlot(ToRemove);
		MapIcons.Remove(Component);
	}
}

void SMap::RemoveAll()
{
	RemoveAllWithSlack(0);
}

void SMap::SetAll(const TArray<USceneMapComponent*>& NewSceneComponents)
{
	RemoveAllWithSlack(NewSceneComponents.Num());
	for (auto NewComponent : NewSceneComponents)
	{
		Add(NewComponent);
	}
}

void SMap::RemoveAllWithSlack(int32 Slack)
{
	TArray<TWeakObjectPtr<USceneMapComponent>> Components;
	Components.Reserve(MapIcons.Num());
	if (MapIcons.GetKeys(Components))
	{
		for (auto Component : Components)
		{
			if (Component.IsValid())
			{
				Remove(Component.Get());
			}
		}
		MapIcons.Empty(Slack);
	}
}

FVector2D SMap::ComputeDesiredSize(float) const
{
	return MapBrush.ImageSize;
}

TSharedRef<SWidget> SMap::OnGenerateChildIcon(USceneMapComponent* Component, USceneCaptureComponentMap* CurrentMap) const
{
	return SNew(SImage)
		.Image(&Component->MapIcon)
		.Visibility(this, &SMap::GetComponentVisibility, Component)
		.RenderTransformPivot(FVector2D(0.5f, 0.5f))
		.RenderTransform_Lambda([Component]() -> FSlateRenderTransform
		{
			if (Component)
			{
				return TransformCast<FSlateRenderTransform>(Concatenate(FScale2D(1.0f, 1.0f), FShear2D(0.0f, 0.0f), FQuat2D(FMath::DegreesToRadians(Component->GetComponentRotation().Yaw))));
			}
			else
			{
				return FSlateRenderTransform();
			}
		});
}

FVector2D SMap::WorldLocationToMap(const FVector& WorldLocation) const
{
	return Map.IsValid() ? (Map->ProjectLocationToTextureLocation2D(WorldLocation)) : FVector2D();
}

TAttribute<FVector2D> SMap::CreateComponentToMapPositionAttribute(USceneMapComponent* Component) const
{
	auto ThisShared = SharedThis(this);
	return TAttribute<FVector2D>::Create([ThisShared, InputComponent]() -> FVector2D
	{
		if (InputComponent)
		{
			if(InputComponent->ClampToMapEdge())
			{
				if(ThisShared->Map.IsValid())
				{
					auto CurrentMapLocation = ThisShared->WorldLocationToMap(InputComponent->GetComponentLocation());
					auto ViewRect = ThisShared->Map->GetViewRect();
					if(ViewRect.ContainsPoint(CurrentMapLocation))
					{
						return CurrentMapLocation;
					}
					else
					{
						auto Center = ViewRect.GetCenter();
						auto ClampX = FMath::Clamp(CurrentMapLocation.X, Center.X - ViewRect.Left, Center.X + ViewRight.Right);
						auto ClampY = FMath::Clamp(CurrentMapLocation.Y, Center.Y - ViewRect.Bottom, Center.Y + ViewRect.Top);
						return FVector2D(ClampX, ClampY);
					}
				}
				else
				{
					return FVector2D::ZeroVector;
				}
			}
			else
			{
				return ThisShared->WorldLocationToMap(InputComponent->GetComponentLocation());
			}
		}
		else
		{
			return FVector2D::ZeroVector;
		}
	});
}

EVisibility SMap::GetComponentVisibility(USceneMapComponent* Component) const
{
	return (!Component->bVisible || Component->bHiddenInGame) ? EVisibility::Collapsed : EVisibility::Visible;
}