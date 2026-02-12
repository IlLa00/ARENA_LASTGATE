#include "Character/Components/OZVisionComponent.h"
#include "Utils/OZVisionUtility.h"
#include "Utils/Util.h"
#include "Engine/PostProcessVolume.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Engine/Canvas.h"
#include "CollisionQueryParams.h"
#include "GameFramework/Character.h"
#include "Character/OZPlayer.h"
#include "Character/OZPlayerState.h"
#include "Widget/OZWidgetComponent.h"
#include "Character/OZPlayerAttributeSet.h"
#include "Item/Battle/OZSmokeArea.h"
#include "Object/OZTowerVisionArea.h"
#include "AbilitySystemComponent.h"
#include "Tags/OZGameplayTags.h"
#include "NiagaraComponent.h"

// ===== Unreal Insights 프로파일링을 위한 헤더 추가 =====
#include "ProfilingDebugging/CpuProfilerTrace.h"

UOZVisionComponent::UOZVisionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UOZVisionComponent::BeginPlay()
{
	Super::BeginPlay();


    BaseTraceRange = TraceRange;

    TryInitializeViewingAngle();

    if (!IsActivate)
        return;

    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
        return;

    if (OwnerCharacter->IsLocallyControlled())
    {
        InitializeVisionSystem();
        RefreshActorCaches();  // 초기 캐시 설정
    }

    TArray<UMaterialInterface*> Materials = OwnerCharacter->GetMesh()->GetMaterials();
    if (Materials.Num() <= 0)
        return;

    UTextureRenderTarget2D* VisionTextureToUse = nullptr;

    if (OwnerCharacter->IsLocallyControlled())
    {
        VisionTextureToUse = DynamicRenderTarget;
    }
    else
    {
        ULocalPlayer* LocalPlayerFromController = GetWorld()->GetFirstLocalPlayerFromController();
        if (LocalPlayerFromController)
        {
            APlayerController* LocalPlayerController = LocalPlayerFromController->GetPlayerController(GetWorld());
            if (LocalPlayerController && LocalPlayerController->GetPawn())
            {
                AOZPlayer* LocalPlayer = Cast<AOZPlayer>(LocalPlayerController->GetPawn());
                if (LocalPlayer && LocalPlayer->VisionComp && LocalPlayer->VisionComp->DynamicRenderTarget)
                {
                    VisionTextureToUse = LocalPlayer->VisionComp->DynamicRenderTarget;
                }
            }
        }
    }

    if (!VisionTextureToUse)
        return;

    for (int32 i = 0; i < Materials.Num(); i++)
    {
        UMaterialInstanceDynamic* MI = OwnerCharacter->GetMesh()->CreateDynamicMaterialInstance(i, Materials[i]);
        if (MI)
        {
            MI->SetTextureParameterValue("VisionTexture", VisionTextureToUse);
        }
    }

    AOZPlayer* OwnerPlayer = Cast<AOZPlayer>(GetOwner());
    if (OwnerPlayer && OwnerPlayer->GetWeaponMesh())
    {
        TArray<UMaterialInterface*> WeaponMaterials = OwnerPlayer->GetWeaponMesh()->GetMaterials();

        for (int32 i = 0; i < WeaponMaterials.Num(); i++)
        {
            UMaterialInstanceDynamic* WMI = OwnerPlayer->GetWeaponMesh()->CreateDynamicMaterialInstance(i, WeaponMaterials[i]);
            if (WMI)
            {
                WMI->SetTextureParameterValue("VisionTexture", VisionTextureToUse);
            }
        }
    }
}

void UOZVisionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (EndPlayReason == EEndPlayReason::Type::Destroyed || EndPlayReason == EEndPlayReason::Type::EndPlayInEditor)
    {
        DynamicRenderTarget = nullptr;

        VisionMaterialInstance = nullptr;
        CachedPostProcessVolume = nullptr;
        bIsVisionInitialized = false;
    }
}

