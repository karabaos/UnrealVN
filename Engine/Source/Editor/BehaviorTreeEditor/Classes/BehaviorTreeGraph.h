// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AIGraph.h"
#include "BehaviorTreeGraph.generated.h"

UCLASS()
class UBehaviorTreeGraph : public UAIGraph
{
	GENERATED_UCLASS_BODY()

	enum EUpdateFlags
	{
		RebuildGraph = 0,
		ClearDebuggerFlags = 1,
		KeepRebuildCounter = 2,
	};

	/** increased with every graph rebuild, used to refresh data from subtrees */
	UPROPERTY()
	int32 ModCounter;

	UPROPERTY()
	bool bIsUsingModCounter;

	virtual void UpdateVersion() override;
	virtual void MarkVersion() override;
	virtual void UpdateAsset(int32 UpdateFlags = 0) override;
	virtual void OnSubNodeDropped() override;

	void UpdateBlackboardChange();
	void UpdateAbortHighlight(struct FAbortDrawHelper& Mode0, struct FAbortDrawHelper& Mode1);
	void CreateBTFromGraph(class UBehaviorTreeGraphNode* RootEdNode);
	void SpawnMissingNodes();
	void UpdatePinConnectionTypes();
	void UpdateDeprecatedNodes();
	bool UpdateInjectedNodes();
	class UEdGraphNode* FindInjectedNode(int32 Index);
	void ReplaceNodeConnections(UEdGraphNode* OldNode, UEdGraphNode* NewNode);
	void RebuildExecutionOrder();

	void AutoArrange();

protected:

	virtual void CollectAllNodeInstances(TSet<UObject*>& NodeInstances);

	void UpdateVersion_UnifiedSubNodes();
};
