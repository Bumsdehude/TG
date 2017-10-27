// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Myplugin.h"
#include "MypluginBPLibrary.h"
#include "AssetRegistryModule.h"
#include "ARFilter.h"
#include "Runtime/Core/Public/HAL/FileManager.h"
#include "core.h"
#include "AES.h"
#include "zlib.h"
#include "Engine/Blueprint.h"


UMypluginBPLibrary::UMypluginBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	
}






UClass* UMypluginBPLibrary::GetClassByNameImpl(FString Name) {
	if (UClass* result = FindObject<UClass>(ANY_PACKAGE, *Name)) {
		return result;
	}

	if (UObjectRedirector* RenamedClassRedirector = FindObject<UObjectRedirector>(ANY_PACKAGE, *Name)) {
		return CastChecked<UClass>(RenamedClassRedirector->DestinationObject);
	}

	return nullptr;
}



UClass* UMypluginBPLibrary::GetClassByName(FString Name) {
	if (UClass *result = UMypluginBPLibrary::GetClassByNameImpl(Name)) {
		return result;
	}

	FString BlueprintName = FString::Printf(TEXT("%s_C"), *Name);

	if (UClass *result = UMypluginBPLibrary::GetClassByNameImpl(BlueprintName)) {
		return result;
	}


	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> AssetList;
	FARFilter AssetFilter;
	AssetFilter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
	AssetRegistry.GetAssets(AssetFilter, AssetList);

	for (auto Itr(AssetList.CreateIterator()); Itr; Itr++)
	{
		if (Itr->AssetName.ToString().Equals(Name)) {
			if (UClass* result = LoadObject<UClass>(nullptr, *Itr->ObjectPath.ToString())) {
				return result;
			}

			if (UClass *result = UMypluginBPLibrary::GetClassByNameImpl(BlueprintName)) {
				return result;
			}
		}
	}

	return nullptr;
}


bool UMypluginBPLibrary::CloneFile(FString FilePath, FString Source)
{

	IFileManager& FileManager = IFileManager::Get();
	FileManager.Copy(*FilePath, *Source, true, true, false, nullptr, FILEREAD_None, FILEWRITE_None);

	return true;
}






bool UMypluginBPLibrary::MakeFolder(FString FilePath)
{
	IFileManager& FileManager = IFileManager::Get();
	return FileManager.MakeDirectory(*FilePath, true);


}


bool UMypluginBPLibrary::DeleteFile(FString FilePath, bool EvenIfReadOnly)
{
	bool n = EvenIfReadOnly;
	IFileManager& FileManager = IFileManager::Get();
	return FileManager.Delete(*FilePath, true, n, false);


}


bool UMypluginBPLibrary::DoesFolderExist(FString FilePath)
{
	IFileManager& FileManager = IFileManager::Get();
	return FileManager.DirectoryExists(*FilePath);


}

bool UMypluginBPLibrary::DeleteFolder(FString FilePath, bool EvenIfReadOnly, bool Tree)
{
	bool n = EvenIfReadOnly;
	bool m = Tree;
	IFileManager& FileManager = IFileManager::Get();
	return FileManager.DeleteDirectory(*FilePath, n, m);


}

FString UMypluginBPLibrary::GetGameDir()
{
	return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
}



// Set the SoundClass volume
void UMypluginBPLibrary::SetSoundClassVolume(USoundClass* SoundClass, float Volume)
{
	if (SoundClass)
	{
		SoundClass->Properties.Volume = Volume;
	}
}

// Get the SoundClass volume
float UMypluginBPLibrary::GetSoundClassVolume(USoundClass* SoundClass)
{
	if (SoundClass)
	{
		return SoundClass->Properties.Volume;
	}

	return 0.f;
}

FMainMenuLibInput UMypluginBPLibrary::GetMainMenuLibInput(const FKeyEvent& KeyEvent)
{
	FMainMenuLibInput Input;

	Input.Key = KeyEvent.GetKey();
	Input.KeyAsString = Input.Key.GetDisplayName().ToString();

	Input.bAlt = KeyEvent.IsAltDown();
	Input.bCtrl = KeyEvent.IsControlDown();
	Input.bShift = KeyEvent.IsShiftDown();
	Input.bCmd = KeyEvent.IsCommandDown();

	return Input;
}

FMainMenuLibInputAxis UMypluginBPLibrary::GetMainMenuLibInputAxis(const FKeyEvent& KeyEvent)
{
	FMainMenuLibInputAxis Input;

	Input.Key = KeyEvent.GetKey();
	Input.KeyAsString = Input.Key.GetDisplayName().ToString();

	Input.Scale = 1;

	return Input;
}

