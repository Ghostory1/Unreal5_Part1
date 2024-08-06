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

	//게임 시작됐을 때 마우스커서가 게임 뷰포트로 이동
	FInputModeGameOnly GameOnlyInputMode;
	SetInputMode(GameOnlyInputMode);

	//게임이 시작되면 위젯 생성
	ABHUDWidget = CreateWidget<UABHUDWidget>(this, ABHUDWidgetClass);
	if (ABHUDWidget)
	{
		//위젯을 AddToViewport 라는 함수로 초기화해서 화면에 띄움
		ABHUDWidget->AddToViewport();
	}
}
