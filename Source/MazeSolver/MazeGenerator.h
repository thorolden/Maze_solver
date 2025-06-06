#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "MazeGenerator.generated.h"

USTRUCT(BlueprintType)
struct FCell
{
    GENERATED_BODY()

    UPROPERTY()
    int32 X;

    UPROPERTY()
    int32 Y;

    FCell() : X(0), Y(0) {}
    FCell(int32 InX, int32 InY) : X(InX), Y(InY) {}
};

UCLASS(Blueprintable)
class MAZESOLVER_API AMazeGenerator : public AActor
{
    GENERATED_BODY()

public:
    AMazeGenerator();

    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze Settings")
    int32 MazeWidth = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze Settings")
    int32 MazeHeight = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze Settings")
    UStaticMesh* WallMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze Settings")
    UStaticMesh* FloorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze Settings")
    UMaterialInterface* WallMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze Settings")
    UMaterialInterface* FloorMaterial;

    UFUNCTION(BlueprintCallable, Category = "Maze Generation")
    void GenerateMaze();

    const TArray<TArray<int32>>& GetMazeData() const;

protected:
    void GenerateMazeData(TArray<TArray<int32>>& Maze);
    void SpawnMazeMeshes(const TArray<TArray<int32>>& Maze);
    void SpawnStaticMesh(UStaticMesh* Mesh, UMaterialInterface* Material, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

private:
    TArray<TArray<int32>> MazeData;
};