void UOZVisionComponent::InitializeVisionSystem()
{
    if (bIsVisionInitialized)
        return;

    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
        return;

    if (!OwnerCharacter->IsLocallyControlled())
        return;

    DynamicRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(GetOwner()->GetWorld(), 1800, 1800, ETextureRenderTargetFormat::RTF_RGBA8);
    if (!DynamicRenderTarget)
        return;

    TArray<AActor*> Volumes;
    UGameplayStatics::GetAllActorsOfClass(GetOwner()->GetWorld(), APostProcessVolume::StaticClass(), Volumes);

    if (Volumes.Num() <= 0)
        return;

    int32 VolumeIndex = 0;
    if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
    {
        if (ULocalPlayer* LP = PC->GetLocalPlayer())
        {
            VolumeIndex = LP->GetControllerId();
        }
    }

    int32 SelectedVolumeIndex = (Volumes.Num() > VolumeIndex) ? VolumeIndex : 0;
    CachedPostProcessVolume = Cast<APostProcessVolume>(Volumes[SelectedVolumeIndex]);

    if (!CachedPostProcessVolume)
        return;

    bool bFoundExistingMaterial = false;
    for (const FWeightedBlendable& Blendable : CachedPostProcessVolume->Settings.WeightedBlendables.Array)
    {
        if (UMaterialInstanceDynamic* ExistingMID = Cast<UMaterialInstanceDynamic>(Blendable.Object))
        {
            if (ExistingMID->Parent == Parent)
            {
                VisionMaterialInstance = ExistingMID;
                bFoundExistingMaterial = true;
                break;
            }
        }
    }

    if (!bFoundExistingMaterial)
    {
        VisionMaterialInstance = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetOwner()->GetWorld(), Parent);
        if (!VisionMaterialInstance)
            return;

        CachedPostProcessVolume->AddOrUpdateBlendable(VisionMaterialInstance, 1.0f);
    }

    UKismetMaterialLibrary::SetScalarParameterValue(GetOwner()->GetWorld(), TraceCollection, "TraceRange", TraceRange);
    VisionMaterialInstance->SetTextureParameterValue("VisionTexture", DynamicRenderTarget);

    // 초기화 완료 플래그 설정
    bIsVisionInitialized = true;
}

void UOZVisionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UOZVisionComponent::TickComponent);

    if (!IsActivate)
        return;

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bViewingAngleInitialized)
    {
        TryInitializeViewingAngle();
    }

    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());

    if (OwnerCharacter && OwnerCharacter->IsLocallyControlled() && IsActivate)
    {
        // 액터 캐시 주기적 갱신
        CacheRefreshTimer += DeltaTime;
        if (CacheRefreshTimer >= CacheRefreshInterval)
        {
            CacheRefreshTimer = 0.f;
            RefreshActorCaches();
        }

        LosVisionSystem();
    }
}

void UOZVisionComponent::OnAiming()
{
    bIsAiming = true;
    SetDegreePerTrace(AimingDegreePerTrace);
    RecalculateNumTraces();
    RecalculateTraceRange();
}

void UOZVisionComponent::OffAiming()
{
    bIsAiming = false;
    SetDegreePerTrace(BaseDegreePerTrace);
    RecalculateNumTraces();
    RecalculateTraceRange();
}

void UOZVisionComponent::RecalculateTraceRange()
{
    float Range = BaseTraceRange;

    if (bIsAiming)
    {
        AOZPlayer* OwnerPlayer = Cast<AOZPlayer>(GetOwner());
        if (OwnerPlayer)
        {
            if (AOZPlayerState* PS = Cast<AOZPlayerState>(OwnerPlayer->GetPlayerState()))
            {
                if (PS->PlayerAttributes)
                {
                    Range += PS->PlayerAttributes->GetAimingDistance();
                }
            }
        }
    }

    if (bOwnerInBush)
    {
        Range *= BushTraceRangeRatio;
    }

    SetTraceRange(Range);
}

const TArray<FVector>& UOZVisionComponent::GetVisionResults() const
{
    return TraceResults;
}

void UOZVisionComponent::SetupVisionTextureForRemotePlayers()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UOZVisionComponent::SetupVisionTextureForRemotePlayers);

    if (!DynamicRenderTarget)
        return;

    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
        return;

    // 캐시된 플레이어 목록 사용
    for (AOZPlayer* Player : CachedPlayers)
    {
        if (!Player || Player == OwnerCharacter)
            continue;

        if (InitializedRemotePlayers.Contains(Player))
            continue;

        USkeletalMeshComponent* PlayerMesh = Player->GetMesh();
        if (!PlayerMesh)
            continue;

        TArray<UMaterialInterface*> Materials = PlayerMesh->GetMaterials();
        for (int32 i = 0; i < Materials.Num(); i++)
        {
            UMaterialInstanceDynamic* MI = PlayerMesh->CreateDynamicMaterialInstance(i, Materials[i]);
            if (MI)
            {
                MI->SetTextureParameterValue("VisionTexture", DynamicRenderTarget);
            }

        }

        if (USkeletalMeshComponent* WeaponMesh = Player->GetWeaponMesh())
        {
            TArray<UMaterialInterface*> WeaponMaterials = WeaponMesh->GetMaterials();
            for (int32 i = 0; i < WeaponMaterials.Num(); i++)
            {
                UMaterialInstanceDynamic* WMI = WeaponMesh->CreateDynamicMaterialInstance(i, WeaponMaterials[i]);
                if (WMI)
                {
                    WMI->SetTextureParameterValue("VisionTexture", DynamicRenderTarget);
                }
            }
        }

        UpdateShieldEffectVisibility(Player, false);

        InitializedRemotePlayers.Add(Player);
    }

    // 파괴된 플레이어 정리
    for (auto It = InitializedRemotePlayers.CreateIterator(); It; ++It)
    {
        if (!IsValid(*It))
        {
            It.RemoveCurrent();
        }
    }
}

