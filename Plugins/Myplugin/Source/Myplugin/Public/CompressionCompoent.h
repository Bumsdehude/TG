// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Core.h"
#include "Components/ActorComponent.h"
#include "CompressionCompoent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYPLUGIN_API UCompressionCompoent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCompressionCompoent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Executeable", meta = (WorldContext = "WorldContextObject"))
	void RunExec(FString filename, FString arguments);
	UFUNCTION(BlueprintCallable, Category = "Compression", meta = (WorldContext = "WorldContextObject"))
		FString ReturnCompressedArray(TArray<uint8> InBinaryArray);
	UFUNCTION(BlueprintCallable, Category = "Compression", meta = (WorldContext = "WorldContextObject"))
		TArray<uint8> FStringToBinaryArray(FString InString);
	FString BinaryArrayToFString(TArray<uint8> InBinaryArray);
	UFUNCTION(BlueprintCallable, Category = "Compression", meta = (WorldContext = "WorldContextObject"))
		FString DecompressBytes(TArray<uint8> CompressedBinaryArray);
	UFUNCTION(BlueprintCallable, Category = "Compression", meta = (WorldContext = "WorldContextObject"))
		TArray<uint8> CompressBytes(FString UncompressedString);

};
