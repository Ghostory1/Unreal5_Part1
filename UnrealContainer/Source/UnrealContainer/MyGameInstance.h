// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

USTRUCT()
struct FStudentData
{
	GENERATED_BODY()
	
	//생성자
	FStudentData()
	{
		Name = TEXT("홍길동");
		Order = 1;
	}
	//구조체 인자를 가진 생성자 자유롭게 사용가능
	//구조체는 언리얼 오브젝트가 아니기에 New API를 통해 생성될 일이 없기 때문에
	FStudentData(FString InName, int32 InOrder) : Name(InName), Order(InOrder) {}

	bool operator==(const FStudentData& InOther) const
	{
		return Order == InOther.Order;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FStudentData& InStudentData)
	{
		return GetTypeHash(InStudentData.Order);
	}

	UPROPERTY()
	FString Name;

	UPROPERTY()
	int32 Order;
};


/**
 * 
 */
UCLASS()
class UNREALCONTAINER_API UMyGameInstance : public UGameInstance
{

	GENERATED_BODY()
	
public:
	virtual void Init() override;

private:
	//이 경우는 값 타입이기 때문에 메모리를 관리할 필요가 없다.
	//리플렉션 기능으로 조회할 필요가 없다면 굳이 UPROPERTY를 붙일 필요 없다.
	//데이터를 관리할 목적으로만 선언 
	TArray<FStudentData> StudentsData;

	//Object를 상속받은 Student클래스
	//언리얼 헤더에서 언리얼 오브젝트 포인터를 선언할때는 TObjectPtr로 감싸줘야한다 -> 잊지말자
	//전방선언으로 헤더 의존성 최소화 
	//언리얼 오브젝트를 TArray로 다룰때 UPROPERTY() 필수 -> 언리얼이 메모리를 관리할수 있도록
	UPROPERTY()
	TArray<TObjectPtr<class UStudent>> Students;

	//만약 TMap안에 언리얼 오브젝트 포인터가 들어간다면 UPROPERTY를 선언해줘야한다.
	TMap<int32, FString> StudentsMap;
};
