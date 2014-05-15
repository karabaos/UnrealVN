// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#ifndef __LevelEditor_h__
#define __LevelEditor_h__

#include "UnrealEd.h"
#include "ILevelEditor.h"
#include "ModuleInterface.h"
#include "SEditorViewport.h"
#include "Toolkits/AssetEditorToolkit.h" // For FExtensibilityManager

extern const FName LevelEditorApp;

/**
 * Level editor module
 */
class FLevelEditorModule : public IModuleInterface,
	public IHasMenuExtensibility, public IHasToolBarExtensibility
{
public:
	/** Constructor, set up console commands and variables **/
	FLevelEditorModule();

	/**
	 * Called right after the module DLL has been loaded and the module object has been created
	 */
	virtual void StartupModule() OVERRIDE;

	/**
	 * Called before the module is unloaded, right before the module object is destroyed.
	 */
	virtual void ShutdownModule() OVERRIDE;

	/**
	 * Called before the module has been unloaded
	 */
	virtual void PreUnloadCallback() OVERRIDE;

	/**
	 * Called after the module has been reloaded
	 */
	virtual void PostLoadCallback() OVERRIDE;

	/**
	 * Override this to set whether your module is allowed to be unloaded on the fly
	 *
	 * @return Whether the module supports shutdown separate from the rest of the engine.
	 */
	virtual bool SupportsDynamicReloading() OVERRIDE
	{
		// @todo: Eventually, this should probably not be allowed.
		return true;
	}

	/**
	 * Spawns a new property viewer
	 * @todo This only works with the first level editor. Fix it.
	 */
	virtual void SummonSelectionDetails();

	/**
	 * Spawns a new build and submit widget
	 * @todo This only works with the first level editor. Fix it.
	 */
	virtual void SummonBuildAndSubmit();
	
	/**
	 * Spawns a new level browser tab
	 * @todo This only works with the first level editor. Fix it.
	 */
	virtual void SummonLevelBrowser();

	virtual void SummonWorldBrowserHierarchy();
	virtual void SummonWorldBrowserDetails();
	virtual void SummonWorldBrowserComposition();

	// @todo remove when world-centric mode is added
	/**
	 * Spawns a new sequencer tab if one doesn't exist already
	 * @todo This only works with the first level editor. Fix it.
	 */
	virtual void AttachSequencer(TSharedPtr<SWidget> Sequencer);

	/**
	 * Starts a play in editor session using the active viewport
	 */
	virtual void StartImmersivePlayInEditorSession();

	/**
	 * Toggles immersive mode on the currently active level viewport
	 */
	virtual void ToggleImmersiveOnActiveLevelViewport();

	/** @return Returns the first Level Editor that we currently know about */
	virtual TSharedPtr< class ILevelEditor > GetFirstLevelEditor();

	/** @return the dock tab in which the level editor currently resides. */
	virtual TSharedPtr<SDockTab> GetLevelEditorTab() const;

	/**
	 * Gets the first active viewport of all the viewports.
	 *
	 * @todo This only works with the first level editor. Fix it.
	 */
	virtual TSharedPtr<class ILevelViewport> GetFirstActiveViewport();

	/**
	 * Called to focus the level editor that has a play in editor viewport
	 */
	virtual void FocusPIEViewport();

	/**
	 * Called to focus the level editor viewport
	 */
	virtual void FocusViewport();

	/**
	 * @return The list of bound level editor commands that are common to all level editors
	 */
	virtual const TSharedRef<FUICommandList> GetGlobalLevelEditorActions() const { return GlobalLevelEditorActions.ToSharedRef(); }

	/**
	 * Method for getting level editor commands outside of this module                   
	 */
	virtual const class FLevelEditorCommands& GetLevelEditorCommands() const;

	/**
	 * Method for getting level editor modes commands outside of this module                   
	 */
	virtual const class FLevelEditorModesCommands& GetLevelEditorModesCommands() const;

	/**
	 * Method for getting level viewport commands outside of this module                   
	 */
	virtual const class FLevelViewportCommands& GetLevelViewportCommands() const;

	/* @return The pointer to the current level Editor instance */
	virtual TWeakPtr<class SLevelEditor> GetLevelEditorInstance() const;

	/* @return The pointer to the level editor tab */
	virtual TWeakPtr<class SDockTab> GetLevelEditorInstanceTab() const;

	/* @return The pointer to the level editor tab manager */
	virtual TSharedPtr<FTabManager> GetLevelEditorTabManager() const;

	/* Set the pointer to the current level Editor instance */
	virtual void SetLevelEditorInstance( TWeakPtr<class SLevelEditor> LevelEditor );

	/* Set the pointer to the level editor tab */
	virtual void SetLevelEditorInstanceTab( TWeakPtr<class SDockTab> LevelEditorTab );

	/* Create a tab manager for the level editor based on the given tab (or clears the tab manager if OwnerTab is null) */
	virtual void SetLevelEditorTabManager( const TSharedPtr<SDockTab>& OwnerTab );

	/**
	 * Called when actor selection changes
	 * 
	 * @param NewSelection	List of objects that are now selected
	 */
	virtual void BroadcastActorSelectionChanged( const TArray<UObject*>& NewSelection );
	
	/**
	 * Called by the engine when level editing viewports need to be redrawn
	 * 
	 * @param bInvalidateHitProxies	true to invalidate hit proxies
	 */
	virtual void BroadcastRedrawViewports( bool bInvalidateHitProxies );
	
	/**
	 * Called by the engine when level editing viewports need to take a high res screenshot
	 * 
	 */
	virtual void BroadcastTakeHighResScreenShots( );

	/**
	 * Called by the engine when a new map is loaded
	 * 
	 * @param World	The new world
	 */
	virtual void BroadcastMapChanged( UWorld* World, EMapChangeType::Type MapChangeType );

	/** Called when actor selection changes */
	DECLARE_EVENT_OneParam( FLevelEditorModule, FActorSelectionChangedEvent, const TArray<UObject*>& );
	virtual FActorSelectionChangedEvent& OnActorSelectionChanged() { return ActorSelectionChangedEvent; }

	/** Called when level editor viewports should be redrawn */
	DECLARE_EVENT_OneParam( FLevelEditorModule, FRedrawLevelEditingViewportsEvent, bool );
	virtual FRedrawLevelEditingViewportsEvent& OnRedrawLevelEditingViewports() { return RedrawLevelEditingViewportsEvent; }

	/** Called when a new map is loaded */
	DECLARE_EVENT_TwoParams( FLevelEditorModule, FMapChangedEvent, UWorld*, EMapChangeType::Type );
	virtual FMapChangedEvent& OnMapChanged() { return MapChangedEvent; }

	/** Delegates to be called to extend the level viewport menus */
	DECLARE_DELEGATE_RetVal_OneParam( TSharedRef<FExtender>, FLevelEditorMenuExtender, const TSharedRef<FUICommandList>);
	DECLARE_DELEGATE_RetVal_TwoParams( TSharedRef<FExtender>, FLevelViewportMenuExtender_SelectedObjects, const TSharedRef<FUICommandList>, const TArray<UObject*>);
	DECLARE_DELEGATE_RetVal_TwoParams( TSharedRef<FExtender>, FLevelViewportMenuExtender_SelectedActors, const TSharedRef<FUICommandList>, const TArray<AActor*>);
	virtual TArray<FLevelViewportMenuExtender_SelectedObjects>& GetAllLevelViewportDragDropContextMenuExtenders() {return LevelViewportDragDropContextMenuExtenders;}
	virtual TArray<FLevelViewportMenuExtender_SelectedActors>& GetAllLevelViewportContextMenuExtenders() {return LevelViewportContextMenuExtenders;}
	virtual TArray<FLevelEditorMenuExtender>& GetAllLevelViewportOptionsMenuExtenders() {return LevelViewportOptionsMenuExtenders;}
	virtual TArray<FLevelEditorMenuExtender>& GetAllLevelEditorToolbarViewMenuExtenders() {return LevelEditorToolbarViewMenuExtenders;}
	virtual TArray<FLevelEditorMenuExtender>& GetAllLevelEditorToolbarBuildMenuExtenders() {return LevelEditorToolbarBuildMenuExtenders;}
	virtual TArray<FLevelEditorMenuExtender>& GetAllLevelEditorToolbarCompileMenuExtenders() {return LevelEditorToolbarCompileMenuExtenders;}
	virtual TArray<FLevelEditorMenuExtender>& GetAllLevelEditorToolbarCreateMenuExtenders() {return LevelEditorToolbarCreateMenuExtenders;}
	
	/** Gets the extensibility managers for outside entities to extend static mesh editor's menus and toolbars */
	virtual TSharedPtr<FExtensibilityManager> GetMenuExtensibilityManager() {return MenuExtensibilityManager;}
	virtual TSharedPtr<FExtensibilityManager> GetToolBarExtensibilityManager() {return ToolBarExtensibilityManager;}
	virtual TSharedPtr<FExtensibilityManager> GetModeBarExtensibilityManager() {return ModeBarExtensibilityManager;}
	virtual TSharedPtr<FExtensibilityManager> GetNotificationBarExtensibilityManager() {return NotificationBarExtensibilityManager;}

	/** Called when a new map is loaded */
	DECLARE_EVENT( FLevelEditorModule, FNotificationBarChanged );
	virtual FNotificationBarChanged& OnNotificationBarChanged() { return NotificationBarChangedEvent; }

	virtual void BroadcastNotificationBarChanged() { NotificationBarChangedEvent.Broadcast(); }

	/** Called when a high res screenshot is requested. */
	DECLARE_EVENT( FLevelEditorModule, FTakeHighResScreenShotsEvent );
	virtual FTakeHighResScreenShotsEvent& OnTakeHighResScreenShots() { return TakeHighResScreenShotsEvent; }

	/** Determines whether the level editor can be recompiled on the fly. */
	inline bool CanBeRecompiled() const { return bCanBeRecompiled; }

private:
	/**
	 * Binds all global level editor commands to delegates
	 */
	void BindGlobalLevelEditorCommands();

	// Callback for persisting the Level Editor's layout.
	void HandleTabManagerPersistLayout( const TSharedRef<FTabManager::FLayout>& LayoutToSave )
	{
		FLayoutSaveRestore::SaveToConfig(GEditorLayoutIni, LayoutToSave);
	}

private:
	TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;
	TSharedPtr<FExtensibilityManager> ToolBarExtensibilityManager;
	TSharedPtr<FExtensibilityManager> ModeBarExtensibilityManager;
	TSharedPtr<FExtensibilityManager> NotificationBarExtensibilityManager;

	FNotificationBarChanged NotificationBarChangedEvent;

	/** 
	 * A command list that can be passed around and isn't bound to an instance of a level editor. 
	 * Only commands that arent bound to an instance of SLevelEditor should be used here (like level context menu commands)
	 */
	TSharedPtr<FUICommandList> GlobalLevelEditorActions;

	FAutoConsoleCommand ToggleImmersiveConsoleCommand;

	/** Multicast delegate executed when actor selection changes */
	FActorSelectionChangedEvent ActorSelectionChangedEvent;

	/** Multicast delegate executed when viewports should be redrawn */
	FRedrawLevelEditingViewportsEvent RedrawLevelEditingViewportsEvent;

	/** Multicast delegate executed when viewports should be redrawn */
	FTakeHighResScreenShotsEvent TakeHighResScreenShotsEvent;

	/** Multicast delegate executed when a map is changed (loaded,saved,new map, etc) */
	FMapChangedEvent MapChangedEvent;

	/** All extender delegates for the level viewport menus */
	TArray<FLevelViewportMenuExtender_SelectedObjects> LevelViewportDragDropContextMenuExtenders;
	TArray<FLevelViewportMenuExtender_SelectedActors> LevelViewportContextMenuExtenders;
	TArray<FLevelEditorMenuExtender> LevelViewportOptionsMenuExtenders;
	TArray<FLevelEditorMenuExtender> LevelEditorToolbarViewMenuExtenders;
	TArray<FLevelEditorMenuExtender> LevelEditorToolbarBuildMenuExtenders;
	TArray<FLevelEditorMenuExtender> LevelEditorToolbarCompileMenuExtenders;
	TArray<FLevelEditorMenuExtender> LevelEditorToolbarCreateMenuExtenders;

	/* Pointer to the current level Editor instance */
	TWeakPtr<class SLevelEditor> LevelEditorInstancePtr;

	/* Pointer to the level editor tab */
	TWeakPtr<class SDockTab> LevelEditorInstanceTabPtr;

	/* Holds the Editor's tab manager */
	TSharedPtr<FTabManager> LevelEditorTabManager;

	/* Whether we have source for the level editor, and it can be recompiled. */
	bool bCanBeRecompiled;
};

#endif // __LevelEditor_h__
