# ARENA LASTGATE 
## 📋 프로젝트 개요
**ARENA LASTGATE**는 언리얼 엔진 5.5 기반의 멀티플레이어 배틀 로얄 슈팅 게임입니다. 플레이어는 라운드마다 무기 개조, 캐릭터 강화, 상점 구매 등을 통해 전략적으로 성장하며, 최후의 1인이 되기 위해 경쟁합니다.
### 프로젝트 정보
- **개발 엔진**: Unreal Engine 5.5
- **개발 언어**: C++, Blueprint
- **개발 기간**: 25.11 ~ 26.02
- **팀 구성**: 기획 5명, 프로그래머 4명, 그래픽 6명     
- **담당 역할**: 프로그래머 팀장
- **플랫폼**: PC
### 협업 도구
- **프로젝트 관리**: Jira 링크?
- **문서화**: Notion - Notion 링크
- **버전 관리**: SVN
### 데모 영상
## 🎯 담당 업무
1. **VisionSystem (시야 시스템)** - 실시간 시야 추적 및 가시성 관리
2. **ConvertSystem (무기 개조 시스템)** - 등급 기반 무기 강화 시스템
3. **ModuleSystem (캐릭터 강화 시스템)** - 캐릭터 스탯 업그레이드 시스템
4. **ShopSystem (상점 시스템)** - 아이템 구매 및 판매 시스템
5. **InventorySystem (인벤토리 시스템)** - 아이템 보유 및 사용
6. **Google Sheets 데이터 연동** - 외부 데이터 파이프라인 구축
7. **그 외 기술**
## 💡 핵심 구현 내용
### 1. Post Processing과 Custom Material을 이용한 제한된 시야 시스템         
#### 시야 움직이는 움짤(벽 차폐, 경계선에 걸친 캐릭터)     
<img width="633" height="424" alt="image" src="https://github.com/user-attachments/assets/b6647839-0866-484c-8d19-5edfa090f553" />         

#### 개요    
플레이어의 가시 범위 내에 있는 오브젝트를 실시간으로 탐지하고, 이를 미니맵 및 월드 UI에 동적으로 반영하는 시스템입니다. 원뿔형 시야 알고리즘과 레이캐스팅을 결합하여 정밀한 Line of Sight(LOS)를 구현했습니다.    

#### 기술적 특징    
성능 최적화와 시각적 피드백의 정밀도를 동시에 확보하기 위해 다음과 같은 기술을 채택했습니다.     
- **Cone-based Vision Algorithm** : 플레이어의 정면을 기준으로 원뿔 형태의 시야 범위를 계산합니다.
- **Real-time LOS Tracing** : 매 프레임 다중 레이캐스팅을 통해 장애물에 가려진 오브젝트를 정확히 판별합니다.
단, 성능저하를 우려해 TSet 자료구조와 캐싱 배열을 이용하여 중복 검사를 줄이며 성능을 최적화했습니다.   
- **Dynamic Material Rendering** : 런타임에서 머티리얼 파라미터를 실시간으로 업데이트하여 시각적인 시야 범위를 표현합니다.
- **PostProcess Integration** : 시야 상태에 따른 화면 효과(PostProcessVolume)를 연동하여 몰입감을 높였습니다.

#### 코드

### 2. Custom MMC와 GameplayEffect를 이용한 개조 및 모듈 시스템      
#### 개조 선택하는 움짤(적용결과 보여주기)       
<img width="1410" height="731" alt="image" src="https://github.com/user-attachments/assets/602a1589-d020-492e-adc8-fb9ad57d757c" />          

#### 개요     
개조는 플레이어의 무기에 추가적인 능력치나 특수 효과를 부여하는 시스템입니다. **GAS(Gameplay Ability System)**의 GameplayEffect를 기반으로 설계되어 안정적인 스탯 변조를 지원하며, Data-Driven(데이터 주도) 설계를 통해 수십 가지의 무기 속성을 유연하게 제어합니다.     
모듈은 캐릭터의 영구적인 스탯(체력, 속도, 방어력 등)을 강화하는 시스템입니다. SetByCaller를 통해 서로 다른 수치를 동적으로 주입하는 효율적인 구조를 취합니다.

