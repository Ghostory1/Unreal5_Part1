// Fill out your copyright notice in the Description page of Project Settings.


#include "GameData/ABGameSingleton.h"

DEFINE_LOG_CATEGORY(LogABGameSingleton);

UABGameSingleton::UABGameSingleton()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTableRef(TEXT("/ Script / Engine.DataTable'/Game/ArenaBattle/GameData/ABCharacterStatTable.ABCharacterStatTable'"));
	if (nullptr != DataTableRef.Object)
	{
		const UDataTable* DataTable = DataTableRef.Object;
		check(DataTable->GetRowMap().Num() > 0);

		//��ü Ű,����� ��������
		TArray<uint8*> ValueArray;
		DataTable->GetRowMap().GenerateValueArray(ValueArray);
		//�˰��� Transform �Լ��� ���� Value���� �����ͼ� �����ϴ� ���� �߰�
		Algo::Transform(ValueArray, CharacterStatTable,
			[](uint8* Value)
			{
				return *reinterpret_cast<FABCharacterStat*>(Value);
			}
		);
	}

	CharacterMaxLevel = CharacterStatTable.Num();
	ensure(CharacterMaxLevel > 0);
}

UABGameSingleton& UABGameSingleton::Get()
{
	//�����ϰ� Checked 
	UABGameSingleton* Singleton = CastChecked<UABGameSingleton>(GEngine->GameSingleton);
	if (Singleton)
	{
		return *Singleton;
	}

	//������ ���� �α�
	UE_LOG(LogABGameSingleton, Error, TEXT("Invalid Game Singleton"));
	//������ ���� �׳� �ڵ��� �帧�� ����ϱ����� ������Ʈ �ƹ��ų� �����ؼ� ����
	return *NewObject<UABGameSingleton>();
}