void UOZVisionComponent::LosVisionSystem()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UOZVisionComponent::LosVisionSystem);

    if (!bIsVisionInitialized)
    {
        InitializeVisionSystem();

        // 초기화 실패 시 리턴
        if (!bIsVisionInitialized)
            return;
    }

    if (!DynamicRenderTarget)
    {
        bIsVisionInitialized = false;
        return;
    }

    if (!VisionMaterialInstance)
    {
        bIsVisionInitialized = false;
        return;
    }

    if (!CachedPostProcessVolume)
    {
        bIsVisionInitialized = false;
        return;
    }

    UKismetMaterialLibrary::SetVectorParameterValue(
        GetOwner()->GetWorld(),
        PlayerCollection,
        "PlayerPosition",
        FLinearColor(GetOwner()->GetActorLocation()));

    SetupVisionTextureForRemotePlayers();

    // 섬광탄에 맞은 경우: 원뿔을 그리지 않고 RenderTarget을 BLACK으로 클리어
    if (bFlashbanged)
    {
        UKismetRenderingLibrary::ClearRenderTarget2D(
            GetOwner()->GetWorld(),
            DynamicRenderTarget,
            FLinearColor::Black
        );

        // 섬광탄 상태에서도 BushHidden 세트와 ForceHidden 파라미터를 업데이트
        UpdateBushHiddenSet();

        ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
        for (AOZPlayer* Player : CachedPlayers)
        {
            if (Player && Player != OwnerCharacter)
            {
                // 위젯 컴포넌트 숨김
                if (UOZWidgetComponent* WidgetComp = Player->StatusWidgetComp)
                {
                    if (UUserWidget* Widget = WidgetComp->GetUserWidgetObject())
                    {
                        Widget->SetVisibility(ESlateVisibility::Hidden);
                    }
                }

                UpdateShieldEffectVisibility(Player, false);
            }
        }

        // 섬광탄 중에는 보이는 플레이어가 없으므로 감지 목록 초기화
        PreviousDetectedPlayers.Empty();
        PreviousTowerDetectedPlayers.Empty();

        // 빈 세트로 호출하여 ForceHidden 값을 정상적으로 업데이트
        TSet<AOZPlayer*> EmptySet;
        UpdateSmokeAndBushVisibility(EmptySet);

        UpdateMinimapVisibility();
        return;
    }

    CreateCone();

    PrePareTriangles();

    // 공통 유틸리티 사용하여 RenderTarget에 그리기
    FOZVisionUtility::DrawToRenderTarget(
        GetOwner()->GetWorld(),
        DynamicRenderTarget,
        CanvasTriangles,
        true
    );

    DrawTowerVisionAreas();
    UpdateMinimapVisibility();
}

void UOZVisionComponent::CreateCone()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UOZVisionComponent::CreateCone);

    TSet<AActor*> CurrentlyDetectedMinimapVisibleObjects;
    TSet<AOZPlayer*> CurrentlyDetectedCharacters;

    // 캐릭터 눈높이에서 레이트레이스 시작 (낮은 장애물 무시)
    FVector EyeLocation = GetOwner()->GetActorLocation();
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
    {
        EyeLocation = OwnerCharacter->GetPawnViewLocation();
    }

    // 공통 유틸리티를 사용하여 시야 원뿔 생성
    TraceResults = FOZVisionUtility::CreateVisionCone(
        GetWorld(),
        EyeLocation,
        GetOwner()->GetActorForwardVector(),
        TraceRange,
        NumTraces,
        DegreePerTrace,
        GetOwner(),
        &CurrentlyDetectedMinimapVisibleObjects,
        &CurrentlyDetectedCharacters  // 감지된 플레이어 수집
    );

    UpdateMinimapPlayerVisionObject(CurrentlyDetectedMinimapVisibleObjects);

    UpdateBushHiddenSet();
    UpdateWidgetVisibility(CurrentlyDetectedCharacters);
    UpdateSmokeAndBushVisibility(CurrentlyDetectedCharacters);
}

