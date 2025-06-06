// MazeGenerator.cpp
#include "MazeGenerator.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

AMazeGenerator::AMazeGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AMazeGenerator::BeginPlay()
{
    Super::BeginPlay();
    GenerateMaze();
}

void AMazeGenerator::GenerateMaze()
{
    // Remove previous maze
    TArray<AActor*> AttachedActors;
    GetAttachedActors(AttachedActors);
    for (AActor* Child : AttachedActors)
    {
        if (Child) Child->Destroy();
    }

    // Generate maze data
    TArray<TArray<int32>> Maze;
    GenerateMazeData(Maze);

    MazeData = Maze; // обязательно!

    // Spawn meshes
    SpawnMazeMeshes(Maze);
}

const TArray<TArray<int32>>& AMazeGenerator::GetMazeData() const
{
    return MazeData;
}

void AMazeGenerator::GenerateMazeData(TArray<TArray<int32>>& Maze)
{
    // Maze size: (2*Width+1)x(2*Height+1), 1=wall, 0=path
    int32 W = MazeWidth;
    int32 H = MazeHeight;
    Maze.SetNum(H * 2 + 1);
    for (int32 y = 0; y < Maze.Num(); ++y)
        Maze[y].Init(1, W * 2 + 1);

    // DFS stack
    TArray<FCell> Stack;
    int32 StartX = 1;
    int32 StartY = 1;
    Maze[StartY][StartX] = 0;
    Stack.Add(FCell(StartX, StartY));

    FCell Directions[] = { FCell(0, -2), FCell(0, 2), FCell(-2, 0), FCell(2, 0) };

    while (Stack.Num() > 0)
    {
        FCell Current = Stack.Last();
        TArray<FCell> Neighbors;

        for (const FCell& Dir : Directions)
        {
            int32 NX = Current.X + Dir.X;
            int32 NY = Current.Y + Dir.Y;
            if (NX > 0 && NX < W * 2 && NY > 0 && NY < H * 2 && Maze[NY][NX] == 1)
            {
                Neighbors.Add(FCell(NX, NY));
            }
        }

        if (Neighbors.Num() > 0)
        {
            FCell Next = Neighbors[FMath::RandRange(0, Neighbors.Num() - 1)];
            Maze[(Current.Y + Next.Y) / 2][(Current.X + Next.X) / 2] = 0;
            Maze[Next.Y][Next.X] = 0;
            Stack.Add(Next);
        }
        else
        {
            Stack.Pop();
        }
    }

    // Entrance and exit
    Maze[0][1] = 0;
    Maze[H * 2][W * 2 - 1] = 0;
}

void AMazeGenerator::SpawnMazeMeshes(const TArray<TArray<int32>>& Maze)
{
    if (!WallMesh || !FloorMesh) return;

    float CellSize = 100.0f;
    FVector Origin = GetActorLocation();

    for (int32 y = 0; y < Maze.Num(); ++y)
    {
        for (int32 x = 0; x < Maze[y].Num(); ++x)
        {
            FVector Location = Origin + FVector(x * CellSize, y * CellSize, 0);
            if (Maze[y][x] == 1)
            {
                SpawnStaticMesh(WallMesh, WallMaterial, Location);
            }
            else
            {
                SpawnStaticMesh(FloorMesh, FloorMaterial, Location - FVector(0, 0, 5));
            }
        }
    }
}

void AMazeGenerator::SpawnStaticMesh(UStaticMesh* Mesh, UMaterialInterface* Material, const FVector& Location, const FRotator& Rotation)
{
    UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(this);
    MeshComp->SetStaticMesh(Mesh);
    if (Material)
        MeshComp->SetMaterial(0, Material);
    MeshComp->RegisterComponent();
    MeshComp->SetWorldLocation(Location);
    MeshComp->SetWorldRotation(Rotation);
    MeshComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
}
