# EpicTales 서버 아키텍처 분석

---

## 1. 시스템 개요

EpicTales 서버는 `LobbyServer`와 `GameServer`로 구성된 분산 서버 아키텍처를 채택한 MMORPG 백엔드 시스템입니다. 이 설계는 사용자 인증 및 서버 선택과 실제 게임 플레이 로직을 분리하여 서버 부하를 분산하고 확장성을 확보합니다. 전체 시스템은 C++20 표준을 기반으로 개발되어 Windows와 Linux 환경 모두에서 운영 가능하며, 비동기 I/O, 컴포넌트 기반 설계, 이벤트 주도 방식 등 현대적인 게임 서버 개발 패턴을 적극적으로 활용합니다.

---

## 2. 프로젝트 구조 (솔루션 탐색기 뷰)

프로젝트는 기능별로 명확하게 분리된 모듈식 구조를 가집니다. 각 모듈은 응집도 높은 클래스들로 구성되어 있습니다.

```
G:/Work/EpicTales/Server/Source/
├───Asio/               # 비동기 네트워크 엔진 (Boost.Asio 기반)
│   └───Service/        #   - BaseServer.h, Connection.h, Acceptor.h
├───Core/                 # 공용 기반 라이브러리
│   ├───Algorithm/        #   - AhoCorasick.h, Dijkstra.h
│   ├───Collision/        #   - GJK.h (GJK 충돌 감지)
│   ├───Container/        #   - Delegate.h, LinkedHashMap.h
│   ├───Navmesh/          #   - Navigation.h (Recast & Detour)
│   ├───Script/           #   - Xlsx.h, Csv.h, tinyxml2.h (스크립트 파서)
│   └───Task/             #   - TaskManager.h (비동기 작업 큐)
├───Data/                 # 정적 게임 데이터 (Info 클래스)
│   └───Info/             #   - CharacterInfo.h, ItemInfo.h, QuestInfo.h
├───DB/                   # 데이터베이스 관리
│   ├───Orms/             #   - OrmPlayer.h, OrmQuest.h (ORM 클래스)
│   └───DBThread.h        #   - DB 비동기 처리 스레드
├───GameServer/           # 실제 게임 로직 서버
│   ├───Actor/            #   - Actor.h, Player.h, Npc.h (게임 월드 객체)
│   ├───Component/        #   - IComponent.h (액터 기능 컴포넌트)
│   ├───DB/               #   - CacheTx.h (DB 트랜잭션 관리)
│   ├───Logic/            #   - 핵심 게임플레이 로직
│   │   ├───Fsm/          #     - 유한 상태 기계 (Finite State Machine)
│   │   │   ├───FsmComponent.h  #     - 액터에 부착되어 FSM을 관리
│   │   │   ├───IState.h        #     - 모든 상태의 기반 클래스
│   │   │   └───PlayerStateIdle.h #     - '서 있는 상태' 등 구체적인 상태 클래스
│   │   ├───NpcAi/        #     - 행동 트리(Behavior Tree) 기반 AI
│   │   │   ├───BTNode.h        #     - 행동 트리 기본 노드
│   │   │   └───NpcAiComponent.h#     - AI 실행 컴포넌트
│   │   ├───Quest/        #     - QuestComponent.h
│   │   └───Skill/        #     - SkillComponent.h
│   ├───Room/             #   - 월드 및 공간 관리
│   │   ├───Room.h, World.h, CellManager.h
│   └───User/             #   - 유저 세션 및 패킷 처리
│       ├───User.h
│       └───Packet/       #     - Pkt...ReqHandler.h (요청 핸들러)
├───LobbyServer/          # 로비/인증 서버
├───Protocol/             # 통신 프로토콜 정의
└───(Tools)/              # (추정) 코드 자동 생성을 위한 외부 도구
```

---

## 3. 개발 워크플로우 및 자동화

프로젝트는 개발 생산성을 높이고 기획자와의 협업을 원활하게 하기 위해 코드 및 데이터 자동 생성 기술을 적극적으로 활용합니다.

-   **게임 데이터 자동화 (Excel)**: 기획자가 수정한 Excel 시트(.xlsx)의 내용이 서버 재시작 시 자동으로 인게임에 반영되는 데이터 주도(Data-Driven) 개발 환경을 구축합니다.
-   **패킷 코드 자동 생성**: `Protocol/Struct` 명세를 기반으로 통신 코드를 자동 생성합니다.
-   **DB ORM 클래스 자동 생성**: 데이터베이스 스키마를 기반으로 `DB/Orms`의 ORM 클래스들을 자동 생성합니다.

---

## 4. 주요 설계 패턴 및 기술