void UOZVisionComponent::UpdateWidgetVisibility(const TSet<AOZPlayer*>& VisiblePlayers)
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
        return;

    if (!OwnerCharacter->IsLocallyControlled())
        return;

    for (AOZPlayer* Player : VisiblePlayers)
    {
        // 부쉬에 의해 숨겨진 플레이어는 시야에 감지되어도 위젯을 보여주지 않음
        if (BushHiddenPlayers.Contains(Player))
            continue;

        if (UOZWidgetComponent* WidgetComp = Player->StatusWidgetComp)
        {
            if (UUserWidget* Widget = WidgetComp->GetUserWidgetObject())
            {
                Widget->SetVisibility(ESlateVisibility::Visible);
            }
        }

        UpdateShieldEffectVisibility(Player, true);
    }

    for (AOZPlayer* Player : PreviousDetectedPlayers)
    {
        if (!VisiblePlayers.Contains(Player) || BushHiddenPlayers.Contains(Player))
        {
            // 로컬 플레이어 자신의 위젯은 절대 숨기지 않음
            if (Player == OwnerCharacter)
                continue;

            if (UOZWidgetComponent* WidgetComp = Player->StatusWidgetComp)
            {
                if (UUserWidget* Widget = WidgetComp->GetUserWidgetObject())
                {
                    Widget->SetVisibility(ESlateVisibility::Hidden);
                }
            }

            UpdateShieldEffectVisibility(Player, false);
        }
    }

    PreviousDetectedPlayers = VisiblePlayers;
}

void UOZVisionComponent::UpdateMinimapPlayerVisionObject(const TSet<class AActor*>& MinimapVisibleObjects)
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
        return;

    if (!OwnerCharacter->IsLocallyControlled())
        return;

    PlayerVisionSet = MinimapVisibleObjects;

}

void UOZVisionComponent::UpdateMinimapVisibility()
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
        return;

    if (!OwnerCharacter->IsLocallyControlled())
        return;

    AOZPlayerState* ozPlayerState = Cast<AOZPlayerState>(OwnerCharacter->GetPlayerState());
    if (ozPlayerState == nullptr)
        return;

    TSet<AActor*> PastViewObjects = ozPlayerState->CurrentViewObjects;

    TSet<AActor*> NewFinalSet = PlayerVisionSet;
    NewFinalSet.Append(TowerVisionSet);

    for (AActor*& hiddenActor : HideObjectSet)
    {
        NewFinalSet.Remove(hiddenActor);
    }

    TSet<AActor*> Added = NewFinalSet.Difference(PastViewObjects);
    TSet<AActor*> Removed = PastViewObjects.Difference(NewFinalSet);

    if (AOZPlayerState* PS = Cast<AOZPlayerState>(OwnerCharacter->GetPlayerState()))
    {
        PS->UpdateClientMiniMapUI(Added, Removed);
        PS->CurrentViewObjects = NewFinalSet;
    }
}

void UOZVisionComponent::UpdateSmokeAndBushVisibility(const TSet<AOZPlayer*>& VisiblePlayers)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UOZVisionComponent::UpdateSmokeAndBushVisibility);

    HideObjectSet.Empty();

    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
        return;

    if (!OwnerCharacter->IsLocallyControlled())
        return;

    AOZPlayer* LocalPlayer = Cast<AOZPlayer>(OwnerCharacter);
    if (!LocalPlayer)
        return;

    // 캐시된 연막 목록 사용
    // 로컬 플레이어가 속한 연막 찾기
    AOZSmokeArea* LocalPlayerSmoke = nullptr;
    for (AOZSmokeArea* Smoke : CachedSmokeAreas)
    {
        if (Smoke && Smoke->IsPlayerInside(LocalPlayer))
        {
            LocalPlayerSmoke = Smoke;
            break;
        }
    }

    // 캐시된 플레이어 목록 사용
    for (AOZPlayer* Player : CachedPlayers)
    {
        if (!Player || Player == LocalPlayer)
            continue;

        // 이 플레이어가 속한 연막 찾기
        AOZSmokeArea* PlayerSmoke = nullptr;
        for (AOZSmokeArea* Smoke : CachedSmokeAreas)
        {
            if (Smoke && Smoke->IsPlayerInside(Player))
            {
                PlayerSmoke = Smoke;
                break;
            }
        }

        float ForceHiddenValue = 0.f;

        if (LocalPlayerSmoke == nullptr && PlayerSmoke != nullptr)
        {
            // 로컬은 연막 밖, 타겟은 연막 안 → 숨김
            ForceHiddenValue = 1.f;
        }
        else if (LocalPlayerSmoke != nullptr && PlayerSmoke != nullptr && LocalPlayerSmoke != PlayerSmoke)
        {
            // 서로 다른 연막에 있음 → 숨김 (상대 연막 안을 볼 수 없음)
            ForceHiddenValue = 1.f;
        }

        // 부쉬에 의해 숨겨지는 경우
        if (BushHiddenPlayers.Contains(Player))
        {
            ForceHiddenValue = 1.f;
        }

        USkeletalMeshComponent* PlayerMesh = Player->GetMesh();
        if (PlayerMesh)
        {
            TArray<UMaterialInterface*> Materials = PlayerMesh->GetMaterials();
            for (int32 i = 0; i < Materials.Num(); i++)
            {
                UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(PlayerMesh->GetMaterial(i));
                if (MID)
                {
                    MID->SetScalarParameterValue("ForceHidden", ForceHiddenValue);
                }
            }
        }

        if (USkeletalMeshComponent* WeaponMesh = Player->GetWeaponMesh())
        {
            TArray<UMaterialInterface*> WeaponMaterials = WeaponMesh->GetMaterials();
            for (int32 i = 0; i < WeaponMaterials.Num(); i++)
            {
                UMaterialInstanceDynamic* WMI = Cast<UMaterialInstanceDynamic>(WeaponMesh->GetMaterial(i));
                if (WMI)
                {
                    WMI->SetScalarParameterValue("ForceHidden", ForceHiddenValue);
                }
            }
        }

        // 연막에 의해 숨겨지면 위젯도 숨김
        if (UOZWidgetComponent* WidgetComp = Player->StatusWidgetComp)
        {
            if (UUserWidget* Widget = WidgetComp->GetUserWidgetObject())
            {
                if (ForceHiddenValue > 0.f)
                {
                    Widget->SetVisibility(ESlateVisibility::Hidden);
                }
            }
        }

        // 연막/부쉬에 의해 숨겨지면 실드 이펙트도 숨김
        if (ForceHiddenValue > 0.f)
        {
            UpdateShieldEffectVisibility(Player, false);
        }

        //연막 부쉬에 의해 숨김처리 되는 플레이어 캐치
        if (ForceHiddenValue > 0.f)
        {
            HideObjectSet.Add(Player);
        }
    }
}


