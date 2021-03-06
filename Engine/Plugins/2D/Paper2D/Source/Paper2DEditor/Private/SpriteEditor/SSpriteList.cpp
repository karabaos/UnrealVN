// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "Paper2DEditorPrivatePCH.h"
#include "SSpriteList.h"
#include "AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "SpriteEditor.h"

//////////////////////////////////////////////////////////////////////////
// SSpriteList

void SSpriteList::Construct(const FArguments& InArgs, TSharedPtr<FSpriteEditor> InSpriteEditor)
{
	SpriteEditorPtr = InSpriteEditor;
	
	UTexture2D* CurrentTexture = InSpriteEditor->GetSourceTexture();
	SourceTexturePtr = CurrentTexture;

	RebuildWidget(CurrentTexture);
}

void SSpriteList::RebuildWidget(UTexture2D* NewTextureFilter)
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	// Configure filter for asset picker
	FAssetPickerConfig Config;

	//Config.Filter.bRecursiveClasses = true;
	Config.Filter.ClassNames.Add(UPaperSprite::StaticClass()->GetFName());

	Config.ThumbnailScale = 0.0f;
	Config.InitialAssetViewType = EAssetViewType::Tile;

	if (NewTextureFilter != nullptr)
	{
		const FString TextureString = FAssetData(NewTextureFilter).GetExportTextName();
		const FName SourceTexturePropName(TEXT("SourceTexture")); //@TODO: Protected: GET_MEMBER_NAME_CHECKED(UPaperSprite, SourceTexture);
		Config.Filter.TagsAndValues.Add(SourceTexturePropName, TextureString);

		// Ignore the source texture since we're filtering against it
		AssetRegistryTagsToIgnore.Add(SourceTexturePropName);
	}
	else
	{
		AssetRegistryTagsToIgnore.Empty();
	}

	// Configure response to click and double-click
	Config.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SSpriteList::OnSpriteSelected);
	Config.OnAssetDoubleClicked = FOnAssetDoubleClicked::CreateSP(this, &SSpriteList::OnSpriteDoubleClicked);
	//Config.OnGetAssetContextMenu = FOnGetAssetContextMenu::CreateSP(this, &SSpriteList::OnGetAssetContextMenu);
	Config.OnAssetTagWantsToBeDisplayed = FOnShouldDisplayAssetTag::CreateSP(this, &SSpriteList::CanShowColumnForAssetRegistryTag);
	Config.bFocusSearchBoxWhenOpened = false;

	this->ChildSlot
	[
		ContentBrowserModule.Get().CreateAssetPicker(Config)
	];
}

void SSpriteList::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	TSharedPtr<FSpriteEditor> SpriteEditor = SpriteEditorPtr.Pin();
	if (SpriteEditor.IsValid())
	{
		// Check to see if the source texture has changed; if so, we need to rebuild the list
		UTexture2D* CurrentTexture = SpriteEditor->GetSourceTexture();
		if (CurrentTexture != SourceTexturePtr.Get())
		{
			SourceTexturePtr = CurrentTexture;
			RebuildWidget(CurrentTexture);
		}
	}
}

void SSpriteList::OnSpriteSelected(const FAssetData& AssetData)
{
}

void SSpriteList::OnSpriteDoubleClicked(const FAssetData& AssetData)
{
	TSharedPtr<FSpriteEditor> SpriteEditor = SpriteEditorPtr.Pin();
	if (SpriteEditor.IsValid())
	{
		if (UPaperSprite* NewSpriteToEdit = Cast<UPaperSprite>(AssetData.GetAsset()))
		{
			SpriteEditor->SetSpriteBeingEdited(NewSpriteToEdit);
		}
	}
}

bool SSpriteList::CanShowColumnForAssetRegistryTag(FName AssetType, FName TagName) const
{
	return !AssetRegistryTagsToIgnore.Contains(TagName);
}