### 4.1. 컴포넌트 기반 액터 설계 (Component-Based Actor Design)
상속 대신 조합을 사용하는 객체 설계 패턴입니다. `Actor`라는 뼈대에 `InventoryComponent`, `SkillComponent` 등 기능 단위 부품을 장착하여 `Player`나 `Npc`를 완성합니다. 이 방식은 기능의 재사용성을 높이고, 상속 구조로는 표현하기 힘든 복합적인 객체를 유연하게 생성할 수 있게 해줍니다.

### 4.2. 상태 관리 (FSM)
`Actor`의 상태는 **유한 상태 기계(FSM)**로 관리됩니다. `FsmComponent`는 `Actor`의 현재 상태(`Idle`, `Attacking`, `Dead` 등)를 정의하고, 상태에 따른 행동 규칙을 강제합니다. 예를 들어, `Dead` 상태에서는 모든 입력 이벤트를 무시하도록 하여 로직의 안정성을 보장합니다. NPC의 구체적인 의사결정은 행동 트리(Behavior Tree)가 담당하며, 그 결과가 FSM의 상태를 변화시키는 방식으로 상호작용합니다.

### 4.3. 델리게이트를 이용한 이벤트 기반 시스템
**옵저버 패턴**의 구현체로, 시스템 간의 의존성을 제거하는 데 사용됩니다. `Player`가 NPC를 처치하면, `Player`는 `KillNpcDelegate`라는 방송 채널에 "NPC 처치" 이벤트를 보냅니다. `QuestComponent`나 `AchievementComponent`는 이 채널을 미리 구독하고 있다가, 이벤트가 발생하면 각자 필요한 로직(퀘스트 카운트 증가 등)을 수행합니다. 이 덕분에 `Player`는 퀘스트나 업적 시스템의 존재를 전혀 알 필요가 없어, 코드의 결합도가 낮아지고 기능 확장이 용이해집니다.

### 4.4. 요청/핸들러 기반의 통신 처리 (Request/Handler Pattern)
네트워크 통신부와 게임 로직을 분리하는 설계입니다. `Connection`이 클라이언트로부터 패킷을 수신하면, `PacketDispatcher`는 패킷 ID에 맞는 전용 `Handler` 클래스를 찾아 패킷 처리를 위임합니다. 이 파이프라인 구조는 각 요청에 대한 로직을 독립된 클래스에 캡슐화하여, 코드의 모듈성과 유지보수성을 크게 향상시킵니다.

### 4.5. `TaskManager`를 이용한 비동기 작업 큐
스레드로부터 안전한 작업 큐(Task Queue)입니다. 멀티스레드 환경에서 스레드 간의 통신을 안전하게 중재하는 핵심 장치로, 특정 작업을 다른 스레드에서 실행하거나(e.g., 게임 스레드 -> DB 스레드) 지정된 시간에 맞추어 실행(e.g., 일일,주간,월간)하는 데 사용됩니다. 이를 통해 복잡한 락(Lock) 없이도 데이터 동기화를 보장합니다.

### 4.6. `CacheTx`를 이용한 트랜잭션 관리
**Unit of Work** 패턴의 구현체로, 여러 DB 변경 작업을 하나의 논리적 단위로 묶습니다. 퀘스트 보상으로 여러 아이템과 재화를 지급하는 로직이 실행될 때, 모든 DB 변경 사항은 `CacheTx`에 임시 기록됩니다. 로직이 모두 성공하면 `CacheTx`는 기록된 모든 작업을 DB에 한 번에 커밋(Commit)합니다. 중간에 오류가 발생하면 모든 작업을 폐기하여, '아이템은 지급됐는데 재화는 오르지 않는' 등의 데이터 불일치 상태를 원천적으로 방지합니다.

### 4.7. Cell 기반 공간 분할 및 시야 처리 (AoI)
심리스 월드를 효율적으로 구현하기 위한 핵심 최적화 기법입니다. `CellManager`는 월드를 격자(Cell)로 나누고, 플레이어의 시야를 현재 위치한 Cell과 그 주변의 인접 Cell로 제한합니다. 서버는 이 관심 영역(AoI) 내에서 발생한 변화만을 클라이언트에 선별적으로 전송하여, 대규모 인원이 접속한 환경에서도 서버의 연산 부하와 네트워크 트래픽을 크게 줄여줍니다.

### 4.8. 전투 및 이동 시스템
전투의 복잡한 상호작용과 다수 액터의 효율적인 이동을 처리하기 위해 다양한 기술이 조합되어 사용됩니다.

