// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "Student.h"
#include "JsonObjectConverter.h"
#include "UObject/SavePackage.h"

const FString UMyGameInstance::PackageName = TEXT("/Game/Student");
const FString UMyGameInstance::AssetName = TEXT("TopStudent");

void PrintStudentInfo(const UStudent* InStudent, const FString& InTag)
{
	UE_LOG(LogTemp, Log, TEXT("[%s] 이름 %s 순번 %d"), *InTag, *InStudent->GetName(), InStudent->GetOrder());
}
UMyGameInstance::UMyGameInstance()
{
	//생성자에서 애셋을 로딩할 경우 게임이 시작하기전에 미리 메모리에 다 올라와 있어야한다.
	const FString TopSoftObjectPath = FString::Printf(TEXT("%s.%s"), *PackageName, *AssetName);
	static ConstructorHelpers::FObjectFinder<UStudent> UASSET_TopStudent(*TopSoftObjectPath);
	if (UASSET_TopStudent.Succeeded())
	{
		PrintStudentInfo(UASSET_TopStudent.Object, TEXT("Constructor Asset"));
	}
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

	//패키지를 사용한 에셋 저장
	SaveStudentPackage();
	//LoadStudentPackage();
	//LoadStudentObject();

	//비동기 방식
	
	//애셋의 오브젝트 경로
	const FString TopSoftObjectPath = FString::Printf(TEXT("%s.%s"), *PackageName, *AssetName);
	Handle = StreamableManager.RequestAsyncLoad(TopSoftObjectPath,
		[&]() // [&] : 핸들을 참조해야되기때문에 레퍼런스를 가져옴
		{
			if (Handle.IsValid() && Handle->HasLoadCompleted())
			{
				UStudent* TopStudent = Cast<UStudent>(Handle->GetLoadedAsset());
				if (TopStudent)
				{
					PrintStudentInfo(TopStudent, TEXT("AsyncLoad"));

					//다 쓴 핸들 닫아주기
					Handle->ReleaseHandle();
					Handle.Reset();
				}
			}
		}
	);
}

void UMyGameInstance::SaveStudentPackage() const
{
	//이미 패키지가 존재한다면 다 로딩하고 저장해주는게 좋다.
	UPackage* StudentPackage = ::LoadPackage(nullptr, *PackageName, LOAD_None);
	if (StudentPackage)
	{
		StudentPackage->FullyLoad();
	}

	//패키지를 사용하기위하여 패키지와 패키지를 담고있는 대표 에셋을 지정
	
	//1. 패키지 생성코드
	StudentPackage = CreatePackage(*PackageName);
	//2. 패키지 저장 옵션
	EObjectFlags ObjectFlag = RF_Public | RF_Standalone;

	//3.패키지에 어떤 내용을 담을지 
	UStudent* TopStudent = NewObject<UStudent>(StudentPackage, UStudent::StaticClass(),*AssetName, ObjectFlag);
	TopStudent->SetName(TEXT("이승한"));
	TopStudent->SetOrder(35);

	const int32 NumfSubs = 10;
	for (int32 ix = 1; ix <= NumfSubs; ++ix)
	{
		FString SubObjectName = FString::Printf(TEXT("Student%d"), ix);
		UStudent* SubStudent = NewObject<UStudent>(TopStudent, UStudent::StaticClass(), *SubObjectName, ObjectFlag);
		SubStudent->SetName(FString::Printf(TEXT("학생%d"), ix));
		SubStudent->SetOrder(ix);
	}

	//4. 패키지에 저장될 경로를 지정
	//5. 패키지에 확장자 부여
	//FPackageName::GetAssetPackageExtension() : 언리얼 엔진에서 지정한 uasset이라는 확장자를 의미
	//우리가 앞서 지정한 PackageName ("/Game/Student") 를 바탕으로 현재 프로젝트의 Content폴더(현재는 Game)를 지정하게 하고 그 다음에 패키지의 Student가 파일 이름이 된다.
	const FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());

	//6. 저장
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = ObjectFlag;

	if (UPackage::SavePackage(StudentPackage, nullptr, *PackageFileName, SaveArgs))
	{
		UE_LOG(LogTemp, Log, TEXT("패키지가 성공적으로 저장되었습니다."));
	}
	

}

void UMyGameInstance::LoadStudentPackage() const
{
	UPackage* StudentPackage = ::LoadPackage(nullptr, *PackageName, LOAD_None);
	if (nullptr == StudentPackage)
	{
		UE_LOG(LogTemp, Warning, TEXT("패키지를 찾을 수 없습니다."));
		return;
	}

	StudentPackage->FullyLoad();

	//로딩된 패키지에서 AssetName을 찾아줘서 TopStudent에 저장
	UStudent* TopStudent = FindObject<UStudent>(StudentPackage, *AssetName);
	PrintStudentInfo(TopStudent, TEXT("FindObject Asset"));
}

void UMyGameInstance::LoadStudentObject() const
{
	//애셋의 오브젝트 경로
	const FString TopSoftObjectPath = FString::Printf(TEXT("%s.%s"), *PackageName, *AssetName);

	//이때 패키지를 로딩하지않기에 nullptr 지정
	UStudent* TopStudent = LoadObject<UStudent>(nullptr, *TopSoftObjectPath);
	PrintStudentInfo(TopStudent, TEXT("LoadObject Asset"));
}
