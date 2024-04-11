// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ABCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "ABCharacterControlData.h"
#include "GameFramework/CharacterMovementComponent.h"
// Sets default values
AABCharacterBase::AABCharacterBase()
{
    //���� ȸ���� ����
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    //ĸ��
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

    //Movement
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 700.0f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

    //Mesh
    // ĳ���͵��� �츮�� ������ �𵨸������� ������⿡ ���簡���� ����� ��ġ�� ȸ���� ������ ������Ѵ�.
    GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -100.0f), FRotator(0.0f, -90.0f, 0.0f));
    //�ִϸ��̼� ��带 ����
    GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
    GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));

    //������ ���̷�Ż �޽� ������Ʈ�� ���� ������ ����
    //���⼭�� ThirdPersonCharacter���� �������ִ� �޽��� Ŭ������ �̿�
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> CharacterMeshRef(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple'"));
    if (CharacterMeshRef.Object)
    {
        GetMesh()->SetSkeletalMesh(CharacterMeshRef.Object);
    }
    //�ִϸ��̼� ��� ( �ִϸ��̼��� Ŭ������ �����ͼ� ����)
    static ConstructorHelpers::FClassFinder<UAnimInstance> AnimInstanceClassRef(TEXT("/Game/Characters/Mannequins/Animations/ABP_Quinn.ABP_Quinn_C"));
    if (AnimInstanceClassRef.Class)  
    {
        GetMesh()->SetAnimInstanceClass(AnimInstanceClassRef.Class);
        
    }

    //TMap�� �߰�
    static ConstructorHelpers::FObjectFinder<UABCharacterControlData> ShoulderDataRef(TEXT("/Script/ArenaBattle.ABCharacterControlData'/Game/ArenaBattle/CharacterControl/ABC_Shoulder.ABC_Shoulder'"));
    if (ShoulderDataRef.Object)
    {
        CharacterControlManager.Add(ECharacterControlType::Shoulder, ShoulderDataRef.Object);
    }

    static ConstructorHelpers::FObjectFinder<UABCharacterControlData> QuaterDataRef(TEXT("/Script/ArenaBattle.ABCharacterControlData'/Game/ArenaBattle/CharacterControl/ABC_Quater.ABC_Quater'"));
    if (QuaterDataRef.Object)
    {
        CharacterControlManager.Add(ECharacterControlType::Quater, QuaterDataRef.Object);
    }
}

void AABCharacterBase::SetCharacterControlData(const UABCharacterControlData* CharacterControlData)
{
    //Pawn
    bUseControllerRotationYaw = CharacterControlData->bUseControllerRotationYaw;

    // CharacterMovement
    GetCharacterMovement()->bOrientRotationToMovement = CharacterControlData->bOrientRotationToMovement;
    GetCharacterMovement()->bUseControllerDesiredRotation = CharacterControlData->bUseControllerDesiredRotation;
    GetCharacterMovement()->RotationRate = CharacterControlData->RotationRate;
}



