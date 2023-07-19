// Fill out your copyright notice in the Description page of Project Settings.


#include "FunctionLibraries/MeshMergeFunctionLibrary.h"

#include "SkeletalMeshMerge.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/Skeleton.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"

void UMeshMergeFunctionLibrary::MergeAndSaveMeshes(const FSkeletalMeshMergeParams& Params)
{
    if(Params.MeshesToMerge.Num() <= 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("Must provide multiple valid Skeletal Meshes in order to perform a merge."));
        return;
    }

    const EMeshBufferAccess BufferAccess = Params.bNeedsCpuAccess ? EMeshBufferAccess::ForceCPUAndGPU : EMeshBufferAccess::Default;
	
    bool bRunDuplicateCheck = false;

    const FString PackageName = TEXT("/Game/SavedAssets/NewSkeletalMesh");
    auto* Package = CreatePackage(*PackageName);
    Package->FullyLoad();
    
    USkeletalMesh* BaseMesh = NewObject<USkeletalMesh>(Package, *PackageName, RF_Public|RF_Standalone|RF_MarkAsRootSet, nullptr);
    BaseMesh->AddToRoot();
    
    if (Params.Skeleton && Params.bSkeletonBefore)
    {
        BaseMesh->SetSkeleton(Params.Skeleton);
        bRunDuplicateCheck = true;
        for (USkeletalMeshSocket* Socket : BaseMesh->GetMeshOnlySocketList())
        {
            if (Socket)
            {
                UE_LOG(LogTemp, Warning, TEXT("SkelMeshSocket: %s"), *(Socket->SocketName.ToString()));
            }
        }
        for (USkeletalMeshSocket* Socket : BaseMesh->GetSkeleton()->Sockets)
        {
            if (Socket)
            {
                UE_LOG(LogTemp, Warning, TEXT("SkelSocket: %s"), *(Socket->SocketName.ToString()));
            }
        }
    }
    
    FSkeletalMeshMerge Merger(BaseMesh, Params.MeshesToMerge, Params.SectionMappings, Params.StripTopLODS, BufferAccess, &Params.UVTransformMappings);
    if (!Merger.DoMerge())
    {
        UE_LOG(LogTemp, Warning, TEXT("Merge failed!"));
        return;
    }
    if (Params.Skeleton && !Params.bSkeletonBefore)
    {
        BaseMesh->SetSkeleton(Params.Skeleton);
    }
    
    if (bRunDuplicateCheck)
    {
        TArray<FName> SkelMeshSockets;
        TArray<FName> SkelSockets;
        for (const USkeletalMeshSocket* Socket : BaseMesh->GetMeshOnlySocketList())
        {
            if (Socket)
            {
                SkelMeshSockets.Add(Socket->GetFName());
                UE_LOG(LogTemp, Warning, TEXT("SkelMeshSocket: %s"), *(Socket->SocketName.ToString()));
            }
        }

        if(!BaseMesh->GetSkeleton())
        {
            UE_LOG(LogTemp, Warning, TEXT("Skeleton was not set"));
            return;
        }
        for (const USkeletalMeshSocket* Socket : BaseMesh->GetSkeleton()->Sockets)
        {
            if (Socket)
            {
                SkelSockets.Add(Socket->GetFName());
                UE_LOG(LogTemp, Warning, TEXT("SkelSocket: %s"), *(Socket->SocketName.ToString()));
            }
        }
        
        TSet<FName> UniqueSkelMeshSockets;
        TSet<FName> UniqueSkelSockets;
        UniqueSkelMeshSockets.Append(SkelMeshSockets);
        UniqueSkelSockets.Append(SkelSockets);
        int32 Total = SkelSockets.Num() + SkelMeshSockets.Num();
        int32 UniqueTotal = UniqueSkelMeshSockets.Num() + UniqueSkelSockets.Num();
        UE_LOG(LogTemp, Warning, TEXT("SkelMeshSocketCount: %d | SkelSocketCount: %d | Combined: %d"), SkelMeshSockets.Num(), SkelSockets.Num(), Total);
        UE_LOG(LogTemp, Warning, TEXT("SkelMeshSocketCount: %d | SkelSocketCount: %d | Combined: %d"), UniqueSkelMeshSockets.Num(), UniqueSkelSockets.Num(), UniqueTotal);
        UE_LOG(LogTemp, Warning, TEXT("Found Duplicates: %s"), *((Total != UniqueTotal) ? FString("True") : FString("False")));
    }

    Package->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(BaseMesh);
    FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());

    FSavePackageArgs SavePackageArgs;

    SavePackageArgs.TopLevelFlags = RF_Public | RF_Standalone;
    SavePackageArgs.bForceByteSwapping = true;
    SavePackageArgs.bWarnOfLongFilename = true;
    SavePackageArgs.SaveFlags = SAVE_None;
    
    bool bSaved = UPackage::SavePackage(Package, BaseMesh, *PackageFileName, SavePackageArgs);
}