#### 기술적 특징    
- 데이터 주도 설계 (Data-Driven) : CSV 및 DataTable을 활용하여 기획자가 코드 수정 없이 무기 및 캐릭터 스탯 밸런싱이 가능하게 구현했습니다.
- CustomMMC(ModMagnitudeCalculation) : 
<img width="1070" height="340" alt="image" src="https://github.com/user-attachments/assets/c6f63951-a168-4182-be78-6e3342706761" />
위 사진에 보여지듯 기획자가 요구하는 데미지 수식을 구현하기위해 GAS가 기본적으로 제공하는 연산으로는 불가능하여, UGameplayModMagnitudeCalculation을 상속받아 스텟에 따른 수식을 새롭게 정의하였습니다.    

### 3. 서버가 중앙 관리하는 상점 시스템           
#### 상점에서 구매 및 판매 움짤                    
<img width="1051" height="584" alt="image" src="https://github.com/user-attachments/assets/07f4503e-02e4-48f6-9f3a-e9a42528275c" />       


### 4. 네트워크 자동 동기화 되는 인벤토리 시스템           
#### 아이템 사용 움짤                 
<img width="644" height="367" alt="image" src="https://github.com/user-attachments/assets/62d3697d-b493-4b96-a302-bec42e2c09e4" />

### 5. 기획자 친화적인 Google Sheets 데이터 연동 시스템        
#### 데이터 파이프라인 구조
```
Google Sheets (온라인 데이터 관리)
    ↓ CSV Export
CSV Files (Content/Data/*.csv)
    ↓ Unreal Import
DataTable Assets (DT_*.uasset)
    ↓ Subsystem 로드
In-Memory Cache (TMap, TArray)
    ↓ Runtime 사용
Gameplay Systems
```
### 7. 그 외 기술      
#### 협업 효율성을 증가시키기 위한 Persistant Level 시스템.       
<img width="529" height="246" alt="image" src="https://github.com/user-attachments/assets/31f12f81-0c6f-4c67-9580-8c539e019fef" />             
여러 팀원(레벨 디자인, 라이팅, 아트)이 충돌 없이 병렬로 작업할 수 있도록 퍼시스턴트 레벨 기반의 레벨 스트리밍 구조를 채택했습니다.            
## 🔧 트러블슈팅
### 1. VisionSystem         
초기 기획은 원뿔 시야 범위 내 오브젝트만 보이는 단순한 요구사항으로, Overlap 이벤트 발생 시 SetVisibility() 함수 호출로 구현 예정이었습니다.      
하지만 추가 기획이 들어왔습니다:       
1. 장애물 뒤 캐릭터는 보이지 않아야 함 / 2. 시야 경계선에 걸친 캐릭터는 걸친 부분만 보여야 함            
기존의 Overlap 기반 SetVisibility(true/false) 방식은 Actor 단위로만 제어 가능했기 때문에, 부분 가시성 구현이 불가능했습니다.               
기술적 고민
- Actor 단위 on/off가 아닌 픽셀 단위 가시성 제어가 필요
- 장애물 차단 판정을 정확하게 처리하면서 자연스러운 시각적 전환 필요
  해결 방법 : **RenderTarget 기반 시야 맵 생성 + Material Function을 통한 GPU 가시성 판정**
1. RenderTarget(1800x1800) 생성 후 매 프레임 레이캐스팅으로 시야 원뿔 형태를 흰색 삼각형으로 렌더링 (시야 내부 = 흰색, 외부 = 검은색)
2. MPC(Material Parameter Collection) 를 통해 플레이어 위치(PlayerPosition)와 시야 범위(TraceRange)를 전역 파라미터로 공유
3. 모든 캐릭터 머티리얼에 VisionTexture 파라미터로 RenderTarget 바인딩
4. MF(Material Function) 에서 각 픽셀의 월드 위치를 RenderTarget UV로 변환하여 샘플링 후, 결과 값을 Opacity Mask로 사용.
### 2. GameinstanceSubsystem    
### 3. Level Streaming Volume
본 프로젝트는 0.1초의 반응 속도가 중요한 멀티플레이 PvP의 특성을 고려하여, 프레임 수치 최적화보다 **프레임 타임의 일관성(Consistency)**을 우선순위로 두었습니다.     
이동 중 CPU 스파이크를 유발하는 Level Streaming Volume을 과감히 제거하고, 전장 진입 시 필수 레벨을 선행 로드하는 방식을 채택했습니다. 이를 통해 렌더링 부하(Draw Call/Prims)는 다소 증가하더라도, 플레이어에게 끊김 없는 완벽한 교전 경험을 제공하는 시스템을 구축했습니다."
