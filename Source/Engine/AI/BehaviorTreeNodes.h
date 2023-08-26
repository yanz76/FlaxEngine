// Copyright (c) 2012-2023 Wojciech Figat. All rights reserved.

#pragma once

#include "BehaviorKnowledge.h"
#include "BehaviorTreeNode.h"
#include "BehaviorKnowledgeSelector.h"
#include "Engine/Core/Collections/Array.h"
#include "Engine/Core/Collections/BitArray.h"
#include "Engine/Content/AssetReference.h"
#include "Engine/Level/Tags.h"

class Actor;

/// <summary>
/// Base class for compound Behavior Tree nodes that composite child nodes.
/// </summary>
API_CLASS(Abstract) class FLAXENGINE_API BehaviorTreeCompoundNode : public BehaviorTreeNode
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(BehaviorTreeCompoundNode, BehaviorTreeNode);

    /// <summary>
    /// List with all child nodes.
    /// </summary>
    API_FIELD(Readonly) Array<BehaviorTreeNode*, InlinedAllocation<8>> Children;

public:
    // [BehaviorTreeNode]
    void Init(BehaviorTree* tree) override;
    BehaviorUpdateResult Update(const BehaviorUpdateContext& context) override;

protected:
    // [BehaviorTreeNode]
    void BecomeIrrelevant(const BehaviorUpdateContext& context) override;
};

/// <summary>
/// Sequence node updates all its children (from left to right) as long as they return success. If any child fails, the sequence is failed.
/// </summary>
API_CLASS() class FLAXENGINE_API BehaviorTreeSequenceNode : public BehaviorTreeCompoundNode
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(BehaviorTreeSequenceNode, BehaviorTreeCompoundNode);

public:
    // [BehaviorTreeNode]
    int32 GetStateSize() const override;
    void InitState(const BehaviorUpdateContext& context) override;
    BehaviorUpdateResult Update(const BehaviorUpdateContext& context) override;

private:
    struct State
    {
        int32 CurrentChildIndex = 0;
    };
};

/// <summary>
/// Selector node updates all its children (from left to right) until one of them succeeds. If all children fail, the selector fails.
/// </summary>
API_CLASS() class FLAXENGINE_API BehaviorTreeSelectorNode : public BehaviorTreeCompoundNode
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(BehaviorTreeSelectorNode, BehaviorTreeCompoundNode);

public:
    // [BehaviorTreeNode]
    int32 GetStateSize() const override;
    void InitState(const BehaviorUpdateContext& context) override;
    BehaviorUpdateResult Update(const BehaviorUpdateContext& context) override;

private:
    struct State
    {
        int32 CurrentChildIndex = 0;
    };
};

/// <summary>
/// Root node of the behavior tree. Contains logic properties and definitions for the runtime.
/// </summary>
API_CLASS(Sealed) class FLAXENGINE_API BehaviorTreeRootNode : public BehaviorTreeSequenceNode
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(BehaviorTreeRootNode, BehaviorTreeSequenceNode);
    API_AUTO_SERIALIZATION();

    // Full typename of the blackboard data type (structure or class). Spawned for each instance of the behavior.
    API_FIELD(Attributes="EditorOrder(0), TypeReference(\"\", \"IsValidBlackboardType\"), CustomEditorAlias(\"FlaxEditor.CustomEditors.Editors.TypeNameEditor\")")
    StringAnsi BlackboardType;

    // The target amount of the behavior logic updates per second.
    API_FIELD(Attributes="EditorOrder(10)")
    float UpdateFPS = 10.0f;
};

/// <summary>
/// Delay node that waits a specific amount of time while executed.
/// </summary>
API_CLASS(Sealed) class FLAXENGINE_API BehaviorTreeDelayNode : public BehaviorTreeNode
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(BehaviorTreeDelayNode, BehaviorTreeNode);
    API_AUTO_SERIALIZATION();

    // Time in seconds to wait when node gets activated. Unused if WaitTimeSelector is used.
    API_FIELD(Attributes="EditorOrder(10), Limit(0)")
    float WaitTime = 3.0f;

    // Wait time randomization range to deviate original value.
    API_FIELD(Attributes="EditorOrder(20), Limit(0)")
    float RandomDeviation = 0.0f;

    // Wait time from behavior's knowledge (blackboard, goal or sensor). If set, overrides WaitTime but still uses RandomDeviation.
    API_FIELD(Attributes="EditorOrder(30)")
    BehaviorKnowledgeSelector<float> WaitTimeSelector;

public:
    // [BehaviorTreeNode]
    int32 GetStateSize() const override;
    void InitState(const BehaviorUpdateContext& context) override;
    BehaviorUpdateResult Update(const BehaviorUpdateContext& context) override;

private:
    struct State
    {
        float TimeLeft;
    };
};

