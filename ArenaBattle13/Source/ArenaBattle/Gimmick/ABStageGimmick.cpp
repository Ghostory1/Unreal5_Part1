// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/ABStageGimmick.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Physics/ABCollision.h"
#include "Character/ABCharacterNonPlayer.h"
#include "Item/ABItemBox.h"

void AABStageGimmick::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    SetState(CurrentState);
}

// Sets default values
AABStageGimmick::AABStageGimmick()
{
 	//Stage Section
    Stage = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Stage"));
    RootComponent = Stage;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> StageMeshRef(TEXT("/Script/Engine.StaticMesh'/Game/ArenaBattle/Environment/Stages/SM_SQUARE.SM_SQUARE'"));
    if (StageMeshRef.Object)
    {
        Stage->SetStaticMesh(StageMeshRef.Object);
    }

    StageTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("StageTrigger"));
    StageTrigger->SetBoxExtent(FVector(775.0f, 775.0f, 300.0f));
    StageTrigger->SetupAttachment(Stage);
    StageTrigger->SetRelativeLocation(FVector(0.0f, 0.0f, 250.0f));
    StageTrigger->SetCollisionProfileName(CPROILE_ABTRIGGER); //���� �����Ҽ��ִ� Ư���� abtrigger ����
    StageTrigger->OnComponentBeginOverlap.AddDynamic(this, &AABStageGimmick::OnStageTriggerBeginOverlap);

    //Gate Section
    static FName GateSockets[] = { TEXT("+XGate"), TEXT("-XGate"), TEXT("+YGate"), TEXT("-YGate") };
    static ConstructorHelpers::FObjectFinder<UStaticMesh> GateMeshRef(TEXT("/ Script / Engine.StaticMesh'/Game/ArenaBattle/Environment/Props/SM_GATE.SM_GATE'"));
    for (FName GateSocket : GateSockets)
    {
        UStaticMeshComponent* Gate = CreateDefaultSubobject<UStaticMeshComponent>(GateSocket);
        Gate->SetStaticMesh(GateMeshRef.Object);
        Gate->SetupAttachment(Stage, GateSocket);
        Gate->SetRelativeLocation(FVector(0.0f, -80.5f, 0.0f));
        Gate->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
        Gates.Add(GateSocket, Gate);

        //������ ��Ʈ������ ��ȯ�� �Ŀ� Ʈ���Ŷ���ϴ� ���̻縦 �򰥸����ʰ� �ٿ�����
        FName TriggerName = *GateSocket.ToString().Append(TEXT("Trigger")); 
        UBoxComponent* GateTrigger = CreateDefaultSubobject<UBoxComponent>(TriggerName);
        GateTrigger->SetBoxExtent(FVector(100.0f, 100.0f, 300.0f));
        GateTrigger->SetupAttachment(Stage, GateSocket);
        GateTrigger->SetRelativeLocation(FVector(70.0f, 0.0f, 250.0f));
        GateTrigger->SetCollisionProfileName(CPROILE_ABTRIGGER);
        GateTrigger->OnComponentBeginOverlap.AddDynamic(this, &AABStageGimmick::OnGateTriggerBeginOverlap);
        GateTrigger->ComponentTags.Add(GateSocket);

        GateTriggers.Add(GateTrigger);
    }

    //State Section
    CurrentState = EStageState::READY;
    StateChangeActions.Add(EStageState::READY, FStageChangedDelegateWrapper(FOnStageChangedDelegate::CreateUObject(this, &AABStageGimmick::SetReady)));
    StateChangeActions.Add(EStageState::FIGHT, FStageChangedDelegateWrapper(FOnStageChangedDelegate::CreateUObject(this, &AABStageGimmick::SetFight)));
    StateChangeActions.Add(EStageState::REWARD, FStageChangedDelegateWrapper(FOnStageChangedDelegate::CreateUObject(this, &AABStageGimmick::SetChooseReward)));
    StateChangeActions.Add(EStageState::NEXT, FStageChangedDelegateWrapper(FOnStageChangedDelegate::CreateUObject(this, &AABStageGimmick::SetChooseNext)));

    // Fight Section
    OpponentSpawnTime = 2.0f;
    OpponentClass = AABCharacterNonPlayer::StaticClass();

    //Reward Section
    RewardBoxClass = AABItemBox::StaticClass();
    for (FName GateSocket : GateSockets)
    {
        FVector BoxLocation = Stage->GetSocketLocation(GateSocket) / 2;
        RewardBoxLocations.Add(GateSocket, BoxLocation);
    }

    //Stage Stat
    CurrentStageNum = 0;

}

void AABStageGimmick::OnStageTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
    SetState(EStageState::FIGHT);
}

