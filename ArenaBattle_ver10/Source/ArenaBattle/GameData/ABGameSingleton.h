// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ABCharacterStat.h"
#include "ABGameSingleton.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogABGameSingleton, Error, All);
/**
 * 
 */
UCLASS()
class ARENABATTLE_API UABGameSingleton : public UObject
{
	GENERATED_BODY()
	
public:
	UABGameSingleton();
	static UABGameSingleton& Get();

// Character Stat Data Section
public:
	//캐릭터 스탯을 가져올떄 Level 정보 키값으로 다른 값을 같이 가져온다 . 없을경우 기본 생성자를 생성해서 반환해준다.
	FORCEINLINE FABCharacterStat GetCharacterStat(int32 InLevel) const { return CharacterStatTable.IsValidIndex(InLevel - 1) ? CharacterStatTable[InLevel - 1] : FABCharacterStat(); }

	//몇개의 레벨이 있는지에 대한 정보 가져오기
	UPROPERTY()
	int32 CharacterMaxLevel;
private:
	TArray<FABCharacterStat> CharacterStatTable;
};
