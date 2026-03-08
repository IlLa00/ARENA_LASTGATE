# ARENA LASTGATE 
## 📋 프로젝트 개요    
본 프로젝트는 인하대학교 인공지능프로그래밍 게임개발 교육의 협력사 **Kog**의 리소스를 사용해 제작되었습니다.      
**ARENA LASTGATE**는 언리얼 엔진 5.5 기반의 **최대 4인 멀티플레이** 배틀 로얄 슈팅 게임입니다.      
플레이어는 라운드마다 무기 개조, 캐릭터 강화, 상점 구매 등을 통해 전략적으로 성장하며, 최후의 1인이 되기 위해 경쟁합니다.      

### 프로젝트 정보
- **개발 엔진**: Unreal Engine 5.5
- **개발 언어**: C++, Blueprint
- **개발 기간**: 3개월
- **팀 구성**: 기획 5명, 프로그래머 4명, 그래픽 6명     
- **담당 역할**: 프로그래머 팀장
- **플랫폼**: PC

### 협업 도구
- **프로젝트 관리**: Jira
- **문서화**: Notion 
- **버전 관리**: SVN

### 플레이 영상 링크
https://youtu.be/Np2qcAdtUEQ      

---

## 담당 시스템
1. [실시간 시야 시스템](#1-실시간-시야-시스템)
2. [GAS 기반 개조 및 모듈 시스템](#2-gas-기반-개조-및-모듈-시스템)
3. [서버 권한 상점 시스템](#3-서버-권한-상점-시스템)
4. [인벤토리 시스템](#4-인벤토리-시스템)
5. [Google Sheets 데이터 파이프라인](#5-google-sheets-데이터-파이프라인)
6. [그 외 기술](#6-그-외-기술)

---

## 1. 실시간 시야 시스템

![bandicam 2026-02-12 10-13-45-501](https://github.com/user-attachments/assets/c24eda4e-cc58-41e1-8ab2-6a89d834fe8e)

### LineTrace 기반 원뿔 시야 계산

<details>
<summary><b>OZVisionUtility::CreateVisionCone</b></summary>

```cpp
TArray<FVector> FOZVisionUtility::CreateVisionCone(
	UWorld* World,
	const FVector& Origin,
	const FVector& ForwardVector,
	float Range,
	int32 NumTraces,
	float DegreePerTrace,
	AActor* IgnoreActor,
	TSet<AActor*>* OutDetectedMinimapVisibleObjects,
	TSet<AOZPlayer*>* OutDetectedPlayers)
{
	TArray<FVector> TraceResults;

	if (!World)
		return TraceResults;

	TraceResults.Empty();

	if (OutDetectedPlayers)
		OutDetectedPlayers->Empty();

	float StartAngle = -(NumTraces * DegreePerTrace) / 2.0f;

	for (int32 i = 0; i < NumTraces; i++)
	{
		float CurrentAngle = StartAngle + (DegreePerTrace * i);
		FRotator Rotation = FRotator(0.0f, CurrentAngle, 0.0f);
		FVector RotatedDirection = Rotation.RotateVector(ForwardVector);
		FVector Start = Origin;
		FVector End = Origin + (RotatedDirection * Range);

		TArray<FHitResult> HitResults;
		FCollisionQueryParams QueryParams;
		QueryParams.bTraceComplex = false;

		if (IgnoreActor)
			QueryParams.AddIgnoredActor(IgnoreActor);

		FCollisionObjectQueryParams ObjectParams;
		ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

		bool bHit = World->LineTraceMultiByObjectType(
			HitResults, Start, End, ObjectParams, QueryParams);

		FVector TraceEndPoint = End + (RotatedDirection * 10.0f);
		bool bFoundWorldStatic = false;

		if (bHit && HitResults.Num() > 0)
		{
			for (const FHitResult& Hit : HitResults)
			{
				if (Hit.Component.IsValid() &&
					Hit.Component->GetCollisionObjectType() == ECC_WorldStatic)
				{
					TraceEndPoint = Hit.Location + (RotatedDirection * 10.0f);
					bFoundWorldStatic = true;
					break;
				}
			}

			if (OutDetectedMinimapVisibleObjects)
			{
				for (const FHitResult& Hit : HitResults)
				{
					AActor* hittedActor = Hit.GetActor();
					if (hittedActor && hittedActor->GetClass()->ImplementsInterface(
						UOZIMinimapVisibleActor::StaticClass()))
					{
						if (!bFoundWorldStatic || Hit.Distance < (TraceEndPoint - Start).Size())
							OutDetectedMinimapVisibleObjects->Add(hittedActor);
					}
				}
			}

			if (OutDetectedPlayers)
			{
				for (const FHitResult& Hit : HitResults)
				{
					if (AOZPlayer* Player = Cast<AOZPlayer>(Hit.GetActor()))
					{
						if (!bFoundWorldStatic || Hit.Distance < (TraceEndPoint - Start).Size())
							OutDetectedPlayers->Add(Player);
					}
				}
			}
		}
		TraceResults.Add(TraceEndPoint);
	}
	return TraceResults;
}
```
</details>

<details>
<summary><b>OZVisionUtility::PrepareTriangles - 렌더타겟 삼각형 생성</b></summary>

```cpp
TArray<FCanvasUVTri> FOZVisionUtility::PrepareTriangles(
	const TArray<FVector>& TraceResults,
	const FVector& CenterLocation,
	float TraceRange)
{
	TArray<FCanvasUVTri> CanvasTriangles;
	CanvasTriangles.Empty();

	if (TraceResults.Num() < 2)
		return CanvasTriangles;

	FVector RenderTargetCenter = FVector(900.0, 900.0, 0.0);
	float Scale = 900.f / TraceRange;

	for (int32 i = 0; i < TraceResults.Num() - 1; ++i)
	{
		FVector Point0 = TraceResults[i];
		FVector RelativePoint0 = Point0 - CenterLocation;
		FVector ScaledPoint0 = RelativePoint0 * Scale;
		FVector FinalPoint0 = ScaledPoint0 + RenderTargetCenter;
		FVector2D V0_Pos = FVector2D(FinalPoint0.X, FinalPoint0.Y);

		FVector Point1 = TraceResults[i + 1];
		FVector RelativePoint1 = Point1 - CenterLocation;
		FVector ScaledPoint1 = RelativePoint1 * Scale;
		FVector FinalPoint1 = ScaledPoint1 + RenderTargetCenter;
		FVector2D V1_Pos = FVector2D(FinalPoint1.X, FinalPoint1.Y);

		FVector2D V2_Pos = FVector2D(RenderTargetCenter.X, RenderTargetCenter.Y);

		FCanvasUVTri Triangle;
		Triangle.V0_Pos = V0_Pos;
		Triangle.V0_UV = FVector2D(0.0f, 0.0f);
		Triangle.V0_Color = FLinearColor(1.0f, 1.0f, 1.0f, 0.0f);
		Triangle.V1_Pos = V1_Pos;
		Triangle.V1_UV = FVector2D(0.0f, 0.0f);
		Triangle.V1_Color = FLinearColor(1.0f, 1.0f, 1.0f, 0.0f);
		Triangle.V2_Pos = V2_Pos;
		Triangle.V2_UV = FVector2D(0.0f, 0.0f);
		Triangle.V2_Color = FLinearColor(1.0f, 1.0f, 1.0f, 0.0f);
		CanvasTriangles.Add(Triangle);
	}
	return CanvasTriangles;
}
```
</details>

### 커스텀 머터리얼 함수 & 렌더타겟

<img width="1879" alt="image" src="https://github.com/user-attachments/assets/35d6504a-16b6-4c01-bfaa-8b572f787f1a" />

<img width="503" alt="image" src="https://github.com/user-attachments/assets/c48e9ccb-995b-4908-a503-10e150931f3b" />

> 머터리얼 함수 적용 결과

<img width="1063" alt="image" src="https://github.com/user-attachments/assets/4db6223b-7787-4c5e-bdf9-11aa9d9a778b" />

### 포스트 프로세싱

<img width="1284" alt="image" src="https://github.com/user-attachments/assets/d11b0715-87d8-489f-9ffa-71cc913cda7a" />

![bandicam 2026-02-12 12-08-56-396](https://github.com/user-attachments/assets/67bab719-1c0e-48b9-9b4d-952a8fe0018b)
> 연막탄 내 적군은 보이지 않습니다.

![bandicam 2026-02-12 12-08-56-396 (1)](https://github.com/user-attachments/assets/83e9c266-8989-4310-986c-6302f0c3251b)
> 섬광탄 피격 시 시야 차단

![bandicam 2026-02-12 12-19-37-339](https://github.com/user-attachments/assets/53403a85-d421-4dd0-84a4-3161434ae99b)
> 타워 활성화 시 특정 지역 시야 확보 + 미니맵 반영

<details>
<summary><b>OZVisionComponent::LosVisionSystem</b></summary>

```cpp
void UOZVisionComponent::LosVisionSystem()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UOZVisionComponent::LosVisionSystem);

    if (!bIsVisionInitialized)
    {
        InitializeVisionSystem();
        if (!bIsVisionInitialized) return;
    }

    if (!DynamicRenderTarget)    { bIsVisionInitialized = false; return; }
    if (!VisionMaterialInstance) { bIsVisionInitialized = false; return; }
    if (!CachedPostProcessVolume) { bIsVisionInitialized = false; return; }

    UKismetMaterialLibrary::SetVectorParameterValue(
        GetOwner()->GetWorld(), PlayerCollection,
        "PlayerPosition", FLinearColor(GetOwner()->GetActorLocation()));

    SetupVisionTextureForRemotePlayers();

    // 섬광탄 피격: RenderTarget BLACK으로 클리어
    if (bFlashbanged)
    {
        UKismetRenderingLibrary::ClearRenderTarget2D(
            GetOwner()->GetWorld(), DynamicRenderTarget, FLinearColor::Black);

        UpdateBushHiddenSet();

        ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
        for (AOZPlayer* Player : CachedPlayers)
        {
            if (Player && Player != OwnerCharacter)
            {
                if (UOZWidgetComponent* WidgetComp = Player->StatusWidgetComp)
                {
                    if (UUserWidget* Widget = WidgetComp->GetUserWidgetObject())
                        Widget->SetVisibility(ESlateVisibility::Hidden);
                }
                UpdateShieldEffectVisibility(Player, false);
            }
        }

        PreviousDetectedPlayers.Empty();
        PreviousTowerDetectedPlayers.Empty();

        TSet<AOZPlayer*> EmptySet;
        UpdateSmokeAndBushVisibility(EmptySet);
        UpdateMinimapVisibility();
        return;
    }

    CreateCone();
    PrePareTriangles();

    FOZVisionUtility::DrawToRenderTarget(
        GetOwner()->GetWorld(), DynamicRenderTarget, CanvasTriangles, true);

    DrawTowerVisionAreas();
    UpdateMinimapVisibility();
}
```
</details>

### 최적화 - 에셋 캐싱

> Before: 6,283회 / 571ms
<img width="645" alt="Before" src="https://github.com/user-attachments/assets/f292b53b-4721-428a-9e79-88be6d142c36" />

> After: 2,602회 / 324ms
<img width="649" alt="After" src="https://github.com/user-attachments/assets/17a77a71-fead-49ba-a221-f3d7e7115c8d" />

---

## 2. GAS 기반 개조 및 모듈 시스템

![bandicam 2026-02-12 11-27-35-314](https://github.com/user-attachments/assets/c8b0bb33-8b5f-4ff4-96e4-a4137e987f88)

### 데이터 구조체

<details>
<summary><b>FOZConvertData - 개조 데이터</b></summary>

```cpp
UENUM(BlueprintType)
enum class EConvertGrade : uint8
{
	None       UMETA(DisplayName = "None"),
	Rare       UMETA(DisplayName = "Rare"),
	Unique     UMETA(DisplayName = "Unique"),
	Legendary  UMETA(DisplayName = "Legendary")
};

USTRUCT(BlueprintType)
struct FOZConvertData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ID;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EConvertGrade Grade;

	// Damage
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 DamageAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float DamageMulti;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float DamageAmpMulti;
	// Attack Delay
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float AtkDelayMulti;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float AtkDelayAmpMulti;
	// Heat System
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 MaxHeatAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float CoolingAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float HeatCoefficientMulti;
	// Spread & Noise
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float SpreadAngleMulti;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float AmpSpreadAngleMulti;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float NoiseMulti;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float AmpNoiseMulti;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float NoiseMaxRangeMulti;
	// Range & Projectile
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float RangeAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float RangeMulti;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float ProjectileSpeedAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float ProjectileSpeedMulti;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float ProjectileSizeAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 ProjectilesPershotAdd;
	// Combat Effects
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 KnockbackPowerAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float BaseKnockbackPowerMulti;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 RicochetConutAdd;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float ExplosionRadiusAdd;

	UPROPERTY(EditAnywhere, BlueprintReadOnly) FText tooltip_text;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FText description;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<class UGameplayEffect> Convert_Asset;
};
```
</details>

<details>
<summary><b>FOZModuleData - 모듈 데이터</b></summary>

```cpp
USTRUCT(BlueprintType)
struct FOZModuleData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Module_ID = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FText name;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UTexture2D> Icon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float ValueMin;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float ValueMax;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float ValueStep;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float ScaleMulti;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float weight;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FText description;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<class UGameplayEffect> Module_Asset;

	// 런타임 캐싱 값
	UPROPERTY(BlueprintReadOnly) float FinalValue = 0.0f;
	UPROPERTY(BlueprintReadOnly) float ScaledMinValue = 0.0f;
	UPROPERTY(BlueprintReadOnly) float ScaledMaxValue = 0.0f;
};
```
</details>

### Custom MMC - 데미지 수식

<img width="1070" alt="image" src="https://github.com/user-attachments/assets/c6f63951-a168-4182-be78-6e3342706761" />

> `BaseDamage * (1 + DamageMulti) * (1 + DamageAmpMulti)`

<details>
<summary><b>OZDamageMMC</b></summary>

```cpp
UOZDamageMMC::UOZDamageMMC()
{
	BaseDamageDef = FGameplayEffectAttributeCaptureDefinition(
		UOZWeaponAttributeSet::GetBaseDamageAttribute(),
		EGameplayEffectAttributeCaptureSource::Source, false);

	DamageMultiDef = FGameplayEffectAttributeCaptureDefinition(
		UOZWeaponAttributeSet::GetDamageMultiAttribute(),
		EGameplayEffectAttributeCaptureSource::Source, false);

	DamageAmpMultiDef = FGameplayEffectAttributeCaptureDefinition(
		UOZWeaponAttributeSet::GetDamageAmpMultiAttribute(),
		EGameplayEffectAttributeCaptureSource::Source, false);

	RelevantAttributesToCapture.Add(BaseDamageDef);
	RelevantAttributesToCapture.Add(DamageMultiDef);
	RelevantAttributesToCapture.Add(DamageAmpMultiDef);
}

float UOZDamageMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	FAggregatorEvaluateParameters EvaluationParameters;

	float BaseValue = 0.0f;
	float MultiValue = 0.0f;
	float AmpMultiValue = 0.0f;

	GetCapturedAttributeMagnitude(BaseDamageDef, Spec, EvaluationParameters, BaseValue);
	GetCapturedAttributeMagnitude(DamageMultiDef, Spec, EvaluationParameters, MultiValue);
	GetCapturedAttributeMagnitude(DamageAmpMultiDef, Spec, EvaluationParameters, AmpMultiValue);

	// 기획 수식: BaseDamage * (1 + DamageMulti) * (1 + DamageAmpMulti)
	float Result = BaseValue * (1.0f + MultiValue) * (1.0f + AmpMultiValue);
	return Result;
}
```
</details>

### SetByCaller를 활용한 개조 적용

<details>
<summary><b>OZConvertSubsystem::ApplyConvertToSelf</b></summary>

```cpp
FActiveGameplayEffectHandle UOZConvertSubsystem::ApplyConvertToSelf(
	int32 ConvertID, UAbilitySystemComponent* TargetASC)
{
	if (!TargetASC) return FActiveGameplayEffectHandle();

	// GameplayEffect는 서버에서만 적용
	if (!TargetASC->IsOwnerActorAuthoritative())
		return FActiveGameplayEffectHandle();

	FOZConvertData* ConvertData = FindConvert(ConvertID);
	if (!ConvertData) return FActiveGameplayEffectHandle();

	TSubclassOf<UGameplayEffect> EffectClass = ConvertData->Convert_Asset.Get();
	if (!EffectClass) return FActiveGameplayEffectHandle();

	FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
	Context.AddSourceObject(this);
	FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(EffectClass, 1.0f, Context);
	if (!SpecHandle.IsValid()) return FActiveGameplayEffectHandle();

	// SetByCallerMagnitude로 DataTable 수치를 동적 주입
	if (ConvertData->DamageAdd != 0)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.BaseDamage"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, static_cast<float>(ConvertData->DamageAdd));
	}
	if (ConvertData->DamageMulti != 0.0f)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.DamageMulti"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, ConvertData->DamageMulti);
	}
	if (ConvertData->DamageAmpMulti != 0.0f)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.DamageAmpMulti"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, ConvertData->DamageAmpMulti);
	}
	if (ConvertData->AtkDelayMulti != 0.0f)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.AtkDelayMulti"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, ConvertData->AtkDelayMulti);
	}
	if (ConvertData->RicochetConutAdd != 0)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.MaxRicochetCount"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, static_cast<float>(ConvertData->RicochetConutAdd));

		FGameplayTag RicochetTag = FGameplayTag::RequestGameplayTag(FName("Weapon.State.CanRicochet"));
		TargetASC->AddLooseGameplayTag(RicochetTag);
	}
	// ... (SpreadAngle, Noise, Range, Projectile, Knockback, Explosion 등 동일 패턴)

	FActiveGameplayEffectHandle Handle = TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	return Handle;
}
```
</details>

### 모듈 - 라운드 스케일링 계산

<details>
<summary><b>OZModuleSubsystem::CalculateFinalValue</b></summary>

```cpp
float UOZModuleSubsystem::CalculateFinalValue(const FOZModuleData& ModuleData, int32 CurrentRound) const
{
	int32 X = CurrentRound / 2;
	float FinalScaleMulti = 1.0f + (X * ModuleData.ScaleMulti);

	float RawMin = FinalScaleMulti * ModuleData.ValueMin;
	float RawMax = FinalScaleMulti * ModuleData.ValueMax;

	float ValueStep = ModuleData.ValueStep;
	if (ValueStep <= 0.f)
		ValueStep = 1.f;

	float CurrentValueMin = FMath::RoundToFloat(RawMin / ValueStep) * ValueStep;
	float CurrentValueMax = FMath::RoundToFloat(RawMax / ValueStep) * ValueStep;

	int32 StepCount = FMath::RoundToInt((CurrentValueMax - CurrentValueMin) / ValueStep);
	int32 RandomStepIndex = FMath::RandRange(0, StepCount);
	float FinalValue = CurrentValueMin + (RandomStepIndex * ValueStep);

	return FinalValue;
}
```
</details>

<details>
<summary><b>OZModuleSubsystem::ApplyModuleToSelf</b></summary>

```cpp
FActiveGameplayEffectHandle UOZModuleSubsystem::ApplyModuleToSelf(
	int32 ModuleID, UAbilitySystemComponent* TargetASC, int32 CurrentRound)
{
	if (!TargetASC) return FActiveGameplayEffectHandle();
	if (!TargetASC->IsOwnerActorAuthoritative()) return FActiveGameplayEffectHandle();

	FOZModuleData* ModuleData = FindModule(ModuleID);
	if (!ModuleData) return FActiveGameplayEffectHandle();

	TSubclassOf<UGameplayEffect> EffectClass = ModuleData->Module_Asset.Get();
	if (!EffectClass) return FActiveGameplayEffectHandle();

	FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(EffectClass, 1.0f, Context);
	if (!SpecHandle.IsValid()) return FActiveGameplayEffectHandle();

	// 캐시된 FinalValue 사용
	float FinalValue = GetCachedFinalValue(ModuleID);

	FGameplayTag DataTag = GetTagFromAttributeName(ModuleData->Module_ID);
	if (DataTag.IsValid())
		SpecHandle.Data->SetSetByCallerMagnitude(DataTag, FinalValue);
	else
		return FActiveGameplayEffectHandle();

	return TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}
```
</details>

<details>
<summary><b>OZModuleSubsystem::GetTagFromAttributeName - 모듈 ID → GameplayTag 매핑</b></summary>

```cpp
FGameplayTag UOZModuleSubsystem::GetTagFromAttributeName(const int32 ID) const
{
	FString TagName;
	switch (ID)
	{
	case 40101: TagName = TEXT("Module.HP");          break;
	case 40102: TagName = TEXT("Module.Stamina");     break;
	case 40103: TagName = TEXT("Module.WalkSpeed");   break;
	case 40104: TagName = TEXT("Module.SprintSpeed"); break;
	case 40105: TagName = TEXT("Module.EVLDistance");  break;
	case 40106: TagName = TEXT("Module.Shield");      break;
	case 40107: TagName = TEXT("Module.Armor");       break;
	default:    return FGameplayTag();
	}
	return FGameplayTag::RequestGameplayTag(FName(*TagName), false);
}
```
</details>

---

## 3. 서버 권한 상점 시스템

![bandicam 2026-02-12 11-34-20-944](https://github.com/user-attachments/assets/b607b85a-87f7-4cbc-8197-4d663e3b2f0e)
![bandicam 2026-02-12 11-34-34-577](https://github.com/user-attachments/assets/9b37ae33-e4ff-40c2-b672-2dea35530465)

<details>
<summary><b>PlayerController → ShopManager Server RPC 흐름</b></summary>

```cpp
// 1단계: PlayerController — 클라이언트 요청 진입점
void AOZPlayerController::Server_PurchaseItem_Implementation(
	int32 ItemID, EOZItemType ItemType, int32 Quantity)
{
	if (!HasAuthority()) return;  // 서버 권한 검증

	AOZInGameGameState* GS = GetWorld()->GetGameState<AOZInGameGameState>();
	AOZShopManager* ShopManager = GS->GetShopManager();
	AOZPlayerState* OZPlayerState = GetPlayerState<AOZPlayerState>();  // 서버가 직접 조회

	ShopManager->Server_PurchaseItem(OZPlayerState, ItemID, ItemType, Quantity);
}

// 2단계: ShopManager — 서버에서만 실행되는 거래 로직
void AOZShopManager::Server_PurchaseItem_Implementation(
	AOZPlayerState* BuyerPS, int32 ItemID, EOZItemType ItemType, int32 Quantity)
{
	UOZInventoryComponent* InvComp = BuyerPS->GetInventoryComponent();
	int32 Price = GetItemPrice(ItemID, ItemType);      // DataTable에서 조회
	int32 MaxStack = GetItemMaxStack(ItemID, ItemType);

	int32 CurrentQuantity = InvComp->GetItemQuantity(ItemID, ItemType);
	int32 MaxPurchasable = MaxStack - CurrentQuantity;
	int32 ActualQuantity = FMath::Min(Quantity, MaxPurchasable);  // 수량 클램핑

	int32 TotalPrice = Price * ActualQuantity;
	if (BuyerPS->OwningScraps < TotalPrice) return;  // 재화 부족

	if (CurrentQuantity == 0 && !InvComp->HasEmptySlot()) return;  // 슬롯 검증

	if (!InvComp->AddItem(ItemID, ItemType, ActualQuantity)) return;  // 추가 실패 시 중단

	BuyerPS->OwningScraps -= TotalPrice;  // 성공 후에만 차감
	OnScrapChanged.Broadcast(-TotalPrice);
}
```
</details>

---

## 4. 인벤토리 시스템

### 경량 구조체 + 네트워크 복제

<details>
<summary><b>FOZInventorySlot & OZInventoryComponent</b></summary>

```cpp
USTRUCT(BlueprintType)
struct FOZInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite) int32 ItemID = 0;
	UPROPERTY(BlueprintReadWrite) EOZItemType ItemType = EOZItemType::None;
	UPROPERTY(BlueprintReadWrite) int32 Quantity = 0;

	bool IsEmpty() const { return ItemID == 0 || Quantity <= 0; }
	void Clear()
	{
		ItemID = 0;
		ItemType = EOZItemType::None;
		Quantity = 0;
	}
};

// ---- OZInventoryComponent ----

void UOZInventoryComponent::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UOZInventoryComponent, ItemSlots);
}

void UOZInventoryComponent::OnRep_ItemSlots()
{
	OnInventoryUpdated.Broadcast();
}

bool UOZInventoryComponent::AddItem(int32 ItemID, EOZItemType ItemType, int32 Amount)
{
	// 1단계: 스택 탐색 — 동일 아이템이 있으면 수량만 증가
	int32 ExistingSlot = FindSlotWithItem(ItemID, ItemType);
	if (ExistingSlot != INDEX_NONE)
	{
		ItemSlots[ExistingSlot].Quantity += Amount;
		OnRep_ItemSlots();
		return true;
	}

	// 2단계: 종류 제한 검증 — 배틀아이템 3종 미만인지 확인
	if (ItemType == EOZItemType::Battle)
	{
		int32 UniqueBattleItemCount = GetUniqueBattleItemCount();
		if (UniqueBattleItemCount >= 3)
			return false;
	}

	// 3단계: 빈 슬롯 배치
	int32 EmptySlot = FindEmptySlot();
	if (EmptySlot != INDEX_NONE)
	{
		ItemSlots[EmptySlot].ItemID = ItemID;
		ItemSlots[EmptySlot].ItemType = ItemType;
		ItemSlots[EmptySlot].Quantity = Amount;
		OnRep_ItemSlots();
		return true;
	}

	return false;
}
```
</details>

---

## 5. Google Sheets 데이터 파이프라인

```
Google Sheets (기획자 수치 관리)
    ↓ CSV Export
CSV Files (Content/Data/*.csv)
    ↓ Unreal Import
DataTable Assets (DT_*.uasset)
    ↓ Subsystem Initialize
In-Memory Cache
  ├─ TArray (순회용)
  ├─ TMap<ID, Data*> (O(1) 조회용)
  ├─ TMap<ID, GameplayEffect> (GC 방지 + 중복 로드 방지)
  └─ TMap<ID, Texture2D> (아이콘 캐시)
    ↓ Runtime
GAS (SetByCallerMagnitude → AttributeSet)
```

<img width="1900" alt="image" src="https://github.com/user-attachments/assets/503707c0-3bed-413b-ba9d-d204c1dfa79b" />

<details>
<summary><b>OZConvertSubsystem::Initialize - DataTable 캐싱 & TSoftObjectPtr 강참조</b></summary>

```cpp
void UOZConvertSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (ConvertDataTableRef.IsNull())
		ConvertDataTableRef = FSoftObjectPath(TEXT("/Game/Data/DT_Convert.DT_Convert"));

	ConvertDataTable = ConvertDataTableRef.LoadSynchronous();
	if (!ConvertDataTable) return;

	TArray<FName> RowNames = ConvertDataTable->GetRowNames();
	ConvertCache.Reserve(RowNames.Num());

	for (const FName& RowName : RowNames)
	{
		FOZConvertData* Data = ConvertDataTable->FindRow<FOZConvertData>(RowName, TEXT(""));
		if (Data)
		{
			// 캐시에 복사
			int32 Index = ConvertCache.Add(*Data);
			FOZConvertData& CachedData = ConvertCache[Index];
			ConvertMap.Add(CachedData.ID, &CachedData);

			// GameplayEffect 로드 → UPROPERTY TMap에 강참조 (GC 방지)
			if (!CachedData.Convert_Asset.IsNull())
			{
				UClass* LoadedClass = CachedData.Convert_Asset.LoadSynchronous();
				if (LoadedClass)
					CachedEffectClasses.Add(CachedData.ID, LoadedClass);
			}

			// Icon 로드 → UPROPERTY TMap에 강참조 (메모리 상주 보장)
			if (!CachedData.Icon.IsNull())
			{
				UTexture2D* LoadedIcon = CachedData.Icon.LoadSynchronous();
				if (LoadedIcon)
					CachedIcons.Add(CachedData.ID, LoadedIcon);
			}
		}
	}
}
```
</details>

---

## 6. 그 외 기술

### 퍼시스턴트 레벨 시스템

<img width="529" alt="image" src="https://github.com/user-attachments/assets/31f12f81-0c6f-4c67-9580-8c539e019fef" />

### Lumen + Lightmass 충돌 히칭 해결

> Before (269ms)
<img width="1916" alt="image" src="https://github.com/user-attachments/assets/4dab64b6-2e2e-407c-90f1-f0ae02561a8c" />

> After (26.6ms)
<img width="1225" alt="image" src="https://github.com/user-attachments/assets/c9691d9a-6834-4238-a257-7247cc1038b5" />
