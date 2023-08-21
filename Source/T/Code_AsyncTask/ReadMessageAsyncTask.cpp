#include "ReadMessageAsyncTask.h"
#include "Kismet/KismetSystemLibrary.h"

#include "../Code_Public/Tools/JsonObjectConverterEx.h"
#include "../Code_Public/GameInstance/ClientGameInstance.h"
#include "../Code_Public/Struct/PublicStruct.h"
#include "../Code_Public/MyGlobal.h"

DEFINE_LOG_CATEGORY(ReadMessageTaskLog);

FReadMessageAsyncTask::~FReadMessageAsyncTask() {
	
}

void FReadMessageAsyncTask::DisplayText(const FString& text) {
	UClientGameInstance* T = Cast<UClientGameInstance>(Owner);

	// find <sp> and split it
	FString Delimiter = TEXT("<sp>");

	TArray<FString> Substrings;
	text.ParseIntoArray(Substrings, *Delimiter, true);

	if (Substrings.Num() == 2)
	{
		FString FirstPart = Substrings[0];
		FString SecondPart = Substrings[1];

		T->MyText(FColor::Orange, TEXT("----------"));
		T->MyText(FColor(173, 216, 230), FirstPart);  // LightBlue
		T->MyText(FColor::Orange, TEXT("----------"));
		T->MyText(FColor::Orange, SecondPart);
	}
	else
	{
		T->MyText(FColor::Orange, text);
	}
}

void FReadMessageAsyncTask::DoCommand(const FString& cmd) {
	UClientGameInstance* T = Cast<UClientGameInstance>(Owner);

	if (cmd == TEXT("現在進入正常模式")) {
		MyGlobal::SaveDebugLevel(0);
	}
	else if (cmd == TEXT("現在進入字幕模式")) {
		MyGlobal::SaveDebugLevel(1);
	}
	else if (cmd == TEXT("現在進入除錯模式")) {
		MyGlobal::SaveDebugLevel(2);
	}
}

