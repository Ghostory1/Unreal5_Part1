// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/ABAIController.h"
//블랙보드 관련된 헤더 3개 추가
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/ABAI.h"

AABAIController::AABAIController()
{
	//항상 사용할것이기 때문에 ( 이 말은 게임실행시키면 옆에 생성해서 사용하니까) ConstruectorHelpers 로
	static ConstructorHelpers::FObjectFinder<UBlackboardData> BBAssetRef(TEXT("/Script/AIModule.BlackboardData'/Game/ArenaBattle/AI/BB_ABCharacter.BB_ABCharacter'"));
	if (nullptr != BBAssetRef.Object)
	{
		BBAsset = BBAssetRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTAssetRef(TEXT("/Script/AIModule.BehaviorTree'/Game/ArenaBattle/AI/BT_ABCharacter.BT_ABCharacter'"));
	if (nullptr != BTAssetRef.Object)
	{
		BTAsset = BTAssetRef.Object;
	}
}

void AABAIController::RunAI()
{
	//블랙보드 헤더 추가
	//블랙보드를 가져오고 Ptr로 반환되니 변수 추가
	UBlackboardComponent* BlackboardPtr = Blackboard.Get();
	if (UseBlackboard(BBAsset, BlackboardPtr))
	{
		Blackboard->SetValueAsVector(BBKEY_HOMEPOS, GetPawn()->GetActorLocation());
		//블랙보드트리 실행 
		bool RunResult = RunBehaviorTree(BTAsset);
		ensure(RunResult);
	}
}

void AABAIController::StopAI()
{
	//1.동작하고있는 행동트리 컴포넌트를 가져온다.
	//따로 선언하지않아도 이미 UBehaviorTree 안에 BrainComponent에 저장된다.
	UBehaviorTreeComponent* BTComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (BTComponent)
	{
		BTComponent->StopTree();
	}
}

void AABAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	//폰이 빙의했을때
	RunAI();
}
