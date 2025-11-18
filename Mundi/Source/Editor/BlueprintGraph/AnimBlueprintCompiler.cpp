#include "pch.h"
#include "AnimBlueprintCompiler.h"
#include "AnimationGraph.h"
#include "BlueprintEvaluator.h"
#include "EdGraphNode.h"
#include "K2Node_Animation.h"
#include "Source/Runtime/Engine/Animation/AnimInstance.h"
#include "Source/Runtime/Engine/Animation/AnimationStateMachine.h"

void FAnimBlueprintCompiler::Compile(UAnimationGraph* InGraph, UAnimInstance* InAnimInstance, UAnimationStateMachine* OutStateMachine)
{
    if (!InGraph)
    {
        return;
    }

    OutStateMachine->Clear();

    TMap<UEdGraphNode*, FName> NodeToStateMap;
    
    FBlueprintContext Context(InAnimInstance);

    // --- #1. 모든 상태 노드 등록 ---
    for (UEdGraphNode* Node : InGraph->Nodes)
    {
        if (UK2Node_AnimState* StateNode = Cast<UK2Node_AnimState>(Node))
        {
            UAnimSequence* AnimSeq = FBlueprintEvaluator::EvaluateInput<UAnimSequence*>(StateNode->FindPin("Animation"), &Context);
            bool bLoop = FBlueprintEvaluator::EvaluateInput<bool>(StateNode->FindPin("Looping"), &Context);
            float PlayRate = FBlueprintEvaluator::EvaluateInput<float>(StateNode->FindPin("PlayRate"), &Context);

            FName StateName = FName(StateNode->StateName);

            FAnimationState NewState(StateName, AnimSeq, bLoop, PlayRate);
            OutStateMachine->AddState(NewState);

            NodeToStateMap.Add(StateNode, StateName);
        }
    }

    // --- #2. 진입점 생성 ---
    UK2Node_AnimStateEntry* EntryNode = InGraph->GetEntryNode();
    if (EntryNode)
    {
        if (UEdGraphPin* ExecPin = EntryNode->FindPin("Entry", EEdGraphPinDirection::EGPD_Output))
        {
            UEdGraphNode* NextNode = GetConnectedNode(ExecPin);
            if (NextNode && NodeToStateMap.Contains(NextNode))
            {
                OutStateMachine->SetInitialState(NodeToStateMap[NextNode]);
            }
        }
    }

    // --- #3. 전이 규칙 등록 ---
    for (UEdGraphNode* Node : InGraph->Nodes)
    {
        if (UK2Node_AnimTransition* TransitionNode = Cast<UK2Node_AnimTransition>(Node))
        {
            UEdGraphPin* InputExec = TransitionNode->FindPin("Execute", EEdGraphPinDirection::EGPD_Input);
            UEdGraphPin* OutputExec = TransitionNode->FindPin("Transition To", EEdGraphPinDirection::EGPD_Output);

            UEdGraphNode* FromNode = GetConnectedNode(InputExec);
            UEdGraphNode* ToNode = GetConnectedNode(OutputExec);

            if (FromNode && ToNode && NodeToStateMap.Contains(FromNode) && NodeToStateMap.Contains(ToNode))
            {
                FName FromName = NodeToStateMap[FromNode];
                FName ToName = NodeToStateMap[ToNode];

                float BlendTime = FBlueprintEvaluator::EvaluateInput<float>(TransitionNode->FindPin("Blend Time"), &Context);

                auto Condition = [TransitionNode, InAnimInstance]() -> bool
                {
                    if (!TransitionNode || !InAnimInstance)
                    {
                        return false;
                    }

                    FBlueprintContext RuntimeContext(InAnimInstance);

                    return FBlueprintEvaluator::EvaluateInput<bool>(
                        TransitionNode->FindPin("Can Transition"),
                        &RuntimeContext
                    );
                };

                OutStateMachine->AddTransition(FStateTransition(FromName, ToName, Condition, BlendTime));
            }
        }
    }
}

UEdGraphNode* FAnimBlueprintCompiler::GetConnectedNode(UEdGraphPin* Pin)
{
    if (!Pin || Pin->LinkedTo.Num() == 0)
    {
        return nullptr;
    }
    return Pin->LinkedTo[0]->OwningNode;
}