void AABStageGimmick::OnGateTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
    //OverlappedComponent �� �츮�� Gate�� �±������� �ϳ� �������µ� �װ��� check�� �˻�
    check(OverlappedComponent->ComponentTags.Num() == 1);
    //�׷��� �� �±� �������� �����ʿ��� �������� 2���� ©�󳻸� +X,+Y,-X,-Y��� �̸� ����
    FName ComponentTag = OverlappedComponent->ComponentTags[0];
    FName SocketName = FName(*ComponentTag.ToString().Left(2));
    //�׷��� +X,+Y,-X,-Y�� ������ �ִ��� �˻� 
    check(Stage->DoesSocketExist(SocketName));

    FVector NewLocation = Stage->GetSocketLocation(SocketName);
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams CollisionQueryParam(SCENE_QUERY_STAT(GateTrigger), false, this);
    //OverlapMultiByObjectType �� �ش� ��ġ�� ���� �ִ��� Ȯ���ϴ°�
    //�׷��� MakeSphere (775)�� �ش� ��ġ�� ū ���� �������ְ� ��� ����ƽ ������Ʈ Ÿ�Կ��� 
    bool bResult = GetWorld()->OverlapMultiByObjectType(
        OverlapResults,
        NewLocation,
        FQuat::Identity,
        FCollisionObjectQueryParams::InitType::AllStaticObjects,
        FCollisionShape::MakeSphere(775.0f),
        CollisionQueryParam
    );

    //�ƹ��͵� ���ٴ°��� �����Ǹ� �ش� ��ġ�� �Ȱ��� ��� ���� ���� (���� ��)
    if (!bResult)
    {
        FTransform NewTransform(NewLocation);
        AABStageGimmick* NewGimmick = GetWorld()->SpawnActorDeferred<AABStageGimmick>(AABStageGimmick::StaticClass(), NewTransform);
        if (NewGimmick)
        {
            NewGimmick->SetStageNum(CurrentStageNum + 1);
            NewGimmick->FinishSpawning(NewTransform);
        }
    }
}

void AABStageGimmick::OpenAllGates()
{
    for (auto Gate : Gates)
    {
        (Gate.Value)->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }
}

void AABStageGimmick::CloseAllGates()
{
    for (auto Gate : Gates)
    {
        (Gate.Value)->SetRelativeRotation(FRotator::ZeroRotator);
    }
}

void AABStageGimmick::SetState(EStageState InNewState)
{
    CurrentState = InNewState;

    if(StateChangeActions.Contains(InNewState))
    {
        StateChangeActions[CurrentState].StageDelegate.ExecuteIfBound();
    }
}

void AABStageGimmick::SetReady()
{
    StageTrigger->SetCollisionProfileName(CPROILE_ABTRIGGER);
    for (auto GateTrigger : GateTriggers)
    {
        GateTrigger->SetCollisionProfileName(TEXT("NoCollision"));
    }
    OpenAllGates();
}

void AABStageGimmick::SetFight()
{
    StageTrigger->SetCollisionProfileName(TEXT("NoCollision"));
    for (auto GateTrigger : GateTriggers)
    {
        GateTrigger->SetCollisionProfileName(TEXT("NoCollision"));
    }
    CloseAllGates();

    GetWorld()->GetTimerManager().SetTimer(OpponentTimerHandle, this, &AABStageGimmick::OnOpponentSpawn, OpponentSpawnTime, false);
}

void AABStageGimmick::SetChooseReward()
{
    StageTrigger->SetCollisionProfileName(TEXT("NoCollision"));
    for (auto GateTrigger : GateTriggers)
    {
        GateTrigger->SetCollisionProfileName(TEXT("NoCollision"));
    }
    CloseAllGates();
    SpawnRewardBoxes();
}

void AABStageGimmick::SetChooseNext()
{
    StageTrigger->SetCollisionProfileName(TEXT("NoCollision"));
    for (auto GateTrigger : GateTriggers)
    {
        GateTrigger->SetCollisionProfileName(CPROILE_ABTRIGGER);
    }
    OpenAllGates();
}

void AABStageGimmick::OnOpponentDestroyed(AActor* DestroyedActor)
{
    SetState(EStageState::REWARD);
}

void AABStageGimmick::OnOpponentSpawn()
{
    //1. ������ġ �� ���ϱ�
    const FTransform SpawnTransform(GetActorLocation() + FVector::UpVector * 88.0f);
    AABCharacterNonPlayer* ABOpponentCharacter = GetWorld()->SpawnActorDeferred<AABCharacterNonPlayer>(OpponentClass, SpawnTransform);
    if (ABOpponentCharacter)
    {
        ABOpponentCharacter->OnDestroyed.AddDynamic(this, &AABStageGimmick::OnOpponentDestroyed);
        ABOpponentCharacter->SetLevel(CurrentStageNum);
        //FinsihSpawing �Լ��� ȣ��ǰ� ������ ĳ������ �˸��� BeginPlay �Լ��� ����Ǽ� �˸��� ������ �����
        // ���⼭�� ���� ABCharacterStatComponent�� BeginPlay�� ����ȴ�.
        ABOpponentCharacter->FinishSpawning(SpawnTransform);
    }
}

void AABStageGimmick::OnRewardTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
    for (const auto& RewardBox : RewardBoxes)
    {
        if (RewardBox.IsValid())
        {
            AABItemBox* ValidItemBox = RewardBox.Get();
            AActor* OverlappedBox = OverlappedComponent->GetOwner();
            if (OverlappedBox != ValidItemBox)
            {
                ValidItemBox->Destroy();
            }
        }
    }

    SetState(EStageState::NEXT);
}

void AABStageGimmick::SpawnRewardBoxes()
{
    for (const auto& RewardBoxLocation : RewardBoxLocations)
    {
        FTransform SpawnTransform(GetActorLocation() + RewardBoxLocation.Value + FVector(0.0f, 0.0f, 30.0f));
        AABItemBox* RewardBoxActor = GetWorld()->SpawnActorDeferred<AABItemBox>(RewardBoxClass, SpawnTransform);
        
        if (RewardBoxActor)
        {
            RewardBoxActor->Tags.Add(RewardBoxLocation.Key);
            RewardBoxActor->GetTrigger()->OnComponentBeginOverlap.AddDynamic(this, &AABStageGimmick::OnRewardTriggerBeginOverlap);
            RewardBoxes.Add(RewardBoxActor);
        }
    }

    for (const auto& RewardBox : RewardBoxes)
    {
        if (RewardBox.IsValid())
        {
            RewardBox.Get()->FinishSpawning(RewardBox.Get()->GetActorTransform());
        }
    }
}

