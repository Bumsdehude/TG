// Fill out your copyright notice in the Description page of Project Settings.

#include "Myplugin.h"
#include "zlib.h"
#include "CompressionCompoent.h"


// Sets default values for this component's properties
UCompressionCompoent::UCompressionCompoent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}
void UCompressionCompoent::BeginPlay()
{
}


// Called when the game starts


void UCompressionCompoent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
}

void UCompressionCompoent::RunExec(FString filename, FString arguments)
{
	const TCHAR* tFilename = *filename;
	const TCHAR* tArguments = *arguments;

	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, tFilename);

	FPlatformProcess::CreateProc(tFilename, tArguments, true, false, false, NULL, 0,NULL, NULL);
}


FString UCompressionCompoent::ReturnCompressedArray(TArray<uint8> InBinaryArray)
{
	return BinaryArrayToFString(InBinaryArray);

}



TArray<uint8>  UCompressionCompoent::FStringToBinaryArray(FString InString)
{
	TArray<uint8> BinaryArray;

	for (int32 i = 0; i < InString.Len(); i++)
	{
		BinaryArray.Add(InString[i]);
	}
	return BinaryArray;
}

FString UCompressionCompoent::BinaryArrayToFString(TArray<uint8> InBinaryArray)
{
	return FString(UTF8_TO_TCHAR(InBinaryArray.GetData()));
}

FString UCompressionCompoent::DecompressBytes(TArray<uint8> CompressedBinaryArray)
{
	TArray<uint8> UncompressedBinaryArray;
	UncompressedBinaryArray.SetNum(CompressedBinaryArray.Num() * 1032);

	//int ret;
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	strm.avail_in = CompressedBinaryArray.Num();
	strm.next_in = (Bytef *)CompressedBinaryArray.GetData();
	strm.avail_out = UncompressedBinaryArray.Num();
	strm.next_out = (Bytef *)UncompressedBinaryArray.GetData();

	// the actual DE-compression work.
	inflateInit(&strm);
	inflate(&strm, Z_FINISH);
	inflateEnd(&strm);

	return BinaryArrayToFString(UncompressedBinaryArray);
}

TArray<uint8> UCompressionCompoent::CompressBytes(FString UncompressedString)
{
	TArray<uint8> UncompressedBinaryArray = FStringToBinaryArray(UncompressedString);
	TArray<uint8> CompressedBinaryArray;
	CompressedBinaryArray.SetNum(UncompressedBinaryArray.Num() * 1023, true);

	//int ret;
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	strm.avail_in = UncompressedBinaryArray.Num();
	strm.next_in = (Bytef *)UncompressedBinaryArray.GetData();
	strm.avail_out = CompressedBinaryArray.Num();
	strm.next_out = (Bytef *)CompressedBinaryArray.GetData();


	// the actual compression work.
	deflateInit(&strm, Z_BEST_COMPRESSION);
	deflate(&strm, Z_FINISH);
	deflateEnd(&strm);

	// Shrink the array to minimum size
	CompressedBinaryArray.RemoveAt(strm.total_out, CompressedBinaryArray.Num() - strm.total_out, true);
	return CompressedBinaryArray;

}