/// <summary>
/// Sub-tree node runs a nested Behavior Tree within this tree.
/// </summary>
API_CLASS(Sealed) class FLAXENGINE_API BehaviorTreeSubTreeNode : public BehaviorTreeNode
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(BehaviorTreeSubTreeNode, BehaviorTreeNode);
    API_AUTO_SERIALIZATION();

    // Nested behavior tree to execute within this node.
    API_FIELD(Attributes="EditorOrder(10), Limit(0)")
    AssetReference<BehaviorTree> Tree;

public:
    // [BehaviorTreeNode]
    int32 GetStateSize() const override;
    void InitState(const BehaviorUpdateContext& context) override;
    void ReleaseState(const BehaviorUpdateContext& context) override;
    BehaviorUpdateResult Update(const BehaviorUpdateContext& context) override;

    struct State
    {
        Array<byte> Memory;
        BitArray<> RelevantNodes;
    };
};

/// <summary>
/// Forces behavior execution end with a specific result (eg. force fail).
/// </summary>
API_CLASS(Sealed) class FLAXENGINE_API BehaviorTreeForceFinishNode : public BehaviorTreeNode
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(BehaviorTreeForceFinishNode, BehaviorTreeNode);
    API_AUTO_SERIALIZATION();

    // Execution result.
    API_FIELD() BehaviorUpdateResult Result = BehaviorUpdateResult::Success;

public:
    // [BehaviorTreeNode]
    BehaviorUpdateResult Update(const BehaviorUpdateContext& context) override;
};

/// <summary>
/// Inverts node's result - fails if node succeeded or succeeds if node failed.
/// </summary>
API_CLASS(Sealed) class FLAXENGINE_API BehaviorTreeInvertDecorator : public BehaviorTreeDecorator
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(BehaviorTreeInvertDecorator, BehaviorTreeDecorator);

public:
    // [BehaviorTreeNode]
    void PostUpdate(const BehaviorUpdateContext& context, BehaviorUpdateResult& result) override;
};

/// <summary>
/// Forces node to success - even if it failed.
/// </summary>
API_CLASS(Sealed) class FLAXENGINE_API BehaviorTreeForceSuccessDecorator : public BehaviorTreeDecorator
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(BehaviorTreeForceSuccessDecorator, BehaviorTreeDecorator);

public:
    // [BehaviorTreeNode]
    void PostUpdate(const BehaviorUpdateContext& context, BehaviorUpdateResult& result) override;
};

/// <summary>
/// Forces node to fail - even if it succeeded.
/// </summary>
API_CLASS(Sealed) class FLAXENGINE_API BehaviorTreeForceFailedDecorator : public BehaviorTreeDecorator
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(BehaviorTreeForceFailedDecorator, BehaviorTreeDecorator);

public:
    // [BehaviorTreeNode]
    void PostUpdate(const BehaviorUpdateContext& context, BehaviorUpdateResult& result) override;
};

/// <summary>
/// Loops node execution multiple times as long as it doesn't fail. Returns the last iteration result.
/// </summary>
API_CLASS(Sealed) class FLAXENGINE_API BehaviorTreeLoopDecorator : public BehaviorTreeDecorator
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(BehaviorTreeLoopDecorator, BehaviorTreeDecorator);
    API_AUTO_SERIALIZATION();

    // Amount of times to execute the node. Unused if LoopCountSelector is used.
    API_FIELD(Attributes="EditorOrder(10), Limit(0)")
    int32 LoopCount = 3;

    // Amount of times to execute the node from behavior's knowledge (blackboard, goal or sensor). If set, overrides LoopCount.
    API_FIELD(Attributes="EditorOrder(20)")
    BehaviorKnowledgeSelector<int32> LoopCountSelector;

public:
    // [BehaviorTreeNode]
    int32 GetStateSize() const override;
    void InitState(const BehaviorUpdateContext& context) override;
    void PostUpdate(const BehaviorUpdateContext& context, BehaviorUpdateResult& result) override;

    struct State
    {
        int32 Loops;
    };
};

/// <summary>
/// Limits maximum duration of the node execution time (in seconds). Node will fail if it runs out of time.
/// </summary>
API_CLASS(Sealed) class FLAXENGINE_API BehaviorTreeTimeLimitDecorator : public BehaviorTreeDecorator
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(BehaviorTreeTimeLimitDecorator, BehaviorTreeDecorator);
    API_AUTO_SERIALIZATION();

    // Maximum node execution time (in seconds). Unused if MaxDurationSelector is used.
    API_FIELD(Attributes="EditorOrder(10), Limit(0)")
    float MaxDuration = 3.0;

    // Duration time randomization range to deviate original value.
    API_FIELD(Attributes="EditorOrder(20), Limit(0)")
    float RandomDeviation = 0.0f;

    // Maximum node execution time (in seconds) from behavior's knowledge (blackboard, goal or sensor). If set, overrides MaxDuration but still uses RandomDeviation.
    API_FIELD(Attributes="EditorOrder(20)")
    BehaviorKnowledgeSelector<float> MaxDurationSelector;

