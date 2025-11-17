#pragma once

class UEdGraph;
class UEdGraphNode;

UCLASS(DiaplayName="UBluePrintNodeSpawner", Description="블루프린트 노드 스포너")
class UBlueprintNodeSpawner : public UObject
{
    DECLARE_CLASS(UBlueprintNodeSpawner, UObject)
    
public:
    /** FBlueprintActionUiSpec에 대응되는 변수들 */
    FString MenuName;
    FString Category;
    FString Tooltip;

    /**
     * @brief 생성할 노드의 클래스
     * 
     * @note UEdGraphNode의 하위 클래스이어야 한다.
     */
    UClass* NodeClass;

    /** @brief NodeClass로부터 새 노드를 생성한다. */
    [[nodiscard]] UEdGraphNode* SpawnNode(UEdGraph* Graph, ImVec2 Position);

    /** @brief UClass 타입으로부터 스포너를 생성하는 팩토리 함수 */
    static UBlueprintNodeSpawner* Create(UClass* InNodeClass);
};

/**
 * @brief GetMenuActions에 전달되어 스포너를 수집하는 등록소 클래스 (UE5 FBlueprintActionDatabaseRegistrar)
 */
class FBlueprintActionDatabaseRegistrar
{
public:
    TArray<UBlueprintNodeSpawner*> Spawners;

    void AddAction(UBlueprintNodeSpawner* Spawner)
    {
        if (Spawner)
        {
            Spawners.Add(Spawner);
        }
    }
};

/**
 * @brief 모든 블루프린트 액션(스포너)을 관리하는 전역 싱글톤 데이터베이스
 */
class FBlueprintActionDatabase
{
public:
    /** @brief 싱글톤 접근 함수 */ 
    static FBlueprintActionDatabase& GetInstance()
    {
        static FBlueprintActionDatabase Instance;
        return Instance;
    }

    /** @brief ActionRegistry에 속한 원소들의 소유권은 FBlueprintActionDatabase에 있다고 가정한다. */
    ~FBlueprintActionDatabase();

    /** @brief 프로그램 시작 시 모든 UK2Node 클래스를 순회하며 데이터베이스를 초기화한다. */
    void Initialize();

    /**
     * @brief Registrar가 수집한 블루프린트 액션을 데이터베이스에 등록한다.
     * @note rvalue 참조를 통해서 안전하게 소유권을 이전하도록 한다.
     */
    void RegisterAllNodeActions(FBlueprintActionDatabaseRegistrar&& Registrar);

    /** @brief 데이터베이스에 등록된 블루프린트 액션의 목록 */
    TArray<UBlueprintNodeSpawner*> ActionRegistry;

private:
    FBlueprintActionDatabase() = default;
};