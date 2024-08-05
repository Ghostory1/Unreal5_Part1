// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/ABAIController.h"
//������ ���õ� ��� 3�� �߰�
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/ABAI.h"

AABAIController::AABAIController()
{
	//�׻� ����Ұ��̱� ������ ( �� ���� ���ӽ����Ű�� ���� �����ؼ� ����ϴϱ�) ConstruectorHelpers ��
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
	//������ ��� �߰�
	//�����带 �������� Ptr�� ��ȯ�Ǵ� ���� �߰�
	UBlackboardComponent* BlackboardPtr = Blackboard.Get();
	if (UseBlackboard(BBAsset, BlackboardPtr))
	{
		Blackboard->SetValueAsVector(BBKEY_HOMEPOS, GetPawn()->GetActorLocation());
		//������Ʈ�� ���� 
		bool RunResult = RunBehaviorTree(BTAsset);
		ensure(RunResult);
	}
}

void AABAIController::StopAI()
{
	//1.�����ϰ��ִ� �ൿƮ�� ������Ʈ�� �����´�.
	//���� ���������ʾƵ� �̹� UBehaviorTree �ȿ� BrainComponent�� ����ȴ�.
	UBehaviorTreeComponent* BTComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (BTComponent)
	{
		BTComponent->StopTree();
	}
}

void AABAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	//���� ����������
	RunAI();
}
