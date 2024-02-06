// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class UNREALINTERFACE_API UMyGameInstance : public UGameInstance
{

	GENERATED_BODY()

public:
	UMyGameInstance();
	virtual void Init() override;
private:
	//이렇게 리플렉션 시스템 (프로퍼티)를 사용해주면 지금 선언한 SchoolName에 대한 정보를 런타임, 컴파일타임에서 얻어 올 수 있다
	UPROPERTY()
	FString SchoolName;
};