void UMypluginBPLibrary::GetAllAxisKeyBindings(TArray<FMainMenuLibInputAxis>& Bindings)
{
	Bindings.Empty();

	const UInputSettings* Settings = GetDefault<UInputSettings>();
	if (!Settings) return;

	const TArray<FInputAxisKeyMapping>& Axi = Settings->AxisMappings;

	for (const FInputAxisKeyMapping& Each : Axi)
	{
		Bindings.Add(FMainMenuLibInputAxis(Each));
	}
}

void UMypluginBPLibrary::RemoveAxisKeyBind(FMainMenuLibInputAxis ToRemove)
{
	//GetMutableDefault
	UInputSettings* Settings = GetMutableDefault<UInputSettings>();
	if (!Settings) return;

	TArray<FInputAxisKeyMapping>& Axi = Settings->AxisMappings;

	bool Found = false;
	for (int32 v = 0; v < Axi.Num(); v++)
	{
		if (Axi[v].Key == ToRemove.Key)
		{
			Found = true;
			Axi.RemoveAt(v);
			v = 0;
			continue;
		}
	}

	if (Found)
	{
		//SAVES TO DISK
		Settings->SaveKeyMappings();

		//REBUILDS INPUT, creates modified config in Saved/Config/Windows/Input.ini
		for (TObjectIterator<UPlayerInput> It; It; ++It)
		{
			It->ForceRebuildingKeyMaps(true);
		}
	}
}

void UMypluginBPLibrary::GetAllActionKeyBindings(TArray<FMainMenuLibInput>& Bindings)
{
	Bindings.Empty();

	const UInputSettings* Settings = GetDefault<UInputSettings>();
	if (!Settings) return;

	const TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;

	for (const FInputActionKeyMapping& Each : Actions)
	{
		Bindings.Add(FMainMenuLibInput(Each));
	}
}

void UMypluginBPLibrary::RemoveActionKeyBind(FMainMenuLibInput ToRemove)
{
	//GetMutableDefault
	UInputSettings* Settings = GetMutableDefault<UInputSettings>();
	if (!Settings) return;

	TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;

	bool Found = false;
	for (int32 v = 0; v < Actions.Num(); v++)
	{
		if (Actions[v].Key == ToRemove.Key)
		{
			Found = true;
			Actions.RemoveAt(v);
			v = 0;
			continue;
		}
	}

	if (Found)
	{
		//SAVES TO DISK
		Settings->SaveKeyMappings();

		//REBUILDS INPUT, creates modified config in Saved/Config/Windows/Input.ini
		for (TObjectIterator<UPlayerInput> It; It; ++It)
		{
			It->ForceRebuildingKeyMaps(true);
		}
	}
}

void UMypluginBPLibrary::GetAllAxisAndActionMappingsForKey(FKey Key, TArray<FMainMenuLibInput>& ActionBindings, TArray<FMainMenuLibInputAxis>& AxisBindings)
{
	ActionBindings.Empty();
	AxisBindings.Empty();

	const UInputSettings* Settings = GetDefault<UInputSettings>();
	if (!Settings) return;

	const TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;

	for (const FInputActionKeyMapping& Each : Actions)
	{
		if (Each.Key == Key)
		{
			ActionBindings.Add(FMainMenuLibInput(Each));
		}
	}

	const TArray<FInputAxisKeyMapping>& Axi = Settings->AxisMappings;

	for (const FInputAxisKeyMapping& Each : Axi)
	{
		if (Each.Key == Key)
		{
			AxisBindings.Add(FMainMenuLibInputAxis(Each));
		}
	}
}

bool UMypluginBPLibrary::ReBindAxisKey(FMainMenuLibInputAxis Original, FMainMenuLibInputAxis NewBinding)
{
	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	if (!Settings) return false;

	TArray<FInputAxisKeyMapping>& Axi = Settings->AxisMappings;

	//~~~

	bool Found = false;
	for (FInputAxisKeyMapping& Each : Axi)
	{
		//Search by original
		if (Each.AxisName.ToString() == Original.AxisName &&
			Each.Key == Original.Key
			) {
			//Update to new!
			UMypluginBPLibrary::UpdateAxisMapping(Each, NewBinding);
			Found = true;
			break;
		}
	}

	if (Found)
	{
		//SAVES TO DISK
		const_cast<UInputSettings*>(Settings)->SaveKeyMappings();

		//REBUILDS INPUT, creates modified config in Saved/Config/Windows/Input.ini
		for (TObjectIterator<UPlayerInput> It; It; ++It)
		{
			It->ForceRebuildingKeyMaps(true);
		}
	}
	return Found;
}

