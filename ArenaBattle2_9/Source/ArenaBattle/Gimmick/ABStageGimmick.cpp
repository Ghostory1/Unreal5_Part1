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
    StageTrigger->SetCollisionProfileName(CPROILE_ABTRIGGER); //폰만 감지할수있는 특별한 abtrigger 설정
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

        //소켓을 스트링으로 변환한 후에 트리거라고하는 접미사를 헷갈리지않게 붙여줬음
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

}

void AABStageGimmick::OnStageTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
    SetState(EStageState::FIGHT);
}

void AABStageGimmick::OnGateTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
    //OverlappedComponent 에 우리가 Gate의 태그정보를 하나 가져오는데 그것을 check로 검사
    check(OverlappedComponent->ComponentTags.Num() == 1);
    //그래서 그 태그 정보에서 오른쪽에서 왼쪽으로 2개를 짤라내면 +X,+Y,-X,-Y라는 이름 생성
    FName ComponentTag = OverlappedComponent->ComponentTags[0];
    FName SocketName = FName(*ComponentTag.ToString().Left(2));
    //그래서 +X,+Y,-X,-Y의 소켓이 있는지 검사 
    check(Stage->DoesSocketExist(SocketName));

    FVector NewLocation = Stage->GetSocketLocation(SocketName);
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams CollisionQueryParam(SCENE_QUERY_STAT(GateTrigger), false, this);
    //OverlapMultiByObjectType 은 해당 위치에 무언가 있는지 확인하는것
    //그래서 MakeSphere (775)로 해당 위치에 큰 원을 생성해주고 모든 스태틱 오브젝트 타입에서 
    bool bResult = GetWorld()->OverlapMultiByObjectType(
        OverlapResults,
        NewLocation,
        FQuat::Identity,
        FCollisionObjectQueryParams::InitType::AllStaticObjects,
        FCollisionShape::MakeSphere(775.0f),
        CollisionQueryParam
    );

    //아무것도 없다는것이 판정되면 해당 위치에 똑같은 기믹 액터 스폰 (무한 맵)
    if (!bResult)
    {
        GetWorld()->SpawnActor<AABStageGimmick>(NewLocation, FRotator::ZeroRotator);
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
    //1. 스폰위치 값 정하기
    const FVector SpawnLocation = GetActorLocation() + FVector::UpVector * 88.0f;
    AActor* OpponentActor = GetWorld()->SpawnActor(OpponentClass, &SpawnLocation, &FRotator::ZeroRotator);
    AABCharacterNonPlayer* ABOpponentCharacter = Cast<AABCharacterNonPlayer>(OpponentActor);
    if (ABOpponentCharacter)
    {
        ABOpponentCharacter->OnDestroyed.AddDynamic(this, &AABStageGimmick::OnOpponentDestroyed);
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
        FVector WorldSpawnLocation = GetActorLocation() + RewardBoxLocation.Value + FVector(0.0f, 0.0f, 30.0f);
        AActor* ItemActor = GetWorld()->SpawnActor(RewardBoxClass, &WorldSpawnLocation, &FRotator::ZeroRotator);
        AABItemBox* RewardBoxActor = Cast<AABItemBox>(ItemActor);
        if (RewardBoxActor)
        {
            RewardBoxActor->Tags.Add(RewardBoxLocation.Key);
            RewardBoxActor->GetTrigger()->OnComponentBeginOverlap.AddDynamic(this, &AABStageGimmick::OnRewardTriggerBeginOverlap);
            RewardBoxes.Add(RewardBoxActor);
        }
    }
}


