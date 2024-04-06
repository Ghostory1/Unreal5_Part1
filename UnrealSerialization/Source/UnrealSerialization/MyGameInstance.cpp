// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "Student.h"
#include "JsonObjectConverter.h"

void PrintStudentInfo(const UStudent* InStudent, const FString& InTag)
{
	UE_LOG(LogTemp, Log, TEXT("[%s] 이름 %s 순번 %d"), *InTag, *InStudent->GetName(), InStudent->GetOrder());
}
UMyGameInstance::UMyGameInstance()
{

}

void UMyGameInstance::Init()
{
	Super::Init();

	FStudentData RawDataSrc(16, TEXT("테스트 이름"));

	// 우리가 생성한 언리얼프로젝트 파일안에 Saved 폴더의 경로를 얻어온다.
	const FString SaveDir = FPaths::Combine(FPlatformMisc::ProjectDir(), TEXT("Saved"));
	UE_LOG(LogTemp, Log, TEXT("저장할 파일 폴더 : %s"), *SaveDir);

	{
		//우리가 저장할 파일의 이름
		const FString RawDataFileName(TEXT("RawData.bin"));

		//이 경우 절대경로값을 가져오지만 이상한 값으로 보인다.
		FString RawDataAbsolutePath = FPaths::Combine(*SaveDir, *RawDataFileName);
		UE_LOG(LogTemp, Log, TEXT("저장할 파일의 전체 경로 : %s"), *RawDataAbsolutePath);

		//전체 경로 다듬기
		FPaths::MakeStandardFilename(RawDataAbsolutePath);
		UE_LOG(LogTemp, Log, TEXT("변경할 파일의 전체 경로 : %s"), *RawDataAbsolutePath);

		FArchive* RawFileWriterAr = IFileManager::Get().CreateFileWriter(*RawDataAbsolutePath);
		if (nullptr != RawFileWriterAr)
		{
			*RawFileWriterAr << RawDataSrc;
			RawFileWriterAr->Close();
			delete RawFileWriterAr;
			RawFileWriterAr = nullptr;
		}

		//이제 데이터를 읽어와보자
		FStudentData RawDataDest;
		FArchive* RawFileReaderAr = IFileManager::Get().CreateFileReader(*RawDataAbsolutePath);
		if (nullptr != RawFileReaderAr)
		{
			*RawFileReaderAr << RawDataDest;
			RawFileReaderAr->Close();
			delete RawFileReaderAr;
			RawFileReaderAr = nullptr;
		}

		UE_LOG(LogTemp, Log, TEXT("[RawData] 이름 %s 순번 %d"), *RawDataDest.Name, RawDataDest.Order);
	}

	StudentSrc = NewObject<UStudent>();
	StudentSrc->SetName(TEXT("이승한"));
	StudentSrc->SetOrder(60);

	{
		const FString ObjectDataFileName(TEXT("ObjectData.bin"));
		FString ObjectDataAbsolutePath = FPaths::Combine(*SaveDir, *ObjectDataFileName);
		FPaths::MakeStandardFilename(ObjectDataAbsolutePath);

		//메모리에 언리얼 오브젝트의 내용 저장
		//바이트 스트림은 TArray<uint8>로 지정 ( 직렬화를 위한 버퍼)
		TArray<uint8> BufferArray;
	    //버퍼와 연동되는 메모리Writer
		FMemoryWriter MemoryWriterAr(BufferArray);
		//이렇게 하면 메모리의 BufferArray에는 Student오브젝트의 내용이 저장
		StudentSrc->Serialize(MemoryWriterAr);
		
		//이것을 파일로 저장 (아까 구조체에서는 delete , nullptr 을 지정해줫는데 이번에는 스마트 포인터 사용)
		if (TUniquePtr<FArchive> FileWriterAr = TUniquePtr<FArchive>(IFileManager::Get().CreateFileWriter(*ObjectDataAbsolutePath)))
		{
			*FileWriterAr << BufferArray;
			FileWriterAr->Close();
		}

		//파일 읽기
		TArray<uint8> BufferArrayFromFile;
		if (TUniquePtr<FArchive> FileReaderAr = TUniquePtr<FArchive>(IFileManager::Get().CreateFileReader(*ObjectDataAbsolutePath)))
		{
			*FileReaderAr << BufferArrayFromFile;
			FileReaderAr->Close();
		}

		//다시 메모리에 전송
		FMemoryReader MemoeryReaderAr(BufferArrayFromFile);
		UStudent* StudentDest = NewObject<UStudent>();
		StudentDest->Serialize(MemoeryReaderAr);

		PrintStudentInfo(StudentDest, TEXT("ObjectData"));

		
	}

	{
		const FString JsonDataFileName(TEXT("StudentJsonData.text"));
		FString JsonDataAbsolutePath = FPaths::Combine(*SaveDir, *JsonDataFileName);
		FPaths::CreateStandardFilename(JsonDataAbsolutePath);

		//언리얼 오브젝트 -> Json 오브젝트로 바꿔줘야한다. ( 헤더 선언 필요 : JsonObjectConverter.h
		
		//레퍼런스를 편하게 사용할 수 있게 공유 레퍼런스 설정
		//MakeShared 하는 이유 : 공유 레퍼런스는 NotNull을 보장해야하기에 객체를 만들어줌
		TSharedRef<FJsonObject> JsonObjectSrc = MakeShared<FJsonObject>();
		FJsonObjectConverter::UStructToJsonObject(StudentSrc->GetClass(), StudentSrc, JsonObjectSrc);

		FString JsonOutString;
		TSharedRef<TJsonWriter<TCHAR>> JsonWriterAr = TJsonWriterFactory<TCHAR>::Create(&JsonOutString);
		if (FJsonSerializer::Serialize(JsonObjectSrc, JsonWriterAr))
		{
			FFileHelper::SaveStringToFile(JsonOutString, *JsonDataAbsolutePath);
		}

		//다시 파일을 불러들이는 작업
		FString JsonInString;
		FFileHelper::LoadFileToString(JsonInString, *JsonDataAbsolutePath);

		TSharedRef<TJsonReader<TCHAR>> JsonReaderAr = TJsonReaderFactory<TCHAR>::Create(JsonInString);

		//여기서 문자열에 이상한 값이 들어와서 오브젝트가 안만들어질수도 있다. 널이 들어갈수있는 Ptr 포인터로 선언
		TSharedPtr<FJsonObject> JsonObjectDest;
		if (FJsonSerializer::Deserialize(JsonReaderAr, JsonObjectDest))
		{
			UStudent* JsonStudentDest = NewObject<UStudent>();
			if (FJsonObjectConverter::JsonObjectToUStruct(JsonObjectDest.ToSharedRef(), JsonStudentDest->GetClass(), JsonStudentDest))
			{
				PrintStudentInfo(JsonStudentDest, TEXT("JsonData"));
			}
		}
	}
}