bool UMypluginBPLibrary::ReBindActionKey(FMainMenuLibInput Original, FMainMenuLibInput NewBinding)
{
	UInputSettings* Settings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
	if (!Settings) return false;

	TArray<FInputActionKeyMapping>& Actions = Settings->ActionMappings;

	//~~~

	bool Found = false;
	for (FInputActionKeyMapping& Each : Actions)
	{
		//Search by original
		if (Each.ActionName.ToString() == Original.ActionName &&
			Each.Key == Original.Key
			) {
			//Update to new!
			UMypluginBPLibrary::UpdateActionMapping(Each, NewBinding);
			Found = true;
			break;
		}
	}

	if (Found)
	{
		//SAVES TO DISK
		const_cast<UInputSettings*>(Settings)->SaveKeyMappings();

		//REBUILDS INPUT, creates modified config in Saved/Config/Windows/Input.ini
		for (TObjectIterator<UPlayerInput> It; It; ++It)
		{
			It->ForceRebuildingKeyMaps(true);
		}
	}
	return Found;
}




// Set quality levels from benchmark
bool UMypluginBPLibrary::SetQualitySettingsFromBench()
{
	Scalability::FQualityLevels Results;

	Results = Scalability::BenchmarkQualityLevels();

	UGameUserSettings* Settings = GetGameUserSettings();

	if (!Settings) {
		return false;
	}

	Settings->ScalabilityQuality.AntiAliasingQuality = Results.AntiAliasingQuality;
	Settings->ScalabilityQuality.EffectsQuality = Results.EffectsQuality;
	Settings->ScalabilityQuality.PostProcessQuality = Results.PostProcessQuality;
	Settings->ScalabilityQuality.ResolutionQuality = Results.ResolutionQuality;
	Settings->ScalabilityQuality.ShadowQuality = Results.ShadowQuality;
	Settings->ScalabilityQuality.TextureQuality = Results.TextureQuality;
	Settings->ScalabilityQuality.ViewDistanceQuality = Results.ViewDistanceQuality;

	return true;

}

// Set quality from single quality level
// @param Value 0:low, 1:medium, 2:high, 3:epic
bool UMypluginBPLibrary::SetQualitySettings(int32 Value)
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	Settings->ScalabilityQuality.SetFromSingleQualityLevel(Value);

	return true;
}

// Get a list of screen resolutions supported by video adapter
// NOTE: This function needs "RHI" to be added to <Project>.Build.cs file!
bool UMypluginBPLibrary::GetSupportedScreenResolutions(TArray<FString> &Resolutions)
{
	FScreenResolutionArray ResolutionsArray;

	if (RHIGetAvailableResolutions(ResolutionsArray, true))
	{
		for (const FScreenResolutionRHI &Resolution : ResolutionsArray)
		{
			if (Resolution.Width < MIN_SCREEN_WIDTH || Resolution.Height < MIN_SCREEN_HEIGHT)
			{
				continue;
			}

			FString StrW = FString::FromInt(Resolution.Width);
			FString StrH = FString::FromInt(Resolution.Height);
			Resolutions.AddUnique(StrW + "x" + StrH);
		}

		return true;
	}

	return false;	// Failed to obtain screen resolutions
}

// Get currently set screen resolution
FString UMypluginBPLibrary::GetScreenResolution()
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return FString("");
	}

	FIntPoint Resolution = Settings->GetScreenResolution();
	return FString::FromInt(Resolution.X) + "x" + FString::FromInt(Resolution.Y);
}

// Set the fullscreen mode
bool UMypluginBPLibrary::SetFullscreenMode(int32 WindowMode)
{

	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	switch (WindowMode)
	{
	case 0:
		Settings->SetFullscreenMode(EWindowMode::Fullscreen);
		break;

	case 1:
		Settings->SetFullscreenMode(EWindowMode::WindowedFullscreen);
		break;

	default:
		Settings->SetFullscreenMode(EWindowMode::Windowed);
		break;
	}

	return true;
}

// Get the fullscreen mode
bool UMypluginBPLibrary::GetFullscreenMode(int32 &FullscreenMode)
{

	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}


	switch (Settings->GetFullscreenMode())
	{
	case EWindowMode::Fullscreen:
		FullscreenMode = 0;
		break;

	case EWindowMode::WindowedFullscreen:
		FullscreenMode = 1;
		break;

	default:
		FullscreenMode = 2;
		break;
	}

	return true;

}

// Check whether or not we are currently running in fullscreen mode
bool UMypluginBPLibrary::IsFullscreen()
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	return Settings->GetFullscreenMode() == EWindowMode::Fullscreen;
}

