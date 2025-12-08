#pragma once

class UAnimInstance;
class UEdGraphNode;
class UEdGraphPin;
class UAnimationGraph;
class UAnimationStateMachine;
/**
 * @brief 노드 그래프를 순회하여 런타임용 AnimationStateMachine을 빌드하는 컴파일러
 */
class FAnimBlueprintCompiler
{
public:
    /**
     * @brief 애니메이션 그래프를 받아 상태 머신을 구축한다. 
     */
    static void Compile(UAnimationGraph* InGraph, UAnimInstance* InAnimInstance, UAnimationStateMachine* OutStateMachine);

private:
    /**
     * @brief 특정 핀에 연결된 전/후 노드를 찾는다.
     * @note 실행 흐름은 하나의 출력핀에 여러개의 입력핀이 연결되는 것이 가능하다.
     *       하지만, 구현의 용이성을 위해 가장 먼저 탐색되는 노드만을 반환한다.
     */
    static UEdGraphNode* GetConnectedNode(UEdGraphPin* Pin);

	static TArray<UEdGraphNode*> GetConnectedNodes(UEdGraphPin* Pin);
};
