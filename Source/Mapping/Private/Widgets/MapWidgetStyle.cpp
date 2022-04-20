// Fill out your copyright notice in the Description page of Project Settings.

#include "MappingPrivatePCH.h"
#include "Widgets/MapWidgetStyle.h"
#include "SlateDynamicImageBrush.h"
#include "IPluginManager.h"


FMapStyle::FMapStyle()
{
	TSharedPtr<IPlugin> MappingPlugin = IPluginManager::Get().FindPlugin("Mapping");
	if (MappingPlugin.IsValid())
	{
		FString PluginContentPath = MappingPlugin->GetContentDir();
		BackgroundImage = FSlateDynamicImageBrush(FName(*(PluginContentPath / TEXT("DefaultBackground_640x360.png"))), FVector2D(640, 360));
		ComponentBrush = FSlateDynamicImageBrush(FName(*(PluginContentPath / TEXT("MapIconNuetral_256x256.png"))), FVector2D(64, 64));
	}
}

FMapStyle::~FMapStyle()
{

}

const FName FMapStyle::TypeName(TEXT("FMapStyle"));

const FMapStyle& FMapStyle::GetDefault()
{
	static FMapStyle Default;
	return Default;
}

void FMapStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	OutBrushes.Add(&BackgroundImage);
	OutBrushes.Add(&ComponentBrush);
}