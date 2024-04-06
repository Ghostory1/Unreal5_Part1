// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "Algo/Accumulate.h"


FString MakeRandomName()
{
	TCHAR FirstChar[] = TEXT("김이박최");
	TCHAR MiddleChar[] = TEXT("상혜지성");
	TCHAR LastChar[] = TEXT("수은원연");

	//TArray는 TCHAR배열을 포함하고 있는 컨테이너
	//이 포인터 값을 넘겨주면 반환 값을 FString으로 지정했기에 자동으로 FString이 만들어져서 반환하게 된다.
	TArray<TCHAR> RandArray;
	//빈공간 3개 확보
	RandArray.SetNum(3);
	RandArray[0] = FirstChar[FMath::RandRange(0, 3)];
	RandArray[1] = MiddleChar[FMath::RandRange(0, 3)];
	RandArray[2] = LastChar[FMath::RandRange(0, 3)];

	return RandArray.GetData();
}
void UMyGameInstance::Init()
{
	Super::Init();

	const int32 StudentNum = 300;
	for (int32 ix = 1; ix <= StudentNum; ++ix)
	{
		StudentsData.Emplace(FStudentData(MakeRandomName(), ix));
	}
	//이름정보만 모아둠, StudentsData들을 옮겨보자
	TArray<FString> AllStudentsNames;
	Algo::Transform(StudentsData, AllStudentsNames,
		[](const FStudentData& Val)
		{
			return Val.Name;
		}
	);

	UE_LOG(LogTemp, Log, TEXT("모든 학생 이름의 수 : %d"), AllStudentsNames.Num());


	//중복을 허용 x
	TSet<FString> AllUniqueNames;
	Algo::Transform(StudentsData, AllUniqueNames,
		[](const FStudentData& Val)
		{
			return Val.Name;
		}
	);

	UE_LOG(LogTemp, Log, TEXT("중복 없는 학생의 수 : %d"), AllUniqueNames.Num());

	Algo::Transform(StudentsData, StudentsMap,
		[](const FStudentData& Val)
		{
			return TPair<int32, FString>(Val.Order, Val.Name);
		}
	);

	UE_LOG(LogTemp, Log, TEXT("순번에 따른 학생 맵의 레코드 수 : %d"), StudentsMap.Num());

	TMap<FString, int32> StudentsMapByUnique;

	Algo::Transform(StudentsData, StudentsMapByUnique,
		[](const FStudentData& Val)
		{
			return TPair<FString, int32>(Val.Name, Val.Order);
		}
	);
	UE_LOG(LogTemp, Log, TEXT("이름에 따른 학생 맵의 레코드 수 : %d"), StudentsMapByUnique.Num());

	TMultiMap<FString, int32> StudentMapByName;
	Algo::Transform(StudentsData, StudentMapByName,
		[](const FStudentData& Val)
		{
			return TPair<FString, int32>(Val.Name, Val.Order);
		}
	);

	UE_LOG(LogTemp, Log, TEXT("이름에 따른 학생 멀티맵의 레코드 수 : %d"), StudentMapByName.Num());
	
	const FString TargetName(TEXT("이혜은"));
	TArray<int32> AllOrders;
	StudentMapByName.MultiFind(TargetName, AllOrders);

	UE_LOG(LogTemp, Log, TEXT("이름이 %s인 학생 수 : %d"), *TargetName,AllOrders.Num());

	TSet<FStudentData> StudentsSet;
	for (int32 ix = 1; ix <= StudentNum; ++ix)
	{
		StudentsSet.Emplace(FStudentData(MakeRandomName(), ix));
	}
}