void UOZVisionComponent::UpdateBushHiddenSet()
{
    BushHiddenPlayers.Reset();

    AOZPlayer* LocalPlayer = Cast<AOZPlayer>(GetOwner());
    if (!LocalPlayer)
        return;

    AOZPlayerState* MyPS = LocalPlayer->GetPlayerState<AOZPlayerState>();
    if (!MyPS)
        return;

    const bool bMeInBush = MyPS->IsInBush();

    // 본인의 부쉬 상태 변경 시 시야 거리 재계산
    if (bOwnerInBush != bMeInBush)
    {
        bOwnerInBush = bMeInBush;
        RecalculateTraceRange();
    }

    const int32 MyBushID = MyPS->GetCurrentBushID();

    for (AOZPlayer* Player : CachedPlayers)
    {
        if (!Player || Player == LocalPlayer)
            continue;

        AOZPlayerState* TargetPS = Player->GetPlayerState<AOZPlayerState>();
        if (!TargetPS)
            continue;

        if (!TargetPS->IsInBush())
            continue;

        // 같은 부쉬에 있으면 상대가 보임
        if (bMeInBush && MyBushID != INDEX_NONE && MyBushID == TargetPS->GetCurrentBushID())
            continue;

        // revealed 상태면 보임
        UAbilitySystemComponent* TargetASC = TargetPS->GetAbilitySystemComponent();
        if (TargetASC && TargetASC->HasMatchingGameplayTag(OZGameplayTags::Player_State_Revealed))
            continue;

        BushHiddenPlayers.Add(Player);
    }
}

void UOZVisionComponent::SetDegreePerTrace(float NewDegree)
{
    DegreePerTrace = NewDegree;
}

void UOZVisionComponent::SetTraceRange(float NewRange)
{
    TraceRange = NewRange;

    UKismetMaterialLibrary::SetScalarParameterValue(GetOwner()->GetWorld(), TraceCollection, "TraceRange", TraceRange);
}

void UOZVisionComponent::PrePareTriangles()
{
    // 공통 유틸리티를 사용하여 삼각형 준비
    CanvasTriangles = FOZVisionUtility::PrepareTriangles(
        TraceResults,
        GetOwner()->GetActorLocation(),
        TraceRange
    );
}

void UOZVisionComponent::RecalculateNumTraces()
{
    float currentDegreePerTrace = bIsAiming ? AimingDegreePerTrace : BaseDegreePerTrace;
    NumTraces = CalculateNumTracesFromAngle(CachedViewingAngle, currentDegreePerTrace);
}

int32 UOZVisionComponent::CalculateNumTracesFromAngle(float Angle, float DegreeStep) const
{
    if (Angle <= 0.0f || DegreeStep <= 0.0f)
        return 1;

    int32 traces = FMath::CeilToInt(Angle / DegreeStep);

    // Ensure odd number for center ray
    if (traces % 2 == 0)
        traces++;

    return FMath::Max(1, traces);
}

void UOZVisionComponent::UpdateVisionParameters()
{
    if (AOZPlayer* OwnerPlayer = Cast<AOZPlayer>(GetOwner()))
    {
        if (AOZPlayerState* PS = OwnerPlayer->GetPlayerState<AOZPlayerState>())
        {
            if (PS->PlayerAttributes)
            {
                CachedViewingAngle = PS->PlayerAttributes->GetViewingAngle();
                RecalculateNumTraces();
            }
        }
    }
}