void FReadMessageAsyncTask::DoWork() {
	UClientGameInstance* T = Cast<UClientGameInstance>(Owner);
	while (true) {
		FPlatformProcess::Sleep(0.5);
		if (!T->IsValidLowLevel()) {
			break;
		}

		if (Socket == nullptr || T->StopSocket) {
			continue;
		}

		uint32 ReadSize;

		if (Socket->HasPendingData(ReadSize)) {
			TArray<uint8> TempBuff;

			TempBuff.Init(0, ReadSize);
			int32 NowSize = 0;
			uint8 StateIndex = 0xff;

			if (Socket->Recv((uint8*)TempBuff.GetData(), ReadSize, NowSize)) {
#if NO_PROTOBUF
				if (TempBuff[0] == 'c' && TempBuff[TempBuff.Num() - 1] == 'n') {
					AsyncTask(ENamedThreads::GameThread, [=] {
						T->OnSwitchPlayerMesh.Broadcast(0);
					});
					continue;
				}
				else if (TempBuff[0] == 'c' && TempBuff[TempBuff.Num() - 1] == 'i') {
					AsyncTask(ENamedThreads::GameThread, [=] {
						T->OnSwitchPlayerMesh.Broadcast(1);
					});
					continue;
				}
				else if (TempBuff[0] == 'c' && TempBuff[TempBuff.Num() - 1] == 'd') {
					AsyncTask(ENamedThreads::GameThread, [=] {
						T->OnSwitchPlayerMesh.Broadcast(2);
					});
					continue;
				}

				if (TempBuff.Num() >= 2) {
					if (TempBuff[TempBuff.Num() - 1] == 'b' && TempBuff[TempBuff.Num() - 2] == 's') {
						continue;
					}
				}

				if (TempBuff.Num() >= 14) {
					if (TempBuff[0] == 's' && TempBuff[TempBuff.Num() - 1] == 'd') {
						T->MyDebug(FColor::Red, TEXT("服务器返回允许说话"));
						AsyncTask(ENamedThreads::GameThread, [=] {
							T->OnWaitServerData.Broadcast();
						});
						continue;
					}
				}

				T->MyDebug(FColor::Red, FString(TEXT("暂时读取到数据量：")) + FString::FromInt(NowSize));

				if (TempBuff[TempBuff.Num() - 2] != '!' || TempBuff[TempBuff.Num() - 3] != '?') {

					ReadBuff += TempBuff;
					RecvSize += NowSize;
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
					T->MyDebug(FColor::Red, TEXT("等待更多数据中(没有发现终止符)"));
					AsyncTask(ENamedThreads::GameThread, [=] {
						int32 Index = FMath::RandRange(0, 9999);
						if (FFileHelper::SaveArrayToFile(ReadBuff, *(UTestFunctionLibrary::GetRootPath() + FString::FromInt(Index) + TEXT(".dat")))) {
							T->MyDebug(FColor::Red, TEXT("暂存位置:") + UTestFunctionLibrary::GetRootPath() + FString::FromInt(Index) + TEXT(".dat"));
						}
					});
#endif
					continue;
				}

				ReadBuff += TempBuff;
				RecvSize += NowSize;

				StateIndex = TempBuff[TempBuff.Num() - 1];

				ReadBuff.RemoveAt(ReadBuff.Num() - 3, 3);
				if (ReadBuff.Num() == 0) {
					continue;
				}

				uint16 slen = ReadBuff[ReadBuff.Num() - 2] * 256 + ReadBuff[ReadBuff.Num() - 1];
				if (slen > 0) {
					int32 StartIndex = ReadBuff.Num() - (slen + 2);
					FString s = FString::Printf(TEXT("%.*s"), slen / 2, &ReadBuff[StartIndex]);

					DoCommand(s);
					DisplayText(s);
				}
				ReadBuff.RemoveAt(ReadBuff.Num() - (slen + 2), slen + 2);

#pragma region 音频和表情处理部分
#if !PLATFORM_ANDROID
				FString RedirectFilePath = SaveFilePath;
				if (FPaths::FileExists(*SaveFilePath)) {
					if (!FPaths::DirectoryExists(UTestFunctionLibrary::GetRootPath() + TEXT("Temp/Temp"))) {
						FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*(UTestFunctionLibrary::GetRootPath() + TEXT("Temp/Temp")));
					}
					RedirectFilePath = UTestFunctionLibrary::GetRootPath() + TEXT("Temp/Temp/") + FString::FromInt(FMath::RandRange(0, 9999999)) + TEXT(".wav");

					FAttachWavFileInfo Info;
					Info.WavFilePath = RedirectFilePath;
					//Info.StateIndex = 9;
					Info.StateIndex = StateIndex;

					if (T->AttachWavFilePathList.Num() == 0) {
						T->AttachWavFilePathList.Add(Info);
					}

					T->AttachWavFilePathList.Add(Info);
					if (FFileHelper::SaveArrayToFile(ReadBuff, *RedirectFilePath)) {
						ReadBuff.Empty();
						RecvSize = 0;
					}
				}
				else {
					if (FFileHelper::SaveArrayToFile(ReadBuff, *RedirectFilePath)) {
						AsyncTask(ENamedThreads::GameThread, [=] {
							T->OnDownLoadFileEnd.Broadcast(RedirectFilePath, StateIndex, true);
							UE_LOG(ReadMessageTaskLog, Error, TEXT("读取到数据量：%d"), RecvSize);
							T->MyDebug(FColor::Red, FString(TEXT("读取到数据量：")) + FString::FromInt(RecvSize));

							ReadBuff.Empty();
							RecvSize = 0;
							});
					}
				}
#else
				AsyncTask(ENamedThreads::GameThread, [=] {
					FString RedirectFilePath = SaveFilePath;
					if (FPaths::FileExists(*SaveFilePath)) {
						if (!FPaths::DirectoryExists(UTestFunctionLibrary::GetRootPath() + TEXT("Content/Movies/Temp"))) {
							FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*(UTestFunctionLibrary::GetRootPath() + TEXT("Content/Movies/Temp")));
						}
						RedirectFilePath = UTestFunctionLibrary::GetRootPath() + TEXT("Content/Movies/Temp/") + FString::FromInt(FMath::RandRange(0, 9999999)) + TEXT(".wav");

						FAttachWavFileInfo Info;
						Info.WavFilePath = RedirectFilePath;
						Info.StateIndex = StateIndex;

						if (T->AttachWavFilePathList.Num() == 0) {
							T->AttachWavFilePathList.Add(Info);
						}

						T->AttachWavFilePathList.Add(Info);
						if (FFileHelper::SaveArrayToFile(ReadBuff, *RedirectFilePath)) {
							ReadBuff.Empty();
							RecvSize = 0;
						}
					}
					else {
						if (FFileHelper::SaveArrayToFile(ReadBuff, *SaveFilePath)) {
							T->OnDownLoadFileEnd.Broadcast(SaveFilePath, StateIndex, true);
							UE_LOG(ReadMessageTaskLog, Error, TEXT("读取到数据量：%d"), RecvSize);
							T->MyDebug(FColor::Red, FString(TEXT("读取到数据量：")) + FString::FromInt(RecvSize));

							ReadBuff.Empty();
							RecvSize = 0;
						}
					}
					});
#endif
#pragma endregion

#else
				if (BaseAskDes == nullptr) {
					T->MyDebug(FColor::Red, TEXT("BaseAsk为空"));
					break;
				}

				ReadBuff += TempBuff;

				auto BaseAsk = google::protobuf::MessageFactory::generated_factory()->GetPrototype(BaseAskDes)->New();
				if (!BaseAsk->ParseFromArray(ReadBuff.GetData(), ReadBuff.Num())) {
					T->MyDebug(FColor::Red, TEXT("尝试解析服务器返回数据错误"));
					ReadBuff.Empty();
					continue;
				}

				DigitalLifeNameSpace::BaseMessageType Type = (DigitalLifeNameSpace::BaseMessageType)(BaseAsk->GetReflection()->GetEnumValue(*BaseAsk, BaseAskDes->FindFieldByName("BaseType")));
				bool Success = BaseAsk->GetReflection()->GetBool(*BaseAsk, BaseAskDes->FindFieldByName("bSuccess"));
				if (!Success) {
					T->MyDebug(FColor::Green, TEXT("正在等待更多数据，暂时读取：") + FString::FromInt(ReadSize));
					continue;
				}

				switch (Type) {
					case DigitalLifeNameSpace::BaseMessageType::BASEMESSAGETYPE_None:
						continue;
					case DigitalLifeNameSpace::BaseMessageType::BASEMESSAGETYPE_SWITCHMESH:
						{
							DigitalLifeNameSpace::S_SwitchMesh ServerData;
							if (!ServerData.ParseFromArray(ReadBuff.GetData(), ReadBuff.Num())) {
								T->MyDebug(FColor::Red, TEXT("尝试解析服务器返回数据错误"));
								ReadBuff.Empty();
								continue;
							}
							AsyncTask(ENamedThreads::GameThread, [=] {
								T->OnSwitchPlayerMesh.Broadcast(ServerData.meshtype());
							});
						}
						continue;
					case DigitalLifeNameSpace::BaseMessageType::BASEMESSAGETYPE_ALLOWEDSAY:
						T->MyDebug(FColor::Red, TEXT("服务器返回允许说话"));
						AsyncTask(ENamedThreads::GameThread, [=] {
							T->OnWaitServerData.Broadcast();
						});
						continue;
				}

				DigitalLifeNameSpace::S_RetData ServerData;
				if (!ServerData.ParseFromArray(ReadBuff.GetData(), ReadBuff.Num())) {
					T->MyDebug(FColor::Red, TEXT("尝试解析服务器返回音频数据错误"));
					ReadBuff.Empty();
					continue;
				}
				StateIndex = ServerData.statetype();
				std::string sWavFileData = ServerData.wavfiledata();

				TArray<uint8> WavFileData;
				WavFileData.Init(0, sWavFileData.length() + 1);
				FMemory::Memcpy(WavFileData.GetData(), sWavFileData.c_str(), sWavFileData.length());

#if !PLATFORM_ANDROID
				FString RedirectFilePath = SaveFilePath;
				if (FPaths::FileExists(*SaveFilePath)) {
					if (!FPaths::DirectoryExists(UTestFunctionLibrary::GetRootPath() + TEXT("Temp/Temp"))) {
						FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*(UTestFunctionLibrary::GetRootPath() + TEXT("Temp/Temp")));
					}
					RedirectFilePath = UTestFunctionLibrary::GetRootPath() + TEXT("Temp/Temp/") + FString::FromInt(FMath::RandRange(0, 9999999)) + TEXT(".wav");

					if (FFileHelper::SaveArrayToFile(WavFileData, *RedirectFilePath)) {

						FAttachWavFileInfo Info;
						Info.WavFilePath = RedirectFilePath;
						Info.StateIndex = StateIndex;

						if (T->AttachWavFilePathList.Num() == 0) {
							T->AttachWavFilePathList.Add(Info);
						}

						T->AttachWavFilePathList.Add(Info);

						ReadBuff.Empty();
						RecvSize = 0;
					}
				}
				else {
					if (FFileHelper::SaveArrayToFile(WavFileData, *RedirectFilePath)) {
						AsyncTask(ENamedThreads::GameThread, [=] {
							T->OnDownLoadFileEnd.Broadcast(RedirectFilePath, StateIndex, true);
							UE_LOG(ReadMessageTaskLog, Error, TEXT("读取到数据量：%d"), RecvSize);
							T->MyDebug(FColor::Red, FString(TEXT("读取到数据量：")) + FString::FromInt(RecvSize));

							ReadBuff.Empty();
							RecvSize = 0;
						});
					}
				}
#else
				AsyncTask(ENamedThreads::GameThread, [=] {
					FString RedirectFilePath = SaveFilePath;
					if (FPaths::FileExists(*SaveFilePath)) {
						if (!FPaths::DirectoryExists(UTestFunctionLibrary::GetRootPath() + TEXT("Content/Movies/Temp"))) {
							FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*(UTestFunctionLibrary::GetRootPath() + TEXT("Content/Movies/Temp")));
						}
						RedirectFilePath = UTestFunctionLibrary::GetRootPath() + TEXT("Content/Movies/Temp/") + FString::FromInt(FMath::RandRange(0, 9999999)) + TEXT(".wav");

						if (FFileHelper::SaveArrayToFile(WavFileData, *RedirectFilePath)) {

							FAttachWavFileInfo Info;
							Info.WavFilePath = RedirectFilePath;
							Info.StateIndex = StateIndex;

							if (T->AttachWavFilePathList.Num() == 0) {
								T->AttachWavFilePathList.Add(Info);
							}

							T->AttachWavFilePathList.Add(Info);

							ReadBuff.Empty();
							RecvSize = 0;
						}
					}
					else {
						if (FFileHelper::SaveArrayToFile(WavFileData, *SaveFilePath)) {
							T->OnDownLoadFileEnd.Broadcast(SaveFilePath, StateIndex, true);
							UE_LOG(ReadMessageTaskLog, Error, TEXT("读取到数据量：%d"), RecvSize);
							T->MyDebug(FColor::Red, FString(TEXT("读取到数据量：")) + FString::FromInt(RecvSize));

							ReadBuff.Empty();
							RecvSize = 0;
						}
					}
				});
#endif
#endif
			}
			continue;
		}
	}
}