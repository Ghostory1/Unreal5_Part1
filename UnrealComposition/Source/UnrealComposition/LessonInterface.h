// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LessonInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULessonInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class UNREALCOMPOSITION_API ILessonInterface
{
	GENERATED_BODY()

public:
	virtual void DoLesson()
	{
		//원래 가상함수에서 구현을 하면 안되지만 여기서는 가능하다. 근데 비워두는게 좋다.
		//이렇게 인터페이스에서 가상함수를 구현하는 경우에는 하위 상속받은 클래스에서 딱히 선언하지않아도 된다.
		UE_LOG(LogTemp, Log, TEXT("수업에 입장합니다."));
	}
};
