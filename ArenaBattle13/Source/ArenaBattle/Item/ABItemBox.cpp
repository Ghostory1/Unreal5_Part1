// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ABItemBox.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Physics/ABCollision.h"
#include "Interface/ABCharacterItemInterface.h"
#include "ABItemData.h"
#include "Engine/AssetManager.h"

// Sets default values
AABItemBox::AABItemBox()
{
    Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Effect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Effect"));

    RootComponent = Trigger;
    Mesh->SetupAttachment(Trigger);
    Effect->SetupAttachment(Trigger);
    
    Trigger->SetCollisionProfileName(CPROILE_ABTRIGGER);
    Trigger->SetBoxExtent(FVector(40.04f, 42.0f, 30.0f));
    

    // Mesh Component
    static ConstructorHelpers::FObjectFinder<UStaticMesh> BoxMeshRef(TEXT("/Script/Engine.StaticMesh'/Game/ArenaBattle/Environment/Props/SM_Env_Breakables_Box1.SM_Env_Breakables_Box1'"));
    if (BoxMeshRef.Object)
    {
        Mesh->SetStaticMesh(BoxMeshRef.Object);
    }
    Mesh->SetRelativeLocation(FVector(0.0f, -3.5f, -30.0f));
    Mesh->SetCollisionProfileName(TEXT("NoCollision")); // 캐릭터는 상자를 그냥 보여주기만 하고 충돌이 일어나지 않도록 NoCollision 설정

    static ConstructorHelpers::FObjectFinder<UParticleSystem> EffectRef(TEXT("/Script/Engine.ParticleSystem'/Game/ArenaBattle/Effect/P_TreasureChest_Open_Mesh.P_TreasureChest_Open_Mesh'"));
    if (EffectRef.Object)
    {
        Effect->SetTemplate(EffectRef.Object);
        Effect->bAutoActivate = false;
    }
}

void AABItemBox::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    UAssetManager& Manager = UAssetManager::Get();

    TArray<FPrimaryAssetId> Assets;
    Manager.GetPrimaryAssetIdList(TEXT("ABItemData"), Assets);
    ensure(0 < Assets.Num());

    int32 RandomIndex = FMath::RandRange(0, Assets.Num() - 1);
    FSoftObjectPtr AssetPtr(Manager.GetPrimaryAssetPath(Assets[RandomIndex]));
    if (AssetPtr.IsPending())
    {
        AssetPtr.LoadSynchronous();
    }
    Item = Cast<UABItemData>(AssetPtr.Get());
    ensure(Item);

    Trigger->OnComponentBeginOverlap.AddDynamic(this, &AABItemBox::OnOverlapBegin);
}

void AABItemBox::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
    if (nullptr == Item)
    {
        //아이템이 꽝인 경우
        Destroy();
        return;
    }
    IABCharacterItemInterface* OverlappingPawn = Cast<IABCharacterItemInterface>(OtherActor);
    if (OverlappingPawn)
    {
        //부딪힌 액터가 해당 인터페이스를 구현했다면 TakeItem
        OverlappingPawn->TakeItem(Item);
    }
    Effect->Activate(true);
    Mesh->SetHiddenInGame(true);
    SetActorEnableCollision(false);
    Effect->OnSystemFinished.AddDynamic(this,&AABItemBox::OnEffectFinished);
}

void AABItemBox::OnEffectFinished(UParticleSystemComponent* ParticleSystem)
{
    Destroy();
}