void UOZVisionComponent::TryInitializeViewingAngle()
{
    if (bViewingAngleInitialized)
        return;

    if (AOZPlayer* OwnerPlayer = Cast<AOZPlayer>(GetOwner()))
    {
        if (AOZPlayerState* PS = OwnerPlayer->GetPlayerState<AOZPlayerState>())
        {
            if (PS->PlayerAttributes)
            {
                CachedViewingAngle = PS->PlayerAttributes->GetViewingAngle();
                RecalculateNumTraces();
                bViewingAngleInitialized = true;
            }
        }
    }
}

void UOZVisionComponent::ClearVisionRenderTarget()
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
    {
        UE_LOG(LogTemp, Warning, TEXT("[VisionComp] ClearVisionRenderTarget: Not locally controlled"));
        return;
    }

    if (!DynamicRenderTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("[VisionComp] ClearVisionRenderTarget: No DynamicRenderTarget"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[VisionComp] Clearing RenderTarget for %s"), *GetNameSafe(GetOwner()));

    UKismetRenderingLibrary::ClearRenderTarget2D(
        GetOwner()->GetWorld(),
        DynamicRenderTarget,
        FLinearColor::Black
    );
}

void UOZVisionComponent::DrawTowerVisionAreas()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UOZVisionComponent::DrawTowerVisionAreas);

    if (!DynamicRenderTarget)
        return;

    AOZPlayer* OwnerPlayer = Cast<AOZPlayer>(GetOwner());
    if (!OwnerPlayer)
        return;

    // 타워 시야에서 현재 감지된 플레이어들
    TSet<AOZPlayer*> CurrentTowerDetectedPlayers;

    // 캐시된 타워 비전 영역 사용
    for (AOZTowerVisionArea* Area : CachedTowerVisionAreas)
    {
        if (!Area)
            continue;

        // 이 플레이어가 소유한 활성화된 영역만 그리기
        if (!Area->IsActivated() || Area->GetOwningPlayer() != OwnerPlayer)
            continue;

        // 타워 시야의 원형 삼각형 생성
        // 플레이어 위치 기준으로 상대 좌표 계산
        const FVector AreaLocation = Area->GetActorLocation();
        const FVector PlayerLocation = OwnerPlayer->GetActorLocation();
        const float AreaRadius = Area->GetVisionRadius();

        // 타워 시야 구역 내의 플레이어 감지 (캐시된 플레이어 목록 사용)
        for (AOZPlayer* OtherPlayer : CachedPlayers)
        {
            if (!OtherPlayer || OtherPlayer == OwnerPlayer)
                continue;

            // 2D 거리로 시야 구역 내 여부 판단
            float Dist2D = FVector::Dist2D(OtherPlayer->GetActorLocation(), AreaLocation);
            if (Dist2D <= AreaRadius)
            {
                CurrentTowerDetectedPlayers.Add(OtherPlayer);
            }
        }

        // RenderTarget 좌표 계산 (1800x1800, 중심 900,900)
        const float RTCenter = 900.f;
        const float Scale = RTCenter / TraceRange;

        FVector RelativePos = AreaLocation - PlayerLocation;
        FVector2D AreaCenterRT = FVector2D(RTCenter + RelativePos.X * Scale, RTCenter + RelativePos.Y * Scale);
        float ScaledRadius = AreaRadius * Scale;

        // 원형 삼각형 생성
        TArray<FCanvasUVTri> CircleTriangles;
        const int32 Segments = 32;

        for (int32 i = 0; i < Segments; i++)
        {
            float Angle1 = (static_cast<float>(i) / Segments) * 2.f * PI;
            float Angle2 = (static_cast<float>(i + 1) / Segments) * 2.f * PI;

            FVector2D P1 = AreaCenterRT + FVector2D(FMath::Cos(Angle1), FMath::Sin(Angle1)) * ScaledRadius;
            FVector2D P2 = AreaCenterRT + FVector2D(FMath::Cos(Angle2), FMath::Sin(Angle2)) * ScaledRadius;

            FCanvasUVTri Triangle;

            Triangle.V0_Pos = P1;
            Triangle.V0_UV = FVector2D(0.f, 0.f);
            Triangle.V0_Color = FLinearColor(1.f, 1.f, 1.f, 0.f);

            Triangle.V1_Pos = P2;
            Triangle.V1_UV = FVector2D(0.f, 0.f);
            Triangle.V1_Color = FLinearColor(1.f, 1.f, 1.f, 0.f);

            Triangle.V2_Pos = AreaCenterRT;
            Triangle.V2_UV = FVector2D(0.f, 0.f);
            Triangle.V2_Color = FLinearColor(1.f, 1.f, 1.f, 0.f);

            CircleTriangles.Add(Triangle);
        }

        // RenderTarget에 추가로 그리기 (bClearFirst = false)
        FOZVisionUtility::DrawToRenderTarget(
            GetWorld(),
            DynamicRenderTarget,
            CircleTriangles,
            false  // 기존 그림 위에 추가
        );
    }

    // 타워 시야에서 감지된 플레이어들의 위젯 가시성 업데이트
    UpdateTowerWidgetVisibility(CurrentTowerDetectedPlayers);
}

