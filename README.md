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
- **프로젝트 관리**: Jira
- **문서화**: Notion - [Notion 링크]
- **버전 관리**: SVN

## 🎯 담당 업무
1. **VisionSystem (시야 시스템)** - 실시간 시야 추적 및 가시성 관리
2. **ConvertSystem (무기 개조 시스템)** - 등급 기반 무기 강화 시스템
3. **ModuleSystem (캐릭터 강화 시스템)** - 캐릭터 스탯 업그레이드 시스템
4. **ShopSystem (상점 시스템)** - 아이템 거래 시스템
5. **InventorySystem (인벤토리 시스템)** - 아이템 보관 및 관리
6. **Google Sheets 데이터 연동** - 외부 데이터 파이프라인 구축

## 💡 핵심 구현 내용
### 1. VisionSystem (시야 시스템)     
### 2. ConvertSystem (무기 개조 시스템)     
### 3. ModuleSystem (캐릭터 강화 시스템)     
### 4. ShopSystem (상점 시스템)    
### 5. InventorySystem (인벤토리 시스템)
### 6. Google Sheets 데이터 연동
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
## 🔧 트러블슈팅
### 1. VisionSystem
초기 기획은 원뿔 시야 범위 내 오브젝트만 보이는 단순한 요구사항으로, Overlap 이벤트 발생 시 SetVisibility() 함수 호출로 구현 예정이었습니다.     
하지만 추가 기획이 들어왔습니다:       
1. 장애물 뒤 캐릭터는 보이지 않아야 함 / 2. 시야 경계선에 걸친 캐릭터는 걸친 부분만 보여야 함
기존의 Overlap 기반 SetVisibility(true/false) 방식은 Actor 단위로만 제어 가능했기 때문에, 부분 가시성 구현이 불가능했습니다.
기술적 고민
- Actor 단위 on/off가 아닌 픽셀 단위 가시성 제어가 필요
- 매 프레임 수백 개의 LineTrace를 수행하면서도 성능 유지 필요
- 장애물 차단 판정을 정확하게 처리하면서 자연스러운 시각적 전환 필요
  해결 방법 : **RenderTarget 기반 시야 맵 생성 + Material Function을 통한 GPU 가시성 판정**
1. RenderTarget(1800x1800) 생성 후 매 프레임 레이캐스팅으로 시야 원뿔 형태를 흰색 삼각형으로 렌더링 (시야 내부 = 흰색, 외부 = 검은색)
2. MPC(Material Parameter Collection) 를 통해 플레이어 위치(PlayerPosition)와 시야 범위(TraceRange)를 전역 파라미터로 공유
3. 모든 캐릭터 머티리얼에 VisionTexture 파라미터로 RenderTarget 바인딩
4. MF(Material Function) 에서 각 픽셀의 월드 위치를 RenderTarget UV로 변환하여 샘플링 후, 색상 값을 Opacity로 사용 (흰색 = 보임, 검은색 = 안 보임)
