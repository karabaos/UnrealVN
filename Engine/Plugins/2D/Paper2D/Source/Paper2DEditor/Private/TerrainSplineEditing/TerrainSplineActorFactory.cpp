// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "Paper2DEditorPrivatePCH.h"
#include "AssetData.h"

//////////////////////////////////////////////////////////////////////////
// UTerrainSplineActorFactory

UTerrainSplineActorFactory::UTerrainSplineActorFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Paper2D", "TerrainSplineFactoryDisplayName", "Paper2D Terrain Spline");
	NewActorClass = APaperTerrainActor::StaticClass();
}

void UTerrainSplineActorFactory::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	if (UPaperTerrainMaterial* TerrainMaterial = Cast<UPaperTerrainMaterial>(Asset))
	{
		GEditor->SetActorLabelUnique(NewActor, TerrainMaterial->GetName());

		APaperTerrainActor* TypedActor = CastChecked<APaperTerrainActor>(NewActor);
		UPaperTerrainComponent* RenderComponent = TypedActor->GetRenderComponent();
		check(RenderComponent);

		RenderComponent->UnregisterComponent();
		RenderComponent->TerrainMaterial = TerrainMaterial;
		RenderComponent->RegisterComponent();
	}
}

void UTerrainSplineActorFactory::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	checkf(false, TEXT("APaperTerrainActor isn't blueprintable; how did you get here?"));
}

bool UTerrainSplineActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (GetDefault<UPaperRuntimeSettings>()->bEnableTerrainSplineEditing)
	{
		if (AssetData.IsValid() && AssetData.GetClass()->IsChildOf(UPaperTerrainMaterial::StaticClass()))
		{
			return true;
		}
		else
		{
			return Super::CanCreateActorFrom(AssetData, OutErrorMsg);
		}
	}
	else
	{
		return false;
	}
}