void UOZVisionComponent::UpdateTowerWidgetVisibility(const TSet<AOZPlayer*>& TowerVisiblePlayers)
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
        return;

    if (!OwnerCharacter->IsLocallyControlled())
        return;

    TowerVisionSet.Empty();

    for (AOZPlayer* Player : TowerVisiblePlayers)
    {
        if (!Player)
            continue;

        TowerVisionSet.Add(Player);

        if (UOZWidgetComponent* WidgetComp = Player->StatusWidgetComp)
        {
            if (UUserWidget* Widget = WidgetComp->GetUserWidgetObject())
            {
                Widget->SetVisibility(ESlateVisibility::Visible);
            }
        }

        // 타워 시야에 들어온 플레이어의 실드 이펙트 보이기
        UpdateShieldEffectVisibility(Player, true);
    }

    for (AOZPlayer* Player : PreviousTowerDetectedPlayers)
    {
        if (!Player)
            continue;

        if (TowerVisiblePlayers.Contains(Player))
            continue;

        if (PreviousDetectedPlayers.Contains(Player))
            continue;

        if (Player == OwnerCharacter)
            continue;

        if (UOZWidgetComponent* WidgetComp = Player->StatusWidgetComp)
        {
            if (UUserWidget* Widget = WidgetComp->GetUserWidgetObject())
            {
                Widget->SetVisibility(ESlateVisibility::Hidden);
            }
        }

        // 타워 시야에서 나간 플레이어의 실드 이펙트 숨기기
        UpdateShieldEffectVisibility(Player, false);
    }

    PreviousTowerDetectedPlayers = TowerVisiblePlayers;

}

void UOZVisionComponent::RefreshActorCaches()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UOZVisionComponent::RefreshActorCaches);

    RefreshPlayerCache();
    RefreshSmokeCache();
    RefreshTowerVisionCache();
}

void UOZVisionComponent::RefreshPlayerCache()
{
    CachedPlayers.Reset();

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AOZPlayer::StaticClass(), FoundActors);

    CachedPlayers.Reserve(FoundActors.Num());
    for (AActor* Actor : FoundActors)
    {
        if (AOZPlayer* Player = Cast<AOZPlayer>(Actor))
        {
            CachedPlayers.Add(Player);
        }
    }
}

void UOZVisionComponent::RefreshSmokeCache()
{
    CachedSmokeAreas.Reset();

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AOZSmokeArea::StaticClass(), FoundActors);

    CachedSmokeAreas.Reserve(FoundActors.Num());
    for (AActor* Actor : FoundActors)
    {
        if (AOZSmokeArea* Smoke = Cast<AOZSmokeArea>(Actor))
        {
            CachedSmokeAreas.Add(Smoke);
        }
    }
}

void UOZVisionComponent::RefreshTowerVisionCache()
{
    CachedTowerVisionAreas.Reset();

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AOZTowerVisionArea::StaticClass(), FoundActors);

    CachedTowerVisionAreas.Reserve(FoundActors.Num());
    for (AActor* Actor : FoundActors)
    {
        if (AOZTowerVisionArea* Area = Cast<AOZTowerVisionArea>(Actor))
        {
            CachedTowerVisionAreas.Add(Area);
        }
    }
}

