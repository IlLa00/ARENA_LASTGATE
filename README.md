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

## 🎯 담당 업무
1. [**VisionSystem (시야 시스템)**](#1-post-processing과-custom-material을-이용한-제한된-시야-시스템) - 실시간 시야 추적 및 가시성 관리
2. [**Convert & Module System (개조 및 모듈 시스템)**](#2-custom-mmc와-gameplayeffect를-이용한-개조-및-모듈-시스템) - 캐릭터 스테이터스 및 무기 강화 시스템
3. [**ShopSystem (상점 시스템)**](#3-서버가-중앙-관리하는-상점-시스템) - 아이템 구매 및 판매 시스템
4. [**InventorySystem (인벤토리 시스템)**](#4-네트워크-자동-동기화-되는-인벤토리-시스템) - 아이템 보유 및 사용
5. [**Google Sheets 데이터 연동**](#5-기획자-친화적인-google-sheets-데이터-연동-시스템) - 외부 데이터 파이프라인 구축
6. [**그 외 기술**](#7-그-외-기술)

## 💡 핵심 구현 내용
### 1. Post Processing과 Custom Material을 이용한 제한된 시야 시스템                   
### ✔ 개요    
플레이어의 가시 범위 내에 있는 오브젝트를 실시간으로 탐지하고, 이를 미니맵 및 월드 UI에 동적으로 반영하는 시스템입니다.   

![bandicam 2026-02-12 10-13-45-501](https://github.com/user-attachments/assets/c24eda4e-cc58-41e1-8ab2-6a89d834fe8e)                          


### 💻 기술 경험         
### **라인트레이스**를 이용한 원뿔 시야 계산
- 초기 구현에서는 SetActorHiddenInGame을 통한 즉시 가시성 전환 방식을 사용했으나, 시각적 이질감이 들어 이를 개선하기 위해 LineTrace 기반 동적 가시성 시스템으로 전환하여 부드러운 시야 표현을 구현했습니다.
- 시야 계산 로직을 유틸리티 클래스 분리하여, 플레이어 외 다른 액터에서도 시야 시스템을 재사용할 수 있도록 **확장성을 고려한 설계**를 적용했습니다.    
       
<details>
<summary><b>🔍LineTrace 코드</b></summary>
     
```C++
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
	{
		OutDetectedPlayers->Empty();
	}

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
		{
			QueryParams.AddIgnoredActor(IgnoreActor);
		}

		FCollisionObjectQueryParams ObjectParams;
		ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

		bool bHit = World->LineTraceMultiByObjectType(
			HitResults,
			Start,
			End,
			ObjectParams,
			QueryParams
		);

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
					if (hittedActor && hittedActor->GetClass()->ImplementsInterface(UOZIMinimapVisibleActor::StaticClass()))
					{
						if (!bFoundWorldStatic || Hit.Distance < (TraceEndPoint - Start).Size())
						{
							OutDetectedMinimapVisibleObjects->Add(hittedActor);
						}
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
						{
							OutDetectedPlayers->Add(Player);
						}
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
<summary><b>🔍원뿔생성 코드</b></summary>
     
```C++
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

### **머터리얼 함수**와 **렌더타겟**를 이용한 원뿔 렌더링
<img width="1879" height="845" alt="image" src="https://github.com/user-attachments/assets/35d6504a-16b6-4c01-bfaa-8b572f787f1a" />       

- 동적 시야 렌더링을 위해 **머터리얼 함수**를 제작하였습니다.      
- **머터리얼 파라미터**와 **머터리얼 콜렉션**을 이용해 플레이어의 위치와 시야거리 등을 반영한 **렌더타겟**을 그립니다. 
<img width="503" height="578" alt="image" src="https://github.com/user-attachments/assets/c48e9ccb-995b-4908-a503-10e150931f3b" />

- 해당 머터리얼 함수를 적용하면 어느 머터리얼이든 시야각에 의한 결과 적용~~~

             


> 결과물
<img width="1063" height="517" alt="image" src="https://github.com/user-attachments/assets/4db6223b-7787-4c5e-bdf9-11aa9d9a778b" />             

### **포스트 프로세싱**                  
![bandicam 2026-02-12 12-08-56-396](https://github.com/user-attachments/assets/67bab719-1c0e-48b9-9b4d-952a8fe0018b)      
> 연막탄 내 적군은 보이지 않습니다.
     
![bandicam 2026-02-12 12-08-56-396 (1)](https://github.com/user-attachments/assets/83e9c266-8989-4310-986c-6302f0c3251b)       
> 섬광탄 피격 받을 시, 시야가 차단됩니다.

![bandicam 2026-02-12 12-19-37-339](https://github.com/user-attachments/assets/53403a85-d421-4dd0-84a4-3161434ae99b)    
> 타워 활성화 시, 특정 지역들의 시야를 확인 가능해 미니맵에도 보여집니다.         
### 에셋 캐싱을 통한 최적화
> Before
<img width="645" height="125" alt="KakaoTalk_20260120_114140293" src="https://github.com/user-attachments/assets/f292b53b-4721-428a-9e79-88be6d142c36" />     

> After
<img width="649" height="78" alt="KakaoTalk_20260120_124115111" src="https://github.com/user-attachments/assets/17a77a71-fead-49ba-a221-f3d7e7115c8d" />         

- 언리얼 인사이트 프로파일링을 통해 TickComponent 내 **GetAllActorsOfClass**함수가 매 프레임마다 호출되며 CPU 병목의 주원인임을 확인했습니다.
- 액터 배열을 멤버 변수에 캐싱하고, 0.5초 간격의 타이머로 갱신 주기를 분리해 매 프레임 재탐색을 제거했습니다.
- 그 결과, VisionComponent의 호출 횟수가 6,300회에서 2,607회로 약 59% 감소했으며, 총 실행 시간도 571ms에서 324ms로 단축되어 시야 시스템 전반의 CPU 부하를 획기적으로 낮췄습니다.


<details>
<summary><b>🔍 VisionComponent 코드</b></summary>
     
```C++
void UOZVisionComponent::LosVisionSystem()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UOZVisionComponent::LosVisionSystem);

    if (!bIsVisionInitialized)
    {
        InitializeVisionSystem();
        if (!bIsVisionInitialized)
            return;
    }

    if (!DynamicRenderTarget)  { bIsVisionInitialized = false; return; }
    if (!VisionMaterialInstance) { bIsVisionInitialized = false; return; }
    if (!CachedPostProcessVolume) { bIsVisionInitialized = false; return; }

    UKismetMaterialLibrary::SetVectorParameterValue(
        GetOwner()->GetWorld(), PlayerCollection,
        "PlayerPosition", FLinearColor(GetOwner()->GetActorLocation()));

    SetupVisionTextureForRemotePlayers();

    // 섬광탄에 맞은 경우: 원뿔을 그리지 않고 RenderTarget을 BLACK으로 클리어
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


### 2. Custom MMC와 GameplayEffect를 이용한 개조 및 모듈 시스템      
![bandicam 2026-02-12 11-27-35-314](https://github.com/user-attachments/assets/c8b0bb33-8b5f-4ff4-96e4-a4137e987f88)

#### 개요     
**개조**는 플레이어의 무기에 추가적인 능력치나 특수 효과를 부여하는 시스템입니다. GAS(Gameplay Ability System)의 GameplayEffect를 기반으로 설계되어 안정적인 스탯 변조를 지원하며, Data-Driven(데이터 주도) 설계를 통해 수십 가지의 무기 속성을 유연하게 제어합니다.     
**모듈**은 캐릭터의 영구적인 스탯(체력, 속도, 방어력 등)을 강화하는 시스템입니다. 데이터 테이블 기반이라 런타임에 값이 결정되므로 SetByCaller를 통해 서로 다른 수치를 동적으로 주입하는 효율적인 구조를 취합니다.    

#### 기술적 특징    
- 데이터 주도 설계 (Data-Driven) : CSV 및 DataTable을 활용하여 기획자가 코드 수정 없이 무기 및 캐릭터 스탯 밸런싱이 가능하게 구현했습니다.
- CustomMMC(ModMagnitudeCalculation) : 
<img width="1070" height="340" alt="image" src="https://github.com/user-attachments/assets/c6f63951-a168-4182-be78-6e3342706761" />
위 사진에 보여지듯 기획자가 요구하는 데미지 수식을 구현하기위해 GAS가 기본적으로 제공하는 연산으로는 불가능하여, UGameplayModMagnitudeCalculation을 상속받아 스텟에 따른 수식을 새롭게 정의하여 GAS를 사용만 한 것이 아니라 커스텀 확장을 해보았습니다.    

<details>
<summary><b>🔍 코드</b></summary>
     
```C++
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


### 3. 서버가 중앙 관리하는 상점 시스템           
![bandicam 2026-02-12 11-34-20-944](https://github.com/user-attachments/assets/b607b85a-87f7-4cbc-8197-4d663e3b2f0e)
![bandicam 2026-02-12 11-34-34-577](https://github.com/user-attachments/assets/9b37ae33-e4ff-40c2-b672-2dea35530465)



#### 개요
실시간 멀티플레이어 환경에서 클라이언트 측 변조를 방지하는 서버 권한 기반의 상점 시스템입니다.      
모든 거래 로직이 서버에서만 실행되며, 클라이언트는 구매 요청만 전송할 수 있고 재화·인벤토리의 실제 변경 권한을 갖지 않습니다.     

#### 기술적 특징
- **Server RPC** 기반 거래 처리 : 모든 구매·판매 함수를 UFUNCTION(Server, Reliable)로 선언하여 클라이언트가 직접 재화나 인벤토리를 조작할 수 없도록 했습니다. 클라이언트 UI는 PlayerController에게 요청만 전달하고, PlayerController가 HasAuthority() 검증 후 서버의 ShopManager로 전송합니다.     
- 서버 측 PlayerState 참조 : 서버가 GetPlayerState<AOZPlayerState>()로 요청자를 직접 조회함으로써 타 플레이어를 사칭한 구매 요청을 차단합니다.      
- DataTable 기반 서버 가격 결정 : 클라이언트가 가격을 임의로 변조해 전송하더라도, 서버는 자체 DataTable의 아이템 가격으로 재계산하므로 가격 위조가 불가능합니다.
- Replicated 재화·인벤토리 : 재화와 인벤토리 슬롯이 모두 Replicated 속성이므로 클라이언트 측 값은 서버로부터 복제되어 덮어쓰이기 때문에,클라이언트 값을 변조해도 다음 복제 시점에 서버 값으로 원복됩니다.         
  
<details>
<summary><b>🔍 코드</b></summary>

```C++
// 1단계: PlayerController — 클라이언트 요청의 진입점
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
    int32 Price = GetItemPrice(ItemID, ItemType);    // DataTable에서 조회
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


### 4. 네트워크 자동 동기화 되는 인벤토리 시스템           

#### 개요
슬롯 기반 아이템 관리 시스템으로, 네트워크 복제와 스택킹을 지원하며 기획의도인 4슬롯 + 3종류 배틀아이템 제한 규칙을 적용합니다.    

#### 기술적 특징    
- **경량 슬롯 구조** : 아이템 객체 참조 대신 ID + Type + Quantity 경량 구조체로 설계했습니다, 아이템의 실제 데이터(이름, 아이콘, 설명 등)는 ItemSubsystem을 통해 필요 시점에 조회합니다. 이 구조 덕분에 네트워크 복제 비용이 최소화됩니다.     
- **네트워크 동기화** : `ReplicatedUsing = OnRep_ItemSlots`으로 서버에서 슬롯만 바꾸면 클라이언트 UI가 자동으로 동기화되는 구조입니다.          
- **스택 가능** : 먼저 스택을 탐색하여 동일한 아이템이 이미 있으면 해당 슬롯 수량만 증가 -> TSet 자료구조를 활용해 신규 배틀아이템이면 보유 종류가 3종미만인지 확인해 아이템 종류 제한을 검증 -> 빈 슬롯을 찾아 새 아이템을 배치하고 없으면 실패를 반환합니다.


<details>
<summary><b>🔍 코드</b></summary>

```
void UOZInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
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

### 5. 기획자 친화적인 Google Sheets 데이터 연동 시스템        
#### 데이터 파이프라인 구조
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
#### 개요
`UGameInstanceSubsystem`을 상속받아 게임 전역에서 접근 가능한 데이터 관리 레이어를 구현했습니다. DataTable 기반의 데이터 드리븐 설계로 기획자 친화적인 구조를 제공합니다.      
<img width="1900" height="574" alt="image" src="https://github.com/user-attachments/assets/503707c0-3bed-413b-ba9d-d204c1dfa79b" />

#### 기술적 특징
- **TSoftObjectPtr** : 에디터에서 경로만 저장, LoadSynchronous()로 필요 시점에 로드합니다.          
- **캐싱 시스템** : `TMap` 기반 O(1) 조회 성능 + UPROPERTY를 붙여 GC가 캐싱된 에셋을 해제하지 못하도록 설정했습니다.              
- **GAS 통합** : SetByCallerMagnitude를 통한 런타임에 결정되는 DataTable 수치를 적용했습니다.            

### 7. 그 외 기술      
#### 협업 효율성을 증가시키기 위한 Persistant Level 시스템.       
<img width="529" height="246" alt="image" src="https://github.com/user-attachments/assets/31f12f81-0c6f-4c67-9580-8c539e019fef" />                 

여러 팀원(레벨 디자인, 라이팅, 아트)이 SVN 충돌 없이 병렬로 작업할 수 있도록 퍼시스턴트 레벨 기반의 레벨 스트리밍 구조를 채택했습니다.     

## 🔧 트러블슈팅
### 에셋 캐싱 및 호출 빈도 최적화를 통한 시야(Vision) 시스템 성능 최적화  
> Before
<img width="645" height="125" alt="KakaoTalk_20260120_114140293" src="https://github.com/user-attachments/assets/f292b53b-4721-428a-9e79-88be6d142c36" />    

        
시야 시스템(VisionComponent)을 구현하던 중, 다수의 액터를 대상으로 하는 연산이 매 프레임 반복되면서 CPU 점유율이 비정상적으로 높아지는 성능 병목 현상을 확인했습니다.      
특히 언리얼 인사이트 프로파일링 결과, TickComponent 내에서 호출되는 GetAllActorsOfClass 함수가 **60FPS기준 초당 약 300회 이상** 반복 실행되며 프레임 드랍의 주원인이 되고 있음을 파악했습니다.        
이를 해결하기 위해, 매 프레임 모든 액터를 새로 검색하는 대신 필요한 액터 배열을 멤버 변수에 담아두는 **에셋 캐싱 메커니즘**을 도입했습니다.      
또한, 시야 업데이트가 매 프레임 이루어질 필요가 없다는 점에 착안하여 CacheRefreshInterval을 0.5초로 설정한 별도의 타이머 로직을 구축했습니다.       
이를 통해 SetupVisionTextureForRemotePlayers, UpdateSmokeVisibility 등의 함수가 매 프레임이 아닌 캐싱된 데이터를 참조하여 동작하도록 구조를 개선했습니다.      

> After
<img width="649" height="78" alt="KakaoTalk_20260120_124115111" src="https://github.com/user-attachments/assets/17a77a71-fead-49ba-a221-f3d7e7115c8d" />         



이러한 최적화 결과, 초당 약 300회에 달하던 무거운 함수 호출 횟수를 **초당 6회 수준으로 약 98% 감소**시켰으며, 전체적인 시야 시스템의 CPU 연산 비용을 획기적으로 낮추었습니다. 이를 통해 복잡한 연산일수록 실행 빈도 조절과 데이터 캐싱이 성능에 얼마나 결정적인 영향을 미치는지 다시 한번 체감했습니다.              

### TSoftObjectPtr 참조 유실 및 GC 수거 문제 해결
프로젝트에서 데이터 테이블을 활용한 시스템을 구축하던 중, 에디터 환경에서는 정상적으로 로드되던 에셋들이 독립형 빌드에서 nullptr로 변하며 오작동하는 현상을 발견했습니다.           
원인을 분석한 결과, 에디터는 툴 자체의 기능으로 에셋을 메모리에 유지해 주지만, 실제 런타임 환경에서는 TSoftObjectPtr가 단순한 약참조(Weak Reference)로만 동작하여 이를 소유한 객체가 없을 경우 가비지 컬렉터(GC)가 즉시 수거해버리는 메커니즘 때문임을 확인했습니다.           

이 과정에서 "처음부터 일반 포인터를 사용해 자동 로딩에 의존하면 안 됐을까?"라는 의문이 들었으나, 데이터 테이블 로드 시 모든 에셋이 한꺼번에 메모리에 올라가는 비효율성을 방지하고 필요한 시점에만 자원을 관리하기 위해 TSoftObjectPtr의 사용을 유지하기로 결정했습니다.    
대신, 로드된 에셋이 GC에 의해 소멸되지 않도록 초기화 단계에서 에셋을 명시적으로 로드하고, 이를 UPROPERTY가 선언된 멤버 변수(TMap 등)에 할당하여 엔진이 인식할 수 있는 강한 참조(Strong Reference)를 수동으로 형성하는 캐싱 레이어를 구현했습니다.     
이러한 개선을 통해 환경에 관계없이 에셋 로딩의 일관성을 100% 확보할 수 있었고, 런타임 중 반복적인 검색과 로드 프로세스를 제거하여 연산 효율까지 높이는 성과를 거두었습니다.                 

<details>
<summary><b>🔍 코드</b></summary>

```C++
TArray<FName> RowNames = ConvertDataTable->GetRowNames();
ConvertCache.Reserve(RowNames.Num());

for (const FName& RowName : RowNames)
{
    FOZConvertData* Data = ConvertDataTable->FindRow<FOZConvertData>(RowName, TEXT(""));
    if (Data)
    {
        // 먼저 캐시에 복사
        int32 Index = ConvertCache.Add(*Data);
        FOZConvertData& CachedData = ConvertCache[Index];
        ConvertMap.Add(CachedData.ID, &CachedData);

        // 복사본에서 로드해야 Get()이 동작함
        // Convert_Asset (GameplayEffect) 로드 및 캐싱
        if (!CachedData.Convert_Asset.IsNull())
        {
            UClass* LoadedClass = CachedData.Convert_Asset.LoadSynchronous();
            if (LoadedClass)
                CachedEffectClasses.Add(CachedData.ID, LoadedClass); // GC 수거 방지
        }

        // Icon 로드 및 캐싱
        if (!CachedData.Icon.IsNull())
        {
            UTexture2D* LoadedIcon = CachedData.Icon.LoadSynchronous();
            if (LoadedIcon)
                CachedIcons.Add(CachedData.ID, LoadedIcon); // 메모리 상주 보장
        }
    }
}
```
</details>                  

### Lumen + Lightmass Volume 충돌로 인한 히칭(Hitching) 현상    
인게임 중 멀티플레이 환경에서 서버와 클라이언트 모두 주기적으로 화면이 멈추는 히칭 현상을 발견했습니다.       
> Before

<img width="1916" height="925" alt="image" src="https://github.com/user-attachments/assets/4dab64b6-2e2e-407c-90f1-f0ae02561a8c" />



이를 해결하기 위해 Unreal Insights를 활용하여 데이터 기반의 프로파일링을 수행한 결과, LumenSceneUpdate의 Card Capture가 프레임당 상한(300)에 도달하면서 히칭이 발생하고 있었습니다.     
추가 분석 결과, 라이트매스 볼륨(Lightmass Volume)으로 라이트를 Bake한 상태에서 실시간 루멘 연산으로 라이팅을 중복 계산하면서 프레임 당 처리 한계를 초과하는 것을 알게 되었습니다.        
이를 해결하기 위해 전역 조명 방식을 루멘에서 None(베이킹 라이트 활용)으로 전환하여 연산 부하를 제거함으로써 히칭을 완전히 사라지게 하였습니다.       
> After

<img width="1225" height="826" alt="image" src="https://github.com/user-attachments/assets/c9691d9a-6834-4238-a257-7247cc1038b5" />
  