// Check whether or not we are currently running in windowed fullscreen mode (borderless)
bool UMypluginBPLibrary::IsWindowedFullscreen()
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	return Settings->GetFullscreenMode() == EWindowMode::WindowedFullscreen;
}

// Check whether or not we are currently running in windowed mode
bool UMypluginBPLibrary::IsWindowed()
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	return Settings->GetFullscreenMode() == EWindowMode::Windowed;
}

// Set the desired screen resolution (does not change it yet)
bool UMypluginBPLibrary::SetScreenResolution(const int32 Width, const int32 Height, const int32 FullscreenMode)
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	Settings->SetWindowPosition(-1, -1);
	Settings->SetScreenResolution(FIntPoint(Width, Height));

	switch (FullscreenMode)
	{

	case 0:
		Settings->SetFullscreenMode(EWindowMode::Fullscreen);
		break;

	case 1:
		Settings->SetFullscreenMode(EWindowMode::WindowedFullscreen);
		break;

	default:
		Settings->SetFullscreenMode(EWindowMode::Windowed);
		break;
	}

	return true;
}

// Change the current screen resolution
bool UMypluginBPLibrary::ChangeScreenResolution(const int32 Width, const int32 Height, const int32 FullscreenMode)
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	Settings->SetWindowPosition(-1, -1);
	EWindowMode::Type WindowMode;

	switch (FullscreenMode)
	{

	case 0:
		WindowMode = EWindowMode::Fullscreen;
		break;

	case 1:
		WindowMode = EWindowMode::WindowedFullscreen;
		break;

	default:
		WindowMode = EWindowMode::Windowed;
		break;
	}

	Settings->RequestResolutionChange(Width, Height, WindowMode, false);

	return true;
}

// Get the current video quality settings
bool UMypluginBPLibrary::GetVideoQualitySettings(int32 &AntiAliasing, int32 &Effects, int32 &PostProcess, int32 &Resolution, int32 &Shadow, int32 &Texture, int32 &ViewDistance)
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	AntiAliasing = Settings->ScalabilityQuality.AntiAliasingQuality;
	Effects = Settings->ScalabilityQuality.EffectsQuality;
	PostProcess = Settings->ScalabilityQuality.PostProcessQuality;
	Resolution = Settings->ScalabilityQuality.ResolutionQuality;
	Shadow = Settings->ScalabilityQuality.ShadowQuality;
	Texture = Settings->ScalabilityQuality.TextureQuality;
	ViewDistance = Settings->ScalabilityQuality.ViewDistanceQuality;

	return true;
}

// Set the quality settings (not applied nor saved yet)
bool UMypluginBPLibrary::SetVideoQualitySettings(const int32 AntiAliasing, const  int32 Effects, const  int32 PostProcess, const  int32 Resolution, const  int32 Shadow, const  int32 Texture, const  int32 ViewDistance)
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	Settings->ScalabilityQuality.AntiAliasingQuality = AntiAliasing;
	Settings->ScalabilityQuality.EffectsQuality = Effects;
	Settings->ScalabilityQuality.PostProcessQuality = PostProcess;
	Settings->ScalabilityQuality.ResolutionQuality = Resolution;
	Settings->ScalabilityQuality.ShadowQuality = Shadow;
	Settings->ScalabilityQuality.TextureQuality = Texture;
	Settings->ScalabilityQuality.ViewDistanceQuality = ViewDistance;

	return true;
}

// Check whether or not we have vertical sync enabled
bool UMypluginBPLibrary::IsVSyncEnabled()
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	return Settings->IsVSyncEnabled();
}

// Set the vertical sync flag
bool UMypluginBPLibrary::SetVSyncEnabled(const bool VSync)
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	Settings->SetVSyncEnabled(VSync);
	return true;
}

// Load user settings from persistence storage
bool UMypluginBPLibrary::LoadVideoModeAndQuality()
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	Settings->LoadSettings();

	return true;
}

// Confirm and save current video mode (resolution and fullscreen/windowed)
bool UMypluginBPLibrary::SaveVideoModeAndQuality()
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	Settings->ConfirmVideoMode();
	Settings->ApplyNonResolutionSettings();
	Settings->SaveSettings();

	return true;
}


// Revert to original video settings
bool UMypluginBPLibrary::RevertVideoMode()
{
	UGameUserSettings* Settings = GetGameUserSettings();
	if (!Settings)
	{
		return false;
	}

	Settings->RevertVideoMode();
	return true;
}

// =================== PRIVATE METHODS ========================

// Try to get the GameUserSettings object from the engine
UGameUserSettings* UMypluginBPLibrary::GetGameUserSettings()
{
	if (GEngine != nullptr)
	{
		return GEngine->GameUserSettings;
	}

	return nullptr;
}