void UOZVisionComponent::DisableVisionAndShowAllPlayers()
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
        return;

    UE_LOG(LogTemp, Log, TEXT("[VisionComp] DisableVisionAndShowAllPlayers for %s"), *GetNameSafe(GetOwner()));

    IsActivate = false;

    // PostProcessVolume의 Vision 효과 비활성화
    if (CachedPostProcessVolume && VisionMaterialInstance)
    {
        CachedPostProcessVolume->Settings.RemoveBlendable(VisionMaterialInstance);
    }

    // RenderTarget을 흰색으로 클리어 (모든 위치에서 시야 통과)
    if (DynamicRenderTarget)
    {
        UKismetRenderingLibrary::ClearRenderTarget2D(
            GetOwner()->GetWorld(),
            DynamicRenderTarget,
            FLinearColor::White  // 흰색으로 클리어 = 모든 캐릭터 보임
        );
    }

    // 최신 플레이어 목록 가져오기
    RefreshPlayerCache();

    for (AOZPlayer* Player : CachedPlayers)
    {
        if (!Player)
            continue;

        // 위젯 표시
        if (UOZWidgetComponent* WidgetComp = Player->StatusWidgetComp)
        {
            if (UUserWidget* Widget = WidgetComp->GetUserWidgetObject())
            {
                Widget->SetVisibility(ESlateVisibility::Visible);
            }
        }

        // 실드 이펙트 표시
        UpdateShieldEffectVisibility(Player, true);

        USkeletalMeshComponent* PlayerMesh = Player->GetMesh();
        if (PlayerMesh)
        {
            // 메시 가시성 강제 활성화
            PlayerMesh->SetVisibility(true, true);
            PlayerMesh->SetHiddenInGame(false, true);

            TArray<UMaterialInterface*> Materials = PlayerMesh->GetMaterials();
            for (int32 i = 0; i < Materials.Num(); i++)
            {
                UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(PlayerMesh->GetMaterial(i));
                if (MID)
                {
                    MID->SetScalarParameterValue("ForceHidden", 0.f);
                    MID->SetScalarParameterValue("DisableVision", 1.f); 

                    // VisionTexture를 완전히 흰색으로 설정 (모든 픽셀이 1.0)
                    if (DynamicRenderTarget)
                    {
                        MID->SetTextureParameterValue("VisionTexture", DynamicRenderTarget);
                    }
                }
            }
        }

        if (USkeletalMeshComponent* WeaponMesh = Player->GetWeaponMesh())
        {
            WeaponMesh->SetVisibility(true, true);
            WeaponMesh->SetHiddenInGame(false, true);

            TArray<UMaterialInterface*> WeaponMaterials = WeaponMesh->GetMaterials();
            for (int32 i = 0; i < WeaponMaterials.Num(); i++)
            {
                UMaterialInstanceDynamic* WMI = Cast<UMaterialInstanceDynamic>(WeaponMesh->GetMaterial(i));
                if (WMI)
                {
                    WMI->SetScalarParameterValue("ForceHidden", 0.f);
                    WMI->SetScalarParameterValue("DisableVision", 1.f);  

                    if (DynamicRenderTarget)
                    {
                        WMI->SetTextureParameterValue("VisionTexture", DynamicRenderTarget);
                    }
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[VisionComp] Forced visibility for all %d players"), CachedPlayers.Num());
}

void UOZVisionComponent::EnableVisionSystem()
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled())
        return;

    UE_LOG(LogTemp, Log, TEXT("[VisionComp] EnableVisionSystem for %s"), *GetNameSafe(GetOwner()));

    IsActivate = true;

    if (!bIsVisionInitialized)
    {
        InitializeVisionSystem();
    }

    // PostProcessVolume에 Vision 효과 다시 추가
    if (CachedPostProcessVolume && VisionMaterialInstance)
    {
        CachedPostProcessVolume->AddOrUpdateBlendable(VisionMaterialInstance, 1.0f);
    }

    // RenderTarget을 검은색으로 클리어 (정상 시야각 적용)
    if (DynamicRenderTarget)
    {
        UKismetRenderingLibrary::ClearRenderTarget2D(
            GetOwner()->GetWorld(),
            DynamicRenderTarget,
            FLinearColor::Black
        );
    }

    // 모든 플레이어의 DisableVision 파라미터를 0으로 되돌림 (정상 시야각 적용)
    RefreshPlayerCache();
    for (AOZPlayer* Player : CachedPlayers)
    {
        if (!Player)
            continue;

        USkeletalMeshComponent* PlayerMesh = Player->GetMesh();
        if (PlayerMesh)
        {
            TArray<UMaterialInterface*> Materials = PlayerMesh->GetMaterials();
            for (int32 i = 0; i < Materials.Num(); i++)
            {
                UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(PlayerMesh->GetMaterial(i));
                if (MID)
                {
                    MID->SetScalarParameterValue("DisableVision", 0.f);  // 시야각 계산 복원
                }
            }
        }

        if (USkeletalMeshComponent* WeaponMesh = Player->GetWeaponMesh())
        {
            TArray<UMaterialInterface*> WeaponMaterials = WeaponMesh->GetMaterials();
            for (int32 i = 0; i < WeaponMaterials.Num(); i++)
            {
                UMaterialInstanceDynamic* WMI = Cast<UMaterialInstanceDynamic>(WeaponMesh->GetMaterial(i));
                if (WMI)
                {
                    WMI->SetScalarParameterValue("DisableVision", 0.f);  // 시야각 계산 복원
                }
            }
        }
    }
}

void UOZVisionComponent::UpdateShieldEffectVisibility(AOZPlayer* Player, bool bVisible)
{
    if (!Player)
        return;

    if (UNiagaraComponent* ShieldEffect = Player->ShieldEffectComponent)
    {
        // 시야에 보이더라도 실드가 없으면 보이지 않음 (Multicast로 동기화된 플래그 사용)
        bool bShouldBeVisible = bVisible && Player->bShieldEffectActive;
        ShieldEffect->SetVisibility(bShouldBeVisible);
    }
}