public:
    // [BehaviorTreeNode]
    int32 GetStateSize() const override;
    void InitState(const BehaviorUpdateContext& context) override;
    BehaviorUpdateResult Update(const BehaviorUpdateContext& context) override;

    struct State
    {
        float TimeLeft;
    };
};

/// <summary>
/// Adds cooldown in between node executions.
/// </summary>
API_CLASS(Sealed) class FLAXENGINE_API BehaviorTreeCooldownDecorator : public BehaviorTreeDecorator
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(BehaviorTreeCooldownDecorator, BehaviorTreeDecorator);
    API_AUTO_SERIALIZATION();

    // Minimum cooldown time (in seconds). Unused if MinDurationSelector is used.
    API_FIELD(Attributes="EditorOrder(10), Limit(0)")
    float MinDuration = 3.0;

    // Duration time randomization range to deviate original value.
    API_FIELD(Attributes="EditorOrder(20), Limit(0)")
    float RandomDeviation = 0.0f;

    // Minimum cooldown time (in seconds) from behavior's knowledge (blackboard, goal or sensor). If set, overrides MinDuration but still uses RandomDeviation.
    API_FIELD(Attributes="EditorOrder(20)")
    BehaviorKnowledgeSelector<float> MinDurationSelector;

public:
    // [BehaviorTreeNode]
    int32 GetStateSize() const override;
    void InitState(const BehaviorUpdateContext& context) override;
    void ReleaseState(const BehaviorUpdateContext& context) override;
    bool CanUpdate(const BehaviorUpdateContext& context) override;
    void PostUpdate(const BehaviorUpdateContext& context, BehaviorUpdateResult& result) override;

    struct State
    {
        float EndTime;
    };
};

/// <summary>
/// Checks certain knowledge value to conditionally enter the node.
/// </summary>
API_CLASS(Sealed) class FLAXENGINE_API BehaviorTreeKnowledgeConditionalDecorator : public BehaviorTreeDecorator
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(BehaviorTreeKnowledgeConditionalDecorator, BehaviorTreeDecorator);
    API_AUTO_SERIALIZATION();

    // The first value from behavior's knowledge (blackboard, goal or sensor) to use for comparision.
    API_FIELD(Attributes="EditorOrder(0)")
    BehaviorKnowledgeSelectorAny ValueA;

    // The second value to use for comparision.
    API_FIELD(Attributes="EditorOrder(10)")
    float ValueB = 0.0f;

    // Values comparision mode.
    API_FIELD(Attributes="EditorOrder(20)")
    BehaviorValueComparison Comparison = BehaviorValueComparison::Equal;

public:
    // [BehaviorTreeNode]
    bool CanUpdate(const BehaviorUpdateContext& context) override;
};

/// <summary>
/// Checks certain knowledge value to conditionally enter the node.
/// </summary>
API_CLASS(Sealed) class FLAXENGINE_API BehaviorTreeKnowledgeValuesConditionalDecorator : public BehaviorTreeDecorator
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(BehaviorTreeKnowledgeValuesConditionalDecorator, BehaviorTreeDecorator);
    API_AUTO_SERIALIZATION();

    // The first value from behavior's knowledge (blackboard, goal or sensor) to use for comparision.
    API_FIELD(Attributes="EditorOrder(0)")
    BehaviorKnowledgeSelectorAny ValueA;

    // The second value from behavior's knowledge (blackboard, goal or sensor) to use for comparision.
    API_FIELD(Attributes="EditorOrder(10)")
    BehaviorKnowledgeSelectorAny ValueB;

    // Values comparision mode.
    API_FIELD(Attributes="EditorOrder(20)")
    BehaviorValueComparison Comparison = BehaviorValueComparison::Equal;

public:
    // [BehaviorTreeNode]
    bool CanUpdate(const BehaviorUpdateContext& context) override;
};

/// <summary>
/// Checks if certain actor (from knowledge) has a given tag assigned.
/// </summary>
API_CLASS(Sealed) class FLAXENGINE_API BehaviorTreeHasTagDecorator : public BehaviorTreeDecorator
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(BehaviorTreeHasTagDecorator, BehaviorTreeDecorator);
    API_AUTO_SERIALIZATION();

    // The actor value from behavior's knowledge (blackboard, goal or sensor) to check against tag ownership.
    API_FIELD(Attributes="EditorOrder(0)")
    BehaviorKnowledgeSelector<Actor*> Actor;

    // The tag to check.
    API_FIELD(Attributes="EditorOrder(10)")
    Tag Tag;

    // If checked, inverts condition - checks if actor doesn't have a tag.
    API_FIELD(Attributes="EditorOrder(20)")
    bool Invert = false;

public:
    // [BehaviorTreeNode]
    bool CanUpdate(const BehaviorUpdateContext& context) override;
};