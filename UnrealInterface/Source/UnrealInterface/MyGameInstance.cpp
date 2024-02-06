// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "Student.h"
#include "Teacher.h"
#include "Staff.h"

UMyGameInstance::UMyGameInstance()
{
	SchoolName = TEXT("기본학교");
}

void UMyGameInstance::Init()
{
	Super::Init();


	UE_LOG(LogTemp, Log, TEXT("========================"));
	
	TArray<UPerson*> Persons = { NewObject<UStudent>(),NewObject<UTeacher>(),NewObject<UStaff>() };
	for (const auto Person : Persons)
	{
		UE_LOG(LogTemp, Log, TEXT("구성원 이름 : %s"), *Person->GetName());
	}
	UE_LOG(LogTemp, Log, TEXT("========================"));

	for (const auto Person : Persons)
	{
		//ILessonInterface를 상속받은 Person을 가려내는 방법으로 Cast 캐스팅 방법을 이용하여 LessonInterface가 true값으로 반환되면, 상속받은것.
		ILessonInterface* LessonInterface = Cast<ILessonInterface>(Person);
		if (LessonInterface)
		{
			UE_LOG(LogTemp, Log, TEXT("%s님이 수업에 참여 가능합니다."), *Person->GetName());
			LessonInterface->DoLesson();
		}
		else 
		{
			UE_LOG(LogTemp, Log, TEXT("%s님이 수업에 참여하지 못했습니다.."), *Person->GetName());
		}

	}
}