-   **행동 트리 (Behavior Tree)**: NPC의 AI, 즉 의사결정 과정을 담당합니다. `NpcAiComponent`를 통해 주기적으로 현재 상황을 평가하여 '플레이어 추적', '스킬 사용', '도망' 등 가장 적절한 행동(Task)을 선택하고 실행합니다.
-   **Effect 처리**: 스킬 사용 시 발생하는 버프, 디버프, 도트 데미지와 같은 각종 상태 효과와 시각적 효과를 관리하는 시스템입니다. 각 효과는 독립된 객체로 관리되며, 지정된 시간 동안 Actor의 상태나 능력치를 변경합니다.
-   **길찾기 (Recast & Detour)**: 3D 월드 환경에서 NPC가 장애물을 피해 목표 지점까지 이동할 수 있도록 네비게이션 메쉬를 생성하고, 이를 기반으로 최적의 경로를 탐색합니다.
-   **3D 충돌 (GJK)**: 액터 간의 정밀한 실시간 3D 충돌을 감지합니다. 주로 스킬의 타격 판정이나 캐릭터 간의 물리적 상호작용에 사용되어 정확한 전투 결과를 보장합니다.
-   **2D 충돌**: 광역 스킬(AoE)의 범위 판정과 같이 높이를 무시하는 단순 충돌 계산에 사용됩니다. 3D 충돌에 비해 연산 비용이 저렴하여 성능 최적화가 필요한 부분에 제한적으로 활용됩니다.
-   **토큰 기반 이동 (Token-Based Movement)**: 다수의 유닛이 밀집했을 때 서로 겹치지 않고 자연스럽게 이동하도록 제어하는 경량화된 지역 회피(Local Avoidance) 시스템입니다. RVO(Reciprocal Velocity Obstacles)와 같은 복잡한 알고리즘 대신, 액터 주변의 16개 방향 '토큰(이동 가능 슬롯)'을 선점하는 간단한 방식으로 연산 부하를 최소화합니다.

---

## 5. 코드 스타일 및 컨벤션

프로젝트 전반에 걸쳐 Doxygen 주석 스타일을 포함한 일관된 코딩 컨벤션이 적용되어 있습니다.

#### 이벤트 기반 로직과 DB 트랜잭션 (`GameServer/Logic/Quest/QuestComponent.h`)
-   **델리게이트(Delegate)**: `Player`의 `onKillMonster`와 같은 이벤트에 `QuestComponent`의 `_onKillMonster` 함수를 등록(구독)합니다. 이를 통해 `Player`는 `QuestComponent`의 존재를 몰라도, 이벤트 발생 시 관련 로직(퀘스트 조건 달성)이 자동으로 처리됩니다. (옵저버 패턴)
-   **DB 트랜잭션(CacheTx)**: 이벤트 처리 함수 내에서는 `CacheTx`를 사용하여 관련된 모든 데이터 변경(퀘스트 상태 업데이트, 보상 아이템 지급 등)을 하나의 작업 단위로 묶어 데이터 정합성을 보장합니다. (Unit of Work 패턴)

```cpp
#pragma once

#include <Core/Container/Delegate.h>
#include "Component/IComponent.h"
#include "DB/CacheTx.h"
// ... other includes

class Player;
class QuestInfo;

/// @brief 플레이어의 퀘스트 관련 기능을 담당하는 컴포넌트
class QuestComponent : public IComponent
{
    Player& _player;

    // Player의 이벤트에 구독하기 위한 핸들
    Core::HandlePtr _killMonsterHandle;
    Core::HandlePtr _acquireItemHandle;
    // ...

private:
    /// @brief 컴포넌트 초기화 시 Player의 델리게이트에 이벤트 핸들러를 등록합니다.
    virtual void initialize() override;

    /// @brief Player가 몬스터를 처치했을 때 호출되는 이벤트 핸들러 함수
    /// @param cacheTx DB 작업을 묶는 트랜잭션 객체
    /// @param npcInfoId 처치된 몬스터의 정보 ID
    void _onKillMonster(CacheTx& cacheTx, PktInfoId npcInfoId) const;

    /// @brief 아이템을 획득했을 때 호출되는 이벤트 핸들러 함수
    void _onItemAcquire(CacheTx& cacheTx, const ItemInfo& info);

public:
    /// @brief 퀘스트 완료를 처리하고 DB에 반영합니다.
    /// @return 모든 작업이 성공하면 true를 반환합니다.
    bool completeQuest(
        CacheTx& cacheTx,
        const QuestInfo& questInfo,
        std::vector<PktQuest>& updatedQuests) const;

    // ...
};
```

---

## 6. 핵심 알고리즘 및 서드파티 라이브러리

### 6.1. 핵심 알고리즘
-   **`Dijkstra`**: NPC의 최단 경로 계산.
-   **`Aho-Corasick`**: 채팅 금칙어 필터링.

### 6.2. 서드파티 라이브러리
-   **`Boost.Asio`**: 고성능 비동기 네트워크 엔진의 기반.
-   **`MySQL Connector`**: MySQL 데이터베이스와의 직접 통신.
-   **`hiredis`**: Redis 서버와의 통신을 위한 C 클라이언트.
-   **`rapidjson` / `tinyxml2`**: JSON 및 XML 데이터 파싱.
-   **`miniz`**: 데이터 압축.
