// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ABPlayerController.h"
#include "UI/ABHUDWidget.h"

AABPlayerController::AABPlayerController()
{
	static ConstructorHelpers::FClassFinder<UABHUDWidget> ABHUDWidgetRef(TEXT("/Game/ArenaBattle/UI/WBP_ABHUD.WBP_ABHUD_C"));
	if (ABHUDWidgetRef.Class)
	{
		ABHUDWidgetClass = ABHUDWidgetRef.Class;
	}

}

void AABPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//���� ���۵��� �� ���콺Ŀ���� ���� ����Ʈ�� �̵�
	FInputModeGameOnly GameOnlyInputMode;
	SetInputMode(GameOnlyInputMode);

	//������ ���۵Ǹ� ���� ����
	ABHUDWidget = CreateWidget<UABHUDWidget>(this, ABHUDWidgetClass);
	if (ABHUDWidget)
	{
		//������ AddToViewport ��� �Լ��� �ʱ�ȭ�ؼ� ȭ�鿡 ���
		ABHUDWidget->AddToViewport();
	}
}
