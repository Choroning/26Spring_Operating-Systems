# 13주차 이론 — 저장 장치 관리 (Storage Management)

> **최종 수정일:** 2026-05-26
>
> Silberschatz, Operating System Concepts Ch 11 (Mass-Storage Structure)

> **선수 지식**: 11주차(주 메모리)와 12주차(가상 메모리) — §7의 swap-space 관리는 12주차의 demand-paging 기제와 직접 연결; 거기서의 EAT / 페이지 폴트 분석이 디스크 접근 시간이 왜 중요한지의 동기. 컴퓨터 구조의 DMA, 컨트롤러, 버스 토폴로지 배경이 §8에 도움. 기본 이산수학 어휘: 배열, 모듈러 산술 (striping에 사용), XOR (parity에 사용).
>
> **학습 목표**:
> 1. HDD의 **물리적 구조** (platter, track, sector, cylinder, head, arm, spindle)를 기술하고 head crash 설명
> 2. **접근 시간** 을 seek time, rotational latency, transfer time으로 분해하고 전형적 총합 계산
> 3. **HDD** 와 **NVM/SSD** 를 기계적, 성능, 내구성 축에서 구분
> 4. **NAND Flash** 의 세부 사항 설명: 페이지 단위 R/W, 블록 단위 erase, 덮어쓰기 불가; 그리고 컨트롤러 메커니즘 **FTL, garbage collection, over-provisioning, wear leveling, TRIM, write amplification**
> 5. 저장 장치 **연결 인터페이스** (SATA, NVMe, SAS, USB, Fibre Channel) 비교와 **LBA** 주소 지정의 의미 (CHS, CAV vs CLV)
> 6. **HDD 스케줄링 알고리즘** 실행과 비교: FCFS, SCAN (elevator), C-SCAN, LOOK, C-LOOK
> 7. **오류 탐지/정정** 기법 설명: parity, CRC, ECC, HDD/SSD에서의 사용
> 8. **드라이브 포매팅** (low-level → 파티셔닝 → 논리 포매팅), 볼륨 관리, **부트 블록**, **불량 블록 처리** (sector sparing, sector slipping)
> 9. Linux의 **swap-space 관리** 설명 (파티션 vs 파일, swap map)
> 10. **저장 장치 부착** 방법 구분: host-attached, NAS, SAN, 클라우드 스토리지
> 11. **RAID 0/1/4/5/6/10** 을 striping, mirroring, parity, 내결함성, 사용 사례 측면에서 설명
> 12. **mirrored array의 MTBF** 계산과 독립 가정이 깨지는 경우 설명
> 13. RAID가 보호 **못** 하는 것과 **ZFS 식 checksum** 이 그 공백을 메우는 방식 설명

---

## 목차

- [1. HDD 구조와 접근 시간](#1-hdd-구조와-접근-시간)
  - [1.1 대용량 저장 장치 개요](#11-대용량-저장-장치-개요)
  - [1.2 HDD 물리적 구조](#12-hdd-물리적-구조)
  - [1.3 회전 속도와 용량](#13-회전-속도와-용량)
  - [1.4 접근 시간 = Seek + Rotational + Transfer](#14-접근-시간--seek--rotational--transfer)
  - [1.5 구체적 접근 시간 예시](#15-구체적-접근-시간-예시)
- [2. NVM 장치 — SSD와 NAND Flash](#2-nvm-장치--ssd와-nand-flash)
  - [2.1 HDD vs SSD](#21-hdd-vs-ssd)
  - [2.2 NAND Flash 특성](#22-nand-flash-특성)
  - [2.3 FTL, Garbage Collection, Over-Provisioning](#23-ftl-garbage-collection-over-provisioning)
  - [2.4 Wear Leveling, TRIM, Write Amplification](#24-wear-leveling-trim-write-amplification)
  - [2.5 NVM 스케줄링](#25-nvm-스케줄링)
  - [2.6 휘발성 메모리를 저장 장치로 (RAM Drive)](#26-휘발성-메모리를-저장-장치로-ram-drive)
- [3. 연결과 주소 매핑](#3-연결과-주소-매핑)
  - [3.1 연결 인터페이스](#31-연결-인터페이스)
  - [3.2 Logical Block Address (LBA)](#32-logical-block-address-lba)
- [4. HDD 스케줄링 알고리즘](#4-hdd-스케줄링-알고리즘)
  - [4.1 목표와 요청 구조](#41-목표와-요청-구조)
  - [4.2 FCFS](#42-fcfs)
  - [4.3 SCAN — Elevator 알고리즘](#43-scan--elevator-알고리즘)
  - [4.4 C-SCAN — Circular SCAN](#44-c-scan--circular-scan)
  - [4.5 LOOK과 C-LOOK](#45-look과-c-look)
  - [4.6 같은 큐에서의 비교](#46-같은-큐에서의-비교)
  - [4.7 Linux I/O 스케줄러](#47-linux-io-스케줄러)
- [5. 오류 탐지와 정정](#5-오류-탐지와-정정)
  - [5.1 Parity, CRC, ECC](#51-parity-crc-ecc)
  - [5.2 실전에서의 ECC](#52-실전에서의-ecc)
- [6. 저장 장치 관리](#6-저장-장치-관리)
  - [6.1 드라이브 포매팅](#61-드라이브-포매팅)
  - [6.2 파티션과 볼륨](#62-파티션과-볼륨)
  - [6.3 부트 블록](#63-부트-블록)
  - [6.4 불량 블록 관리](#64-불량-블록-관리)
- [7. Swap-Space 관리](#7-swap-space-관리)
  - [7.1 목적과 크기 설정](#71-목적과-크기-설정)
  - [7.2 위치 — 파티션 vs 파일](#72-위치--파티션-vs-파일)
  - [7.3 Linux Swap 내부 구조](#73-linux-swap-내부-구조)
- [8. 저장 장치 부착](#8-저장-장치-부착)
  - [8.1 개요](#81-개요)
  - [8.2 NAS](#82-nas)
  - [8.3 SAN](#83-san)
  - [8.4 클라우드 스토리지](#84-클라우드-스토리지)
- [9. RAID](#9-raid)
  - [9.1 목적과 구성 요소](#91-목적과-구성-요소)
  - [9.2 중복성을 통한 신뢰성 — MTBF 수학](#92-중복성을-통한-신뢰성--mtbf-수학)
  - [9.3 Striping을 통한 성능](#93-striping을-통한-성능)
  - [9.4 RAID 0 — Striping 전용](#94-raid-0--striping-전용)
  - [9.5 RAID 1 — Mirroring](#95-raid-1--mirroring)
  - [9.6 RAID 4 — Striping + 전용 Parity](#96-raid-4--striping--전용-parity)
  - [9.7 RAID 5 — 분산 Parity](#97-raid-5--분산-parity)
  - [9.8 RAID 6 — 이중 Parity](#98-raid-6--이중-parity)
  - [9.9 RAID 0+1 vs RAID 1+0 (RAID 10)](#99-raid-01-vs-raid-10-raid-10)
  - [9.10 종합 비교](#910-종합-비교)
  - [9.11 Hot Spare와 구현 계층](#911-hot-spare와-구현-계층)
  - [9.12 RAID가 보호하지 못하는 것 — ZFS](#912-raid가-보호하지-못하는-것--zfs)
- [10. 실습 — 디스크 스케줄링 시뮬레이터](#10-실습--디스크-스케줄링-시뮬레이터)
- [요약](#요약)
- [점검 문제](#점검-문제)

---

<br>

## 1. HDD 구조와 접근 시간

### 1.1 대용량 저장 장치 개요

| 분류                       | 예시                          | 특성                                         |
|----------------------------|-------------------------------|----------------------------------------------|
| **2차 저장 (Secondary)**   | HDD, SSD                      | 주된 비휘발성 작업 저장 장치                  |
| **3차 저장 (Tertiary)**    | 자기 테이프, 광 디스크        | 느리지만 대용량, 백업용                       |
| **NVS (Nonvolatile Storage)** | 모든 비휘발성 장치          | 전원이 꺼져도 데이터 유지                     |

이번 주의 목표:
- 저장 장치의 물리적 구조 이해.
- OS가 요청 순서를 정하는 I/O 스케줄링 알고리즘 분석.
- **RAID** 를 포함한 OS의 저장 장치 관리 서비스 개관.

### 1.2 HDD 물리적 구조

```
    Platter 1  ──── Head 1 (위 표면)
                ──── Head 2 (아래 표면)
    Platter 2  ──── Head 3
                ──── Head 4
    Platter 3  ──── Head 5
                ──── Head 6
         ↑
       Spindle        ←── 디스크 암 (모든 head를 함께 이동)
```

| 구성 요소   | 설명                                                                       |
|-------------|----------------------------------------------------------------------------|
| **Platter** | 데이터가 저장되는 강성 원판 (양쪽 표면 사용 가능)                          |
| **Track**   | platter 표면의 동심원                                                       |
| **Sector**  | track의 최소 논리 단위 (역사적으로 512 B, 4 KB로 전환 중)                  |
| **Cylinder**| 모든 platter에서 같은 반경의 track 집합                                     |

- **Read-write head** 는 매우 얇은 공기(또는 헬륨) 쿠션 위로 떠서 각 platter 표면을 비행.
- 모든 head가 **단일 디스크 암** 에 부착되어 **동시에** 움직이므로, seek time은 platter당이 아닌 암 이동당 한 번씩만 지불.
- Platter 표면은 자성 물질로 코팅되어 자기적으로 기록/읽기.

> **Head crash.** 헤드가 platter 표면에 물리적으로 접촉하는 재앙적 사건 — 자성 코팅을 파괴하여 데이터 복구 불가. 디스크 전체를 교체해야 함. RAID (§9)가 시스템 수준의 주된 방어책.

### 1.3 회전 속도와 용량

Platter는 고정 **RPM** (분당 회전 수)으로 회전:

| RPM    | 전형적 사용 사례               | 1회전 시간 |
|--------|-------------------------------|------------|
| 5,400  | 노트북, 저전력                | 11.1 ms     |
| 7,200  | 데스크탑, 일반 서버           | 8.3 ms      |
| 10,000 | 고성능 서버                   | 6.0 ms      |
| 15,000 | 엔터프라이즈 SAS              | 4.0 ms      |

- **Transfer rate** = 드라이브와 호스트 사이 데이터 속도. 실제 전송은 항상 명목 수치보다 낮음.
- 드라이브 컨트롤러는 prefetch/write-back 캐시로 **내장 DRAM 버퍼** 포함.
- 일반 platter 직경: 1.8 ~ 3.5 인치.

### 1.4 접근 시간 = Seek + Rotational + Transfer

$$
\text{접근 시간} = \text{Seek time} + \text{Rotational latency} + \text{Transfer time}
$$

| 구성 요소               | 설명                                                                 | 크기            |
|-------------------------|----------------------------------------------------------------------|-----------------|
| **Seek time**           | 암이 원하는 cylinder로 이동하는 시간                                  | 가장 큼 (~ ms)  |
| **Rotational latency**  | 원하는 sector가 head 아래로 회전하는 대기 시간                       | 중간 (~ ms)     |
| **Transfer time**       | 데이터를 실제로 읽거나 쓰는 시간                                     | 가장 작음 (~ μs)|

```
  Seek time           Rotational latency       Transfer time
  ──────────→         ──────────────────→      ─────→
  암 이동              sector 회전 대기          데이터 전송
```

랜덤 I/O에서 **seek time이 지배**. HDD 스케줄링(§4)의 목표는 요청 재정렬로 **총 seek 거리 최소화**.

### 1.5 구체적 접근 시간 예시

7,200 RPM 드라이브:

- 1회전 = $60 / 7{,}200 = 8.33$ ms.
- 평균 rotational latency = $8.33 / 2 = \mathbf{4.17\ ms}$ (평균 반 회전 대기).

전형적 분해:

```
  평균 seek time:             8 ms
  평균 rotational latency:    4.17 ms
  Transfer time (4 KB):       ~0.01 ms
  ──────────────────────────────────
  총 접근 시간:                ~12.18 ms
```

- Seek time이 전체의 약 **66%**.
- 따라서 초당 약 **$1000 / 12.18 \approx 82$ 랜덤 I/O** — HDD를 정의하는 "82 IOPS".
- **순차 I/O는 훨씬 빠름** — seek와 회전 비용이 긴 연속 실행 위로 분산되기 때문 — HDD는 랜덤과 순차 사이 성능 격차가 거대.

> **이 슬라이드에서 가장 중요한 숫자는 82 IOPS.** 왜 데이터베이스가 인덱스를 사용하는지(랜덤 접근을 순차로 변환), 왜 파일시스템이 쓰기를 일괄 처리하는지(seek 분산), 왜 SSD가 혁신적으로 느껴지는지(seek 패널티 완전 제거)를 설명.

---

<br>

## 2. NVM 장치 — SSD와 NAND Flash

### 2.1 HDD vs SSD

| 속성                     | HDD                          | SSD                            |
|--------------------------|------------------------------|--------------------------------|
| 기계 부품                | 있음 (암, spindle)            | 없음                            |
| Seek time                | 있음                          | 없음                            |
| Rotational latency       | 있음                          | 없음                            |
| 랜덤 vs 순차 격차        | 큼                            | 작음                            |
| 속도                     | 느림                          | 빠름                            |
| 내구성                   | 기계적 마모                   | 쓰기 사이클 제한                |
| 전력 소비                | 높음                          | 낮음                            |
| GB당 비용                | 저렴                          | 비쌈                            |

폼 팩터: 2.5"/3.5" SATA SSD, 메인보드에 솔더링된 M.2 NVMe 드라이브, USB 스틱, 스마트폰 내장 저장, eMMC 등 — 물리적으로 다양하지만 모두 **NAND Flash** 메모리 기반.

### 2.2 NAND Flash 특성

매우 다른 단위와 속도를 가진 세 가지 연산:

| 연산  | 단위                                       | 속도              |
|-------|--------------------------------------------|-------------------|
| Read  | **Page** (보통 4 KB – 16 KB)              | 가장 빠름         |
| Write | **Page**                                   | Read보다 느림      |
| Erase | **Block** (여러 page의 그룹)               | 가장 느림         |

**핵심 비대칭 — 덮어쓰기 불가.**

- 이미 데이터를 담은 페이지는 직접 덮어쓸 수 없음.
- 페이지를 다시 쓰려면 전체 포함 블록이 먼저 **erase** 되어야 함, 그 다음 쓰기.

이 단일 속성이 아래의 모든 컨트롤러 기교로 연쇄.

- **수명:** 셀당 약 $10^5$ program-erase 사이클 (제품 등급별로 크게 다름).
- **DWPD** (Drive Writes Per Day): 명목 수명 동안 *전체* 드라이브 용량을 하루에 몇 번 덮어쓸 수 있는가 — 표준 내구성 측정.

### 2.3 FTL, Garbage Collection, Over-Provisioning

**Flash Translation Layer (FTL).** OS가 보는 것 (LBA, 단순 블록 read/write)과 NAND flash가 요구하는 것 (페이지 단위 R/W, 블록 단위 erase, 덮어쓰기 불가) 사이를 잇는 컨트롤러 내 소프트웨어.

- **logical block → physical page** 매핑 유지.
- 유효/무효 페이지 추적.
- OS는 단지 LBA를 read/write; 그 아래의 모든 것은 컨트롤러 문제.

**Garbage Collection.** 드라이브가 무효(재기록된) 페이지로 차면 GC 루틴이:

```
  Block A: [Valid][Invalid][Invalid][Valid]   → 유효 페이지 복사 출
  Block B: [Valid][Valid][Valid][Free]          ← 여기로
  Block A: [Erased]                             → 재사용 가능
```

GC는 대역폭을 소비하고 write amplification에 기여.

**Over-Provisioning.** 드라이브 물리 용량의 ~20% 를 사용자에게 보이지 않는 예비 공간으로 예약. GC 중 스크래치 공간으로, wear leveling 버퍼로 사용. **성능** (GC가 유효 데이터를 둘 곳)과 **수명** (쓰기 사이클이 더 큰 물리 풀에 분산) 모두 향상.

### 2.4 Wear Leveling, TRIM, Write Amplification

**Wear leveling.** 없으면 hot 블록(자주 재기록되는)은 cold 블록보다 훨씬 빨리 마모. 컨트롤러가 **erase를 모든 블록에 고르게 분산**:

```
  Wear leveling 없이:               Wear leveling 있음:
  Block 0: ████████ (heavy)         Block 0: ████ (균등)
  Block 1: █        (idle)          Block 1: ████ (균등)
  Block 2: ██████   (heavy)         Block 2: ████ (균등)
  Block 3: █        (idle)          Block 3: ████ (균등)
```

**TRIM.** 파일이 파일시스템 수준에서 삭제될 때 OS가 SSD 컨트롤러에 "블록 X는 더 이상 사용 안 됨" 을 알리는 TRIM 명령 전송. 컨트롤러는 그 페이지들을 즉시 무효로 표시, GC를 미리 실행, 곧 폐기될 데이터의 복사를 피할 수 있음. TRIM 없이는 SSD가 유효 사용자 데이터와 파일시스템-삭제-하지만-여전히-flash-에-있는 데이터를 구분할 방법이 없음.

**Write amplification.** OS의 단일 논리 쓰기가 내부적으로 여러 read, write, erase를 유발 가능 (GC가 유효 데이터를 비키기 위해). Write amplification factor (WAF)가 이 오버헤드를 측정 — WAF 3은 4 KB 호스트 쓰기가 12 KB 실제 NAND 트래픽이 됨. 성능과 수명 모두에 낮을수록 좋음.

> **큰 그림:** OS는 깔끔한 블록 장치를 본다. 그 아래에서 컨트롤러는 끊임없이 garbage-collect, 재매핑, 페이지 셔플링. SSD가 "느리고 깨지기 쉽다" 대신 "빠르고 견고하게" 느껴지는 이유는 이 모든 작업이 FTL에 의해 숨겨지기 때문.

### 2.5 NVM 스케줄링

SSD는 **seek 없음** — §4의 HDD 스케줄링 알고리즘은 거의 무관 — 암 이동을 위해 재정렬할 것이 없음.

| 속성             | HDD                       | SSD                                       |
|------------------|---------------------------|-------------------------------------------|
| 랜덤 read        | 느림 (seek 필요)           | 빠름 (~ 100K IOPS)                        |
| 순차 read        | 비교적 빠름                | 빠름                                       |
| 랜덤 write       | 느림                       | 빠르지만 **가변** (GC 의존)                |
| 순차 write       | 비교적 빠름                | 빠름                                       |

- SSD는 보통 단순한 **FCFS** 정책 사용 (Linux NOOP 스케줄러).
- 인접 요청은 **병합** 되어 더 큰, 효율적 전송으로.
- Read 서비스 시간은 균일, 그러나 **write 서비스 시간은 비균일** — GC, 현재 활용도, wear-leveling 상태에 의존.
- HDD: 수백 IOPS. SSD: **수십만 IOPS**.

### 2.6 휘발성 메모리를 저장 장치로 (RAM Drive)

**RAM drive** 는 DRAM의 일부를 블록 장치로 사용. 디바이스 드라이버가 DRAM 영역을 할당해 파일시스템 계층에 디스크처럼 제시.

- 가능한 가장 빠른 저장 장치 — SSD를 자릿수로 능가.
- **휘발성**: 시스템 재시작 시 데이터 손실.
- 사용 사례: 고속 임시 파일 저장, 프로그램 간 데이터 공유, 빌드 시스템 캐시.

| OS      | RAM drive 메커니즘                       |
|---------|------------------------------------------|
| Linux   | `/dev/ram`, `tmpfs` (예: `/tmp`)          |
| macOS   | `diskutil` 명령                          |
| Windows | 서드파티 도구                            |

Linux의 **initrd** 는 초기 부팅 중 실제 root 장치를 사용할 수 있기 전 RAM drive를 임시 root 파일시스템으로 사용.

---

<br>

## 3. 연결과 주소 매핑

### 3.1 연결 인터페이스

| 인터페이스               | 특성                                                                |
|--------------------------|---------------------------------------------------------------------|
| **SATA**                 | 범용, 저비용, 최대 ~600 MB/s, 가장 일반적                            |
| **NVMe (PCIe)**          | 고속, 저지연, SSD에 최적화, PCIe 직접 연결                            |
| **SAS**                  | 서버급, 고신뢰성, Fibre Channel보다 저렴                              |
| **USB**                  | 외장 저장, 보편적 연결                                                |
| **Fibre Channel (FC)**   | SAN 환경, 고속 전용 네트워크 저장                                     |

- **Host Bus Adapter (HBA)** = *컴퓨터* 측 버스 컨트롤러.
- **Device Controller** = 각 *저장 장치* 의 내장 컨트롤러.

```
  CPU → HBA → 버스 (SATA / NVMe / SAS) → Device Controller → Media
                                                  ↑
                                        DRAM 캐시 (컨트롤러-local)
```

> **NVMe가 SATA보다 극적으로 빠른 이유:** NVMe는 SATA 프로토콜 스택을 거치지 않고 **PCIe** 버스에 직접 탑승. 더 낮은 지연, 훨씬 더 높은 큐 깊이 (큐당 64K 명령, 64K 큐), 그리고 현대 SSD가 실제로 제공할 수 있는 병렬성과 맞아떨어짐.

### 3.2 Logical Block Address (LBA)

OS는 cylinder나 page를 신경 쓰지 않음. 장치를 0, 1, 2, ... 로 번호 매겨진 **1차원 논리 블록 배열** 로 주소 지정.

```
  Application       OS              저장 장치
  ──────────→  ──────────→       ──────────────────
  파일 이름     LBA 번호           물리적 위치
               (0, 1, 2, …)        HDD: Cylinder-Head-Sector (CHS)
                                  NVM: Chip-Block-Page
```

**HDD에서의 LBA → CHS 매핑.** Sector 0은 가장 바깥 cylinder의 첫 track의 첫 sector. 이후 LBA는 track → cylinder 안쪽으로 나선. 실제 매핑은 더 복잡 — 불량 sector 교체, track별 가변 sector 수 등.

**CAV vs CLV** (회전 매체에 데이터를 기록하는 두 방식):

| 방법 | 설명                                                                | 사용처    |
|------|---------------------------------------------------------------------|-----------|
| **CAV (Constant Angular Velocity)** | 일정 RPM; 바깥 track이 더 많은 sector 보유      | HDD       |
| **CLV (Constant Linear Velocity)**  | 일정 데이터 밀도; 바깥 가장자리에서 더 느리게 회전 | CD/DVD    |

---

<br>

## 4. HDD 스케줄링 알고리즘

### 4.1 목표와 요청 구조

디스크 스케줄링의 목표:
- **접근 시간 최소화** (특히 seek time).
- **데이터 전송 대역폭 최대화**.
- 암 이동 거리를 줄이기 위해 요청 큐 재정렬.

각 I/O 요청에 첨부되는 정보:
1. 방향 (input / output).
2. 열린 파일 핸들.
3. 대상 메모리 주소 (바이트를 전달/가져올 곳).
4. 데이터 크기.

디스크가 바쁠 때 새 요청은 **대기 큐(pending queue)** 에 쌓임. OS는 순서 제약(예: 의존 read 전의 write)을 위반하지 않는 한 성능을 위해 큐를 재정렬 가능. 현대 드라이브는 LBA를 쓰므로 가까운 LBA는 물리적으로 가까운 것으로 가정 — 스케줄링에 충분.

> **이 절 전반에 쓰이는 풀이 예시 설정:**
> 요청 큐: `98, 183, 37, 122, 14, 124, 65, 67`
> Head 시작: 53
> 디스크 cylinder: 0 – 199

### 4.2 FCFS

**First-Come, First-Served.** 도착 순서대로 요청 처리.

```
  head
   53 → 98 → 183 → 37 → 122 → 14 → 124 → 65 → 67
```

총 head 이동:
$|53-98| + |98-183| + |183-37| + |37-122| + |122-14| + |14-124| + |124-65| + |65-67|$
$= 45 + 85 + 146 + 85 + 108 + 110 + 59 + 2 = \mathbf{640\ cylinder}$.

| 장점                  | 단점                                       |
|-----------------------|--------------------------------------------|
| 구현 단순             | Head가 격렬히 앞뒤로 흔들림                 |
| 공정 (재정렬 없음)    | 총 seek 거리가 큼                          |
| Starvation 없음       | 성능 빈약                                  |

**122 → 14 → 124** 구간 주목: 암이 거의 디스크 전체를 두 번 횡단. "37, 14 그 다음 122, 124" 로 재정렬하면 훨씬 효율적 — 정확히 다음 알고리즘이 하는 것.

### 4.3 SCAN — Elevator 알고리즘

한 방향으로 디스크 끝까지 이동하며 요청을 처리한 뒤, **방향을 반전** 해 남은 것을 처리 — 정확히 건물 엘리베이터처럼.

같은 큐, head 53, 초기 방향 *감소*:

```
  head
   53 → 37 → 14 → [0] → 65 → 67 → 98 → 122 → 124 → 183
                    ↑
                끝 도달 후 방향 반전
```

총 이동:
$(53-37) + (37-14) + (14-0) + (0-65) + (65-67) + \dots$
$= 16 + 23 + 14 + 65 + 2 + 31 + 24 + 2 + 59 = \mathbf{236\ cylinder}$

FCFS의 640보다 훨씬 좋음.

> **왜 "엘리베이터"?** 위로 가는 사람 한 명, 아래로 가는 사람 한 명을 둔 실제 엘리베이터는 층 사이를 무작위로 튕기지 않음 — 꼭대기까지 스캔한 후 바닥까지 스캔. 같은 발상.

### 4.4 C-SCAN — Circular SCAN

SCAN의 단점: 가운데 근처 위치는 라운드당 **두 번** 서비스 (양 방향 한 번씩), 끝 위치는 라운드당 한 번만 서비스. 대기 시간이 비균일.

**C-SCAN** 은 요청을 **한 방향에만** 서비스. 끝에 도달하면 head가 **즉시 시작으로 점프** 하며 반환 중에는 아무것도 서비스하지 않음.

같은 큐, 방향 *증가*:

```
  53 → 65 → 67 → 98 → 122 → 124 → 183 → [199]
                                            ↓ (점프, 서비스 없음)
  [0] → 14 → 37
```

| 속성                | SCAN                | C-SCAN                                        |
|---------------------|---------------------|-----------------------------------------------|
| 서비스 방향         | 양방향              | 단방향                                        |
| 대기 시간           | 가운데 우대         | **위치별 균일**                                |
| 구현                | 단순                | 약간 더 복잡                                  |

C-SCAN은 cylinder를 **순환 리스트** 로 취급하여 마지막 cylinder에서 첫 번째로 wrap.

### 4.5 LOOK과 C-LOOK

SCAN / C-SCAN의 "끝까지 가기" 는 끝 근처에 요청이 없으면 이동을 낭비. **LOOK** 과 **C-LOOK** 은 현재 방향에 더 이상 요청이 없는 즉시 반전/wrap:

```
  SCAN:    53 → 37 → 14 → [0] → 65 → …    (0까지 감)
  LOOK:    53 → 37 → 14 →        65 → …   (마지막 요청 14에서 반전)

  C-SCAN:  53 → 65 → … → 183 → [199] → [0] → 14 → 37
  C-LOOK:  53 → 65 → … → 183 →               14 → 37
```

| 알고리즘 | 디스크 끝까지? | 양방향 서비스? |
|----------|----------------|-----------------|
| SCAN     | Yes            | Yes             |
| LOOK     | No             | Yes             |
| C-SCAN   | Yes            | No (단방향)     |
| C-LOOK   | No             | No (단방향)     |

실전에서 **LOOK / C-LOOK** 이 SCAN / C-SCAN보다 훨씬 더 자주 쓰임.

### 4.6 같은 큐에서의 비교

위와 같은 설정: 큐 `98, 183, 37, 122, 14, 124, 65, 67`, head 53, 디스크 0–199.

| 알고리즘        | 총 이동       | 특성                                          |
|-----------------|---------------|-----------------------------------------------|
| FCFS            | **640**       | 공정하지만 비효율                              |
| SCAN (down)     | **236**       | 양방향 서비스, 효율적                          |
| C-SCAN (up)     | **382**       | cylinder별 균일 대기 시간                      |
| LOOK (down)     | **208**       | SCAN 개선, 디스크 끝 여행 없음                  |
| C-LOOK (up)     | **330**       | C-SCAN 개선, 디스크 끝 여행 없음                |

**알고리즘 선택 휴리스틱:**

- **부하 높은 시스템:** SCAN, C-SCAN (내장 starvation 방지).
- **균일 응답 시간 필요:** C-SCAN, C-LOOK.
- **가벼운 부하 / 단순 시스템:** FCFS (큐가 짧으면 모든 알고리즘이 비슷한 성능).

### 4.7 Linux I/O 스케줄러

**Deadline scheduler.**
- 별도 read/write 큐; **read 우선** (write는 일괄 처리 가능, read는 사용자를 블록).
- 각 큐 내에서 LBA 정렬(C-SCAN 식)로 효율적 일괄 처리.
- 각 요청은 **deadline** 보유 (기본 500 ms) — 어떤 요청도 영원히 기다리지 않음.
- 오래된 요청 처리를 보장하기 위해 FCFS 큐가 병행 운영.

**NOOP scheduler.**
- FCFS + 인접 요청 병합 — 재정렬 없음.
- CPU-bound 시스템과 특히 seek 최적화가 없는 SSD에 적합.

**CFQ (Completely Fair Queueing).**
- 일부 배포판에서 SATA HDD의 기본.
- 세 우선순위 큐: Real-time > Best-effort > Idle.
- 프로세스별 참조 지역성을 활용하기 위해 프로세스별 큐 유지.

---

<br>

## 5. 오류 탐지와 정정

### 5.1 Parity, CRC, ECC

저장되거나 전송되는 데이터는 비트 오류를 입을 수 있음. 방어책:

| 기법             | 기능                              | 설명                                                              |
|------------------|-----------------------------------|-------------------------------------------------------------------|
| **Parity bit**   | 탐지                              | 단일 비트 오류 탐지, 정정 불가                                   |
| **CRC**          | 무결성 검증                       | Cyclic Redundancy Check, 다항식 기반, 다중 비트 오류 탐지        |
| **ECC**          | 탐지 + 정정                       | Error-Correcting Code; 적은 수의 비트 오류 위치 파악 및 정정 가능 |

```
  Parity (짝수 패리티):
  Data: 10110101 → 1 개수 = 5 (홀수)
  Parity bit = 1  → 10110101 [1]   (총 1 개수가 짝수가 됨)

  ECC:
  Data + 다중 check bits → 나쁜 비트 위치 재구성
  Soft error: ECC로 정정 가능 → 복구 가능
  Hard error: 정정 불가능한 너무 많은 나쁜 비트 → 데이터 손실

  CRC:
  데이터를 다항식으로 취급; 고정 생성 다항식으로 나눈 나머지 저장
  네트워크 프로토콜에서 가장 널리 사용
```

현대 저장 장치는 sector / page 수준에서 자동 오류 탐지/정정을 위해 **내장 ECC** 보유.

### 5.2 실전에서의 ECC

**HDD:**
- 쓰기 시 각 sector마다 ECC가 계산되어 함께 저장.
- 읽기 시 ECC를 재계산해 비교.
- 불일치 → 오류 탐지. 몇 비트만 잘못되었으면 ECC가 자동 정정.

**SSD (NVM):**
- 페이지당 ECC 저장.
- 어떤 페이지가 *자주* 정정 가능 오류를 내면 컨트롤러가 그 페이지를 **bad** 로 표시하고 쓰기를 중단.

```
  Write:  Data → [ECC 계산] → [Data + ECC] → 저장
  Read:   저장 → [Data + ECC] → [ECC 재계산]
                                       │
                       일치? ──→ Yes: 정상 반환
                              └→ No:  정정 가능? ──→ Yes: Soft Error (정정, 반환)
                                                  └→ No:  Hard Error (I/O 오류 보고)
```

**Consumer vs Enterprise.** 엔터프라이즈 제품은 더 강력한 ECC 방식 사용 (Reed–Solomon, LDPC), 더 많은 중복 비트를 더 높은 정정 능력과 교환 — 데이터 센터가 요구하는 더 높은 신뢰성에 필요.

---

<br>

## 6. 저장 장치 관리

### 6.1 드라이브 포매팅

새 디스크를 사용 가능하게 만드는 3단계 과정:

```
  Low-level Formatting  →  Partitioning  →  Logical Formatting
  (물리 sector 구조 생성)  (디스크 논리 분할)  (파일 시스템 생성)
```

| 단계                       | 설명                                                                       |
|----------------------------|----------------------------------------------------------------------------|
| **Low-level format**       | sector당 header + data area + trailer (ECC) 생성. 공장에서 수행            |
| **Partitioning**           | 디스크를 하나 이상의 파티션으로 분할 (Linux `fdisk` 등)                     |
| **Logical formatting**     | 파티션에 파일 시스템 생성 (FAT, ext4, NTFS, APFS, …)                       |

**Low-level format 레이아웃:**

```
  ┌──────────┬────────────┬──────────┐
  │  Header  │  Data area │  Trailer │
  │ (sector#)│ (512B / 4KB)│  (ECC)  │
  └──────────┴────────────┴──────────┘
```

대부분의 드라이브는 low-level 포매팅이 공장에서 완료된 상태로 출하. 사용자는 partitioning + logical formatting만 수행.

### 6.2 파티션과 볼륨

**Partition.** 독립 장치로 사용 가능한 디스크의 논리 분할 영역. Linux: `/dev/sda1`, `/dev/sda2`, ...

**Volume.** 단일 파티션 또는 여러 파티션/장치를 결합한 논리 저장 단위.
- **Linux LVM** 은 여러 디스크를 단일 볼륨으로 결합 가능.
- **ZFS** 는 볼륨 관리와 파일 시스템을 한 계층으로 통합.

```
  물리 디스크
  ┌──────────┬──────────┬──────────┐
  │  Part 1  │  Part 2  │  Part 3  │
  │  (ext4)  │  (swap)  │  (ext4)  │
  └──────────┴──────────┴──────────┘

  Linux LVM:
  ┌── PV1 ──┐ ┌── PV2 ──┐
  │ Disk 1   │ │ Disk 2   │  →  Volume Group (VG)  →  Logical Volume (LV)
  └──────────┘ └──────────┘
```

**Raw I/O.** **파일시스템 없이** 블록 장치에 직접 접근. 데이터베이스 (특히 Oracle)가 가끔 선호 — 성능을 위해 자기 레이아웃을 직접 관리.

### 6.3 부트 블록

부트 시퀀스:

1. 전원 on → **펌웨어** (마더보드의 NVM flash에 저장된 부트스트랩 로더) 실행.
2. 펌웨어가 부트 저장 장치에서 **부트 블록** (MBR 또는 GPT) 읽음.
3. 부트 블록의 코드가 **OS 커널** 적재 및 실행.

- **부트 디스크 / 시스템 디스크:** 부트 파티션을 담은 디스크.
- 기본 Linux 부트스트랩 로더: **GRUB2**.
- 펌웨어 자체가 멀웨어에 감염될 수 있음 → 공급망의 실제 보안 우려.

### 6.4 불량 블록 관리

**불량 블록의 원인:**
- 제조 결함 (불량으로 출하).
- 사용 중 물리 손상 (head crash 등).
- 자성 물질의 노화 관련 열화.

**방법 1 — Sector sparing (forwarding).**
- 컨트롤러가 불량 sector를 low-level 포매팅 중 예약한 **예비 sector** 로 교체.
- 같은 cylinder 내 예비를 선호 (재매핑의 최소 seek 패널티).

```
  LBA 87 요청 → 컨트롤러 ECC 확인 → 불량 sector!
  → 예비 sector로 자동 리디렉트
  → 미래의 모든 LBA 87 요청이 예비로 매핑
```

**방법 2 — Sector slipping.**
- sector가 불량이 되면 이후 모든 sector를 한 위치씩 이동.
- 물리 레이아웃 연속성 유지 — 하지만 수행 시간 길다.

**NVM 장치.** 불량 페이지는 테이블에 기록되어 미래 할당에서 제외. Seek 우려 없음; 재매핑은 본질적으로 무비용.

---

<br>

## 7. Swap-Space 관리

### 7.1 목적과 크기 설정

**목적.** 가상 메모리를 확장하는 디스크 공간: 물리 메모리가 가득 차면 페이지를 디스크로 보냄 (**swap out**), 필요할 때 다시 가져옴 (**swap in**). 현대 OS는 전체 프로세스가 아닌 **페이지 단위로** swap (12주차).

**크기 설정.**
- *과소 할당:* 시스템이 swap을 다 써서 프로세스를 종료 (또는 crash).
- *과다 할당:* 디스크 공간 낭비 외 무해.
- 옛 Linux 경험칙: 물리 메모리의 2배.
- 현대 시스템: RAM이 넉넉하므로 훨씬 적게.

```
  물리 메모리                Swap 공간 (디스크)
  ┌────────────────┐       ┌────────────────┐
  │  활성 페이지   │       │  swap된 페이지  │
  │                │ ←──→  │                │
  └────────────────┘       └────────────────┘
```

### 7.2 위치 — 파티션 vs 파일

| 방법                  | 장점                                  | 단점                                   |
|-----------------------|---------------------------------------|----------------------------------------|
| 별도 raw 파티션       | 빠름 (파일시스템 오버헤드 없음)        | 고정 크기, 낮은 유연성                 |
| 파일 기반 (swap 파일) | 크기 조정 쉬움                        | 파일시스템 오버헤드                    |

**Linux** 는 둘 다 지원, 여러 swap 공간을 여러 디스크에 분산 가능 (더 나은 I/O 대역폭):

```bash
# swap 파티션 생성 및 활성화
mkswap /dev/sda2
swapon /dev/sda2

# swap 파일 생성 및 활성화
dd if=/dev/zero of=/swapfile bs=1M count=4096
mkswap /swapfile
swapon /swapfile
```

**Windows** 는 단일 관리 파일 `pagefile.sys` 사용; 크기는 자동.

### 7.3 Linux Swap 내부 구조

- **익명 메모리** 만 swap 대상 (코드 페이지는 실행 파일에서 다시 읽을 수 있음).
- Swap 영역은 4 KB **page slot** 의 배열로 조직.
- **Swap map**: page slot당 정수 카운터의 배열.

| 카운터 값 | 의미                                          |
|-----------|-----------------------------------------------|
| 0         | slot이 비어 있음 (사용 가능)                    |
| 1         | 한 프로세스가 그 swap된 페이지를 사용           |
| N (> 1)   | N개 프로세스가 그 페이지 공유 (공유 메모리)     |

카운터가 0으로 떨어지면 slot은 회수 가능. 이는 본질적으로 swap 페이지의 작은 참조 카운트 — 12주차의 페이지 테이블 수준 COW 로직을 반영.

---

<br>

## 8. 저장 장치 부착

### 8.1 개요

| 방법                | 설명                                              | 프로토콜               |
|---------------------|---------------------------------------------------|------------------------|
| **Host-Attached**   | SATA / SAS / NVMe로 직접 연결                      | AHCI, NVMe             |
| **NAS**             | 네트워크 위로 파일 시스템 접근                     | NFS, CIFS/SMB          |
| **Cloud Storage**   | 인터넷 기반 저장 서비스                            | REST API (S3 등)       |
| **SAN**             | **전용** 네트워크 위로 블록 수준 접근              | Fibre Channel, iSCSI   |

```
  Host-Attached: 서버 ──[SATA / SAS / NVMe]──→ 디스크
  NAS:           서버 ──[LAN (TCP/IP)]──→ NAS 장치 (파일 수준)
  SAN:           서버 ──[전용 네트워크]──→ 저장 어레이 (블록 수준)
  Cloud:         서버 ──[인터넷]──→ 클라우드 제공자 (API)
```

### 8.2 NAS

**Network-Attached Storage.** 일반 LAN 위에서 파일 수준 접근.

- 프로토콜: **NFS** (UNIX / Linux), **CIFS / SMB** (Windows).
- RPC 기반 통신.
- 파일 잠금 지원 → 여러 클라이언트가 안전하게 파일 공유.

**iSCSI.** SCSI 프로토콜을 IP 네트워크 위로 전달해 **블록 수준** 접근 (NFS/CIFS의 파일 수준과 달리). 호스트는 원격 저장을 직접 부착된 것처럼 취급.

NAS 장점: 설치 쉬움, 저비용.
NAS 단점: 저장 I/O가 일반 LAN 트래픽과 대역폭 경쟁; 부하 하에 병목 가능.

### 8.3 SAN

**Storage-Area Network.** 서버와 저장 장치를 **전용** 고성능 네트워크로 연결.

| 항목              | NAS                  | SAN                   |
|-------------------|----------------------|-----------------------|
| 접근 수준         | 파일 (파일 수준)     | 블록 (블록 수준)      |
| 네트워크          | 일반 LAN             | **전용**              |
| 프로토콜          | NFS, CIFS            | FC, iSCSI             |
| 비용              | 낮음                 | 비쌈                  |
| 성능              | 평균                 | **높음**              |
| 유연성            | 낮음                 | 높음 (동적 할당)      |

SAN은 저장을 특정 서버에 동적으로 할당/해제 가능. **JBOD** (Just a Bunch of Disks)는 SAN 위의 가장 단순한 디스크 어레이 구성 — RAID 없이 raw 인클로저.

### 8.4 클라우드 스토리지

**인터넷** 위로 원격 저장.

| 속성             | NAS                       | Cloud Storage                  |
|------------------|---------------------------|--------------------------------|
| 네트워크         | LAN                       | **인터넷 / WAN**               |
| 접근 방식        | 파일 시스템 프로토콜      | **API 기반** (REST / SDK)      |
| 장애 처리        | 단절 시 행                | 앱이 일시중지 후 재개          |
| 비용 모델        | 장비 구매                 | **사용량 기반 청구**           |

대표 서비스: Amazon S3, Microsoft OneDrive / Azure Blob, Google Cloud Storage, Apple iCloud, Dropbox.

**왜 API 기반?** 광역 네트워크는 높고 가변적인 지연과 빈번한 단절을 가짐. 파일 시스템 프로토콜 (NFS/CIFS)은 LAN 조건에 최적화되어 공용 인터넷을 가로질러 빈약하게 동작 — REST API는 명시적으로 재시도, 최종 일관성, 단절 허용 클라이언트 코드를 위해 설계.

---

<br>

## 9. RAID

### 9.1 목적과 구성 요소

**RAID** = **R**edundant **A**rray of **I**ndependent **D**isks. 여러 물리 디스크를 결합해 향상:
- **신뢰성** — 중복성을 통한 디스크 장애 생존.
- **성능** — 디스크 간 병렬 I/O.

세 가지 기법이 RAID 레벨에서 혼합/매칭:

| 기법         | 설명                                              | 얻는 것                       |
|--------------|---------------------------------------------------|-------------------------------|
| **Striping** | 데이터를 여러 디스크에 분산                        | 성능                          |
| **Mirroring**| 데이터의 중복 사본 저장                            | 신뢰성                        |
| **Parity**   | 복구 정보 계산 및 저장 (XOR 등)                    | 공간 효율적 신뢰성            |

```
  Striping:   [A1][A2][A3][A4]  →  Disk0[A1]  Disk1[A2]  Disk2[A3]  Disk3[A4]
  Mirroring:  [A1]              →  Disk0[A1]  Disk1[A1]   (동일 사본)
  Parity:     [A1][A2][P]       →  Disk0[A1]  Disk1[A2]  Disk2[A1 XOR A2]
```

### 9.2 중복성을 통한 신뢰성 — MTBF 수학

단일 디스크의 **MTBF (Mean Time Between Failures)** ≈ 100,000 시간.

**100 디스크 어레이** 에서 주어진 시간에 *어떤* 디스크라도 장애날 확률은 100배 더 높음 → 어레이 MTBF = $100{,}000 / 100 = 1{,}000$ 시간, 약 **42 일**.

디스크가 많을수록 어레이의 장애율이 *상승* — 그래서 큰 어레이에는 중복성이 필수.

**Mirrored volume MTBF (독립성 가정):**
- 디스크당 MTBF: 100,000 시간.
- Mean time to repair (MTTR): 10 시간.
- 첫 번째 디스크의 10시간 수리 창 동안 두 번째 디스크가 죽어야만 데이터 손실:

$$
\text{MTBF}_{\text{데이터 손실}} = \frac{(10^5)^2}{2 \cdot 10} = 5 \cdot 10^8 \text{ 시간} \approx \mathbf{57{,}000\text{ 년}}.
$$

**주의** — 독립성 가정이 실전에서 깨짐:
- 전력 서지, 자연 재해, 제조 배치 결함 → 동시 장애.
- 노화 디스크는 상관된 장애 시점 (한 장애가 다른 것을 예고).
- 긴 재구축 동안 생존 디스크가 스트레스 받아 장애 가능성 증가.

이것이 RAID 6 (§9.8)가 발명된 실제 이유 — 재구축 창 동안의 두 번째 장애를 견디기 위함.

### 9.3 Striping을 통한 성능

**Bit-level striping.** 각 바이트의 비트를 여러 드라이브에 분산. 8 드라이브면 비트 $i$ 가 드라이브 $i$ 로. 모든 접근이 병렬로 모든 드라이브 사용 → 8배 전송 속도, 그러나 별도 작은 접근에 대한 병렬성은 없음.

**Block-level striping (가장 일반적).** $N$ 드라이브에서 블록 $i$ 가 드라이브 $i \bmod N$ 로.

```
  4-Drive Block Striping:

  Block 0 → Disk 0    Block 1 → Disk 1
  Block 2 → Disk 2    Block 3 → Disk 3
  Block 4 → Disk 0    Block 5 → Disk 1
  ...
```

두 가지 병렬성 목표:
1. **처리율 개선** — 많은 작은 요청을 다른 디스크가 동시에 서비스.
2. **응답 시간 단축** — 하나의 큰 요청을 디스크 간 분할해 병렬 실행.

### 9.4 RAID 0 — Striping 전용

```
  Disk 0    Disk 1    Disk 2    Disk 3
  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐
  │  A1  │  │  A2  │  │  A3  │  │  A4  │
  │  A5  │  │  A6  │  │  A7  │  │  A8  │
  │  A9  │  │ A10  │  │ A11  │  │ A12  │
  └──────┘  └──────┘  └──────┘  └──────┘
```

| 속성              | 상세                                                |
|-------------------|-----------------------------------------------------|
| 최소 디스크       | 2                                                   |
| 사용 가능 용량    | $N \times$ 디스크 크기 (100%)                       |
| Read 성능         | $N\times$                                           |
| Write 성능        | $N\times$                                           |
| 내결함성          | **없음** — 어떤 디스크 장애도 전체 데이터 손실      |
| 사용 사례         | 성능 임계, 손실 허용 (스크래치 공간)                |

순수 성능 최적화, 신뢰성 없음. 한 디스크 장애 시 **모든 것** 손실.

### 9.5 RAID 1 — Mirroring

```
  Disk 0    Disk 1    Disk 2    Disk 3
  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐
  │  A1  │  │  A1  │  │  A3  │  │  A3  │
  │  A2  │  │  A2  │  │  A4  │  │  A4  │
  └──────┘  └──────┘  └──────┘  └──────┘
   미러 쌍 1            미러 쌍 2
```

| 속성              | 상세                                                       |
|-------------------|------------------------------------------------------------|
| 최소 디스크       | 2                                                          |
| 사용 가능 용량    | $N/2 \times$ 디스크 크기 (50%)                             |
| Read 성능         | 개선 (양쪽에서 읽기 가능)                                   |
| Write 성능        | 변화 없음 (양쪽에 써야 함)                                  |
| 내결함성          | 미러 쌍당 1 디스크                                          |
| 재구축            | 가장 빠름 — 단순 복사                                       |

가장 단순한 중복성. 두 배 비용, 그러나 매우 빠른 재구축과 높은 신뢰성.

### 9.6 RAID 4 — Striping + 전용 Parity

```
  Disk 0    Disk 1    Disk 2    Parity
  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐
  │  A1  │  │  A2  │  │  A3  │  │  Ap  │   Ap = A1 ⊕ A2 ⊕ A3
  │  B1  │  │  B2  │  │  B3  │  │  Bp  │
  │  C1  │  │  C2  │  │  C3  │  │  Cp  │
  └──────┘  └──────┘  └──────┘  └──────┘
```

| 속성              | 상세                              |
|-------------------|-----------------------------------|
| 최소 디스크       | 3                                 |
| 사용 가능 용량    | $(N-1) \times$ 디스크 크기        |
| 내결함성          | 1 디스크                          |
| 복구              | parity로 역 XOR                    |

**복구:** Disk 1이 죽으면 $A_2 = A_1 \oplus A_3 \oplus A_p$.

**문제 — parity 병목.** 모든 쓰기가 parity 디스크에도 씀 → parity 디스크가 두들겨 맞고 데이터 디스크는 idle. 작은 쓰기는 read-modify-write (4 디스크 접근) 필요. 그래서 RAID 4는 실전에서 드물다.

### 9.7 RAID 5 — 분산 Parity

```
  Disk 0    Disk 1    Disk 2    Disk 3    Disk 4
  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐
  │  A1  │  │  A2  │  │  A3  │  │  A4  │  │  Ap  │
  │  B1  │  │  B2  │  │  B3  │  │  Bp  │  │  B4  │
  │  C1  │  │  C2  │  │  Cp  │  │  C3  │  │  C4  │
  │  D1  │  │  Dp  │  │  D2  │  │  D3  │  │  D4  │
  │  Ep  │  │  E1  │  │  E2  │  │  E3  │  │  E4  │
  └──────┘  └──────┘  └──────┘  └──────┘  └──────┘
```

**RAID 4와의 차이:** parity 블록이 **모든 디스크에 분산** (stripe 행당 하나). 단일 parity 디스크 없음 → 병목 없음.

| 속성              | 상세                                    |
|-------------------|-----------------------------------------|
| 최소 디스크       | 3                                       |
| 사용 가능 용량    | $(N-1) \times$ 디스크 크기              |
| 내결함성          | 1 디스크                                |
| 사용              | **가장 널리 배포된 parity RAID**         |

### 9.8 RAID 6 — 이중 Parity

```
  Disk 0    Disk 1    Disk 2    Disk 3    Disk 4    Disk 5
  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐
  │  A1  │  │  A2  │  │  A3  │  │  A4  │  │  Ap  │  │  Aq  │
  │  B1  │  │  B2  │  │  B3  │  │  Bp  │  │  Bq  │  │  B4  │
  │  C1  │  │  C2  │  │  Cp  │  │  Cq  │  │  C3  │  │  C4  │
  └──────┘  └──────┘  └──────┘  └──────┘  └──────┘  └──────┘
```

| 속성              | 상세                                              |
|-------------------|---------------------------------------------------|
| 최소 디스크       | 4                                                 |
| 사용 가능 용량    | $(N-2) \times$ 디스크 크기                        |
| 내결함성          | **2 디스크**                                      |
| 이중 parity       | P (XOR) + Q (Galois field 산술)                   |

- P와 Q는 **서로 다른** 수학적 연산으로 계산되어야 함 — 동일하다면 중복성이 추가 복구 정보를 주지 않음.
- 쓰기 오버헤드가 RAID 5보다 높음 (쓰기당 2 parity 갱신).
- **대용량 어레이의 표준** — 단일 장애 후 긴 재구축 시간이 재구축 중 두 번째 장애를 현실적 위협으로 만듦.

### 9.9 RAID 0+1 vs RAID 1+0 (RAID 10)

RAID 0 (striping)과 RAID 1 (mirroring)의 다른 계층 순서 조합.

- **RAID 0+1:** 먼저 striping, 그 다음 stripe를 미러. 단일 디스크 장애가 미러 전체 stripe를 깸 — 어레이의 절반이 오프라인.
- **RAID 1+0 (RAID 10):** 먼저 미러, 그 다음 미러 쌍을 stripe. 단일 디스크 장애는 한 미러만 깸 — 어레이의 나머지는 무사하고 한 디스크만 재구축.

**RAID 10이 선호됨** — 단일 장애의 폭발 반경이 작고 재구축 대상이 단일 디스크 → 더 빠른 복구.

### 9.10 종합 비교

| 레벨    | 구성                                  | 신뢰성             | 성능                | 최소 디스크 | 사용 가능 용량 |
|---------|---------------------------------------|--------------------|---------------------|-------------|----------------|
| RAID 0  | Striping 전용                          | 없음               | 가장 높음           | 2           | 100%            |
| RAID 1  | Mirroring                              | 1 디스크           | Read 개선           | 2           | 50%             |
| RAID 4  | Striping + 전용 parity                 | 1 디스크           | Parity 병목         | 3           | $(N-1)/N$       |
| RAID 5  | Striping + 분산 parity                 | 1 디스크           | RAID 4 대비 개선    | 3           | $(N-1)/N$       |
| RAID 6  | RAID 5 + 이중 parity                    | **2 디스크**       | 더 높은 write 오버헤드 | 4         | $(N-2)/N$       |
| RAID 10 | 미러 + stripe (1+0)                     | 쌍당 1             | 훌륭한 R/W          | 4           | 50%             |

**RAID 레벨 선택:**
- **RAID 0:** 최대 성능, 데이터 손실 수용.
- **RAID 1:** 빠른 재구축 + 높은 신뢰성, 비용 수용.
- **RAID 5:** 공간 효율적, 중간 규모 저장.
- **RAID 6:** 대용량 어레이, 최고 신뢰성.
- **RAID 10:** 성능 *과* 신뢰성 모두 (DB 서버, 가상화 호스트).

### 9.11 Hot Spare와 구현 계층

**Hot spare.** 데이터를 담지 **않지만** 장애 발생 시 **자동으로 교체로 배치** 되는 디스크. Hot spare가 있는 RAID 5에서 사람 개입 없이 재구축이 즉시 시작.

**RAID 구현 계층:**

| 계층                  | 설명                                                                |
|-----------------------|---------------------------------------------------------------------|
| **Software RAID**     | OS 커널이나 볼륨 매니저에 구현 (Linux `md` / `mdadm`)                |
| **HBA**               | 하드웨어 RAID 컨트롤러 카드                                          |
| **Storage Array**     | 외부 저장 장치 내 전용 컨트롤러 (가장 일반적)                         |
| **SAN Interconnect**  | 호스트와 저장 사이의 가상화 장치가 제공하는 RAID                       |

**추가 기능:**
- **Snapshot:** 특정 시점의 파일 시스템 상태 캡처 (COW 덕분에 저렴).
- **Replication:** 재해 복구를 위해 데이터를 원격 사이트로 자동 복제.
  - *동기:* 양쪽이 쓴 뒤 ack → 데이터 손실 없음, 그러나 더 높은 지연.
  - *비동기:* 주기적 일괄 → 소스가 죽으면 일부 데이터 손실 가능, 그러나 훨씬 빠름.

### 9.12 RAID가 보호하지 못하는 것 — ZFS

RAID는 **물리 매체 오류** 를 보호. 다음을 *보호하지 않음*:
- 소프트웨어 버그로 인한 파일 포인터 / 메타데이터 오류.
- **Torn write** — 전원 손실 동안 불완전한 쓰기.
- RAID 컨트롤러 버그 / 장애.
- ECC가 잡는 것 너머의 조용한 비트 부패(bit rot).

**ZFS** 는 다른 접근:
- **모든 블록 (데이터와 메타데이터)** 의 checksum 저장.
- 결정적으로, checksum은 블록 자체가 아닌 **부모 포인터** 에 저장 — 손상된 블록이 자기 checksum에 대해 거짓말할 수 없음.

읽기 시:
- 데이터 → checksum 재계산 → 부모의 저장된 checksum과 비교.
- 불일치 → 손상 탐지.
- 미러 또는 parity 사본이 있으면 무사한 사본에서 자동 복구.

ZFS는 또한 **볼륨 관리와 파일 시스템** 을 통합 저장 풀로 결합 — 전통적 스택을 복잡하게 만드는 볼륨-매니저/파일시스템 계층 불일치 제거.

---

<br>

## 10. 실습 — 디스크 스케줄링 시뮬레이터

FCFS, SCAN, C-SCAN, LOOK, C-LOOK을 Python으로 구현해 정석 요청 큐에서 head 이동 비교.

**입력:** 요청 큐 (cylinder 번호 리스트), 현재 head 위치, 디스크 크기, 초기 방향 (SCAN/C-SCAN용).

```python
def fcfs(requests, head):
    """FCFS: 도착 순서대로 요청 처리"""
    sequence = [head]
    total = 0
    current = head
    for req in requests:
        total += abs(current - req)
        sequence.append(req)
        current = req
    return sequence, total
```

```python
def scan(requests, head, disk_size, direction="up"):
    """SCAN: 한 방향 끝까지, 그 다음 반전"""
    sequence = [head]
    total = 0
    current = head
    left  = sorted(r for r in requests if r < head)
    right = sorted(r for r in requests if r >= head)
    run = right + [disk_size - 1] + left[::-1] if direction == "up" \
        else left[::-1] + [0] + right
    for pos in run:
        total += abs(current - pos)
        sequence.append(pos)
        current = pos
    return sequence, total
```

```python
def c_scan(requests, head, disk_size):
    """C-SCAN: 한 방향만 서비스, 시작으로 wrap"""
    sequence = [head]; total = 0; current = head
    left  = sorted(r for r in requests if r < head)
    right = sorted(r for r in requests if r >= head)
    run = right + [disk_size - 1] + [0] + left
    for pos in run:
        total += abs(current - pos)
        sequence.append(pos)
        current = pos
    return sequence, total
```

LOOK과 C-LOOK은 디스크 끝 여행을 제거한 같은 것.

**드라이버와 기대 출력:**

```python
requests = [98, 183, 37, 122, 14, 124, 65, 67]
head, disk_size = 53, 200

for name, fn in [
    ("FCFS",         lambda: fcfs(requests, head)),
    ("SCAN (down)",  lambda: scan(requests, head, disk_size, "down")),
    ("C-SCAN (up)",  lambda: c_scan(requests, head, disk_size)),
    ("LOOK (down)",  lambda: look(requests, head, "down")),
    ("C-LOOK (up)",  lambda: c_look(requests, head)),
]:
    seq, dist = fn()
    print(f"{name}: distance={dist}, path={seq}")
```

기대 총합: **FCFS 640, SCAN 236, C-SCAN 382, LOOK 208, C-LOOK 330**.

**확장:** matplotlib으로 각 알고리즘의 head 이동 경로 시각화.

---

<br>

## 요약

| 주제                     | 핵심 내용                                                              |
|--------------------------|------------------------------------------------------------------------|
| **HDD 구조**             | Platter, track, sector, cylinder, RPM                                  |
| **접근 시간**            | Seek + rotational latency + transfer (seek 지배)                        |
| **NVM / SSD**            | NAND Flash, FTL, garbage collection, wear leveling, TRIM               |
| **NAND 세부**            | 페이지 단위 R/W, 블록 단위 erase, 덮어쓰기 불가                          |
| **HDD 스케줄링**         | FCFS, SCAN, C-SCAN, LOOK, C-LOOK                                       |
| **Linux 스케줄러**       | Deadline, NOOP, CFQ                                                    |
| **오류 처리**            | Parity, CRC, ECC; soft vs hard error                                   |
| **포매팅**               | Low-level → partitioning → logical formatting                           |
| **부트 블록**            | MBR/GPT → bootstrap → OS 커널                                            |
| **불량 블록**            | Sector sparing, sector slipping                                         |
| **Swap 공간**            | 가상 메모리 확장; 파티션 vs 파일; slot별 참조 카운트                     |
| **저장 부착**            | Host-attached, NAS, SAN, 클라우드                                       |
| **RAID**                 | Striping, mirroring, parity로 성능과 신뢰성                              |
| **RAID 레벨**            | 0, 1, 4, 5, 6, 10 — 사용 사례별 선택                                     |
| **RAID 너머**            | ZFS 식 checksum이 RAID가 못 잡는 것을 잡음                              |

**핵심 정리:**
- **HDD에서는 seek time이 지배적 비용** — 그래서 스케줄링, 인덱싱, SSD 모두 중요.
- **SSD는 근본적으로 다른 저장 장치** — 단순한 "빠른 HDD" 가 아님 — 덮어쓰기 불가 속성이 전체 컨트롤러 스택(FTL, GC, wear leveling)을 재구성.
- **HDD 스케줄링 = 엘리베이터 비유** — SCAN과 LOOK이 무작위 암 휘젓기를 부드러운 단방향 스윕으로; 균일 지연(C-variant) vs 최소 총 이동에 따라 선택.
- **RAID 레벨은 용량을 신뢰성/성능과 교환** — "최선" 레벨이 없고, 워크로드에 맞는 옳은 레벨만 있음.
- **RAID의 사각지대가 ZFS 식 end-to-end checksum의 동기** — 물리 매체 중복성은 장치 계층 위의 손상을 잡지 못함.
- **Swap, RAID, 저장 부착 모두 동일 OS 저장 스택을 통과** — 사용자 공간에 노출되는 것은 단지 "블록 장치", 그러나 그 아래의 모든 것이 흥미롭다.

> "모든 저장은 평탄하고 무한하며 신뢰할 수 있는 바이트 배열에 대한 거짓말이다. OS의 일은 그 거짓말을 신뢰할 만하게 유지하는 것이다."

**다음 주:** 보안과 보호 — CIA 트라이어드, 위협, 암호학, 접근 제어, 인증.

---

<br>

## 점검 문제

1. **접근 시간 산술:** 10,000 RPM 드라이브, 평균 seek 5 ms, 4 KB sector. 기대 접근 시간과 결과 랜덤 IOPS를 계산하라.

   > **정답:** 1회전 = $60{,}000 / 10{,}000 = 6$ ms; 평균 rotational latency = $6 / 2 = 3$ ms. 4 KB에 대한 ~150 MB/s에서 transfer time ≈ 0.027 ms (무시 가능). 총 접근 시간 ≈ $5 + 3 + 0.027 \approx 8\ \text{ms}$. 랜덤 IOPS = $1000 / 8 \approx 125$. 비교해 SSD는 100K+ IOPS — ~1000배 격차가 지연 민감 워크로드에서 "SSD인가?" 가 첫 질문인 이유.

2. **왜 덮어쓰기 불가?** NAND Flash가 페이지를 직접 덮어쓸 수 없는 이유와 그 결과의 연쇄 (FTL, garbage collection, write amplification) 를 설명하라.

   > **정답:** NAND 셀의 program 연산은 비트를 한 방향으로만 뒤집을 수 있음 (보통 페이지 내 1 → 0). 비트를 다시 1로 설정하려면 **erase** 가 필요, 이는 큰 블록(많은 페이지)을 물리적으로 리셋 — 느리고 셀에 스트레스. 따라서 페이지 "재기록" 은 (1) 새 내용을 다른 free 페이지에 쓰고, (2) FTL 매핑에서 옛 페이지를 무효로 표시, (3) 결국 블록의 많은 페이지가 무효가 되면 **garbage collector** 가 아직 유효한 페이지를 다른 곳으로 복사하고 블록을 erase. 매핑 계층 (**FTL**) 이 이를 OS에 숨김. 단점: 한 논리 쓰기가 내부적으로 추가 read/write/erase 유발 → **write amplification**, SSD 내구성은 쓰기 사이클로 측정되므로 amplification이 직접 수명을 단축.

3. **TRIM 중요성:** TRIM 없으면 "오래된" SSD의 쓰기 성능이 왜 저하되는가?

   > **정답:** TRIM 없이는 사용자가 파일을 삭제할 때 OS가 파일시스템 메타데이터만 갱신 — SSD는 그 LBA가 쓰레기를 담고 있음을 알 방법이 없음. SSD 관점에서 모든 블록이 유효해 보이는 데이터로 "사용 중". 그래서 사용자가 나중에 "free" LBA에 쓰면 SSD는 (1) 기존 블록 데이터를 읽고, (2) 수정, (3) 옛 블록을 erase, (4) 새 버전 쓰기 — 무거운 read-modify-erase-write 사이클. TRIM과 함께라면 OS가 SSD에 "이 LBA는 이제 쓰레기" 라고 알림 → 컨트롤러가 그 페이지를 즉시 무효로 표시, idle 시간에 GC 실행, 이후 쓰기에 on-demand erase 작업 없이 응답 가능. 결과: TRIM 활성 SSD는 수명 내내 신선한 쓰기 성능 유지.

4. **HDD 스케줄링 추적:** 큐 `82, 170, 43, 140, 24, 16, 190`, head 50, 디스크 0–199. FCFS, SCAN (초기 방향 up), C-LOOK (방향 up) 하의 총 이동을 계산하라.

   > **정답:** **FCFS:** $|50-82|+|82-170|+|170-43|+|43-140|+|140-24|+|24-16|+|16-190| = 32+88+127+97+116+8+174 = \mathbf{642}$. **SCAN up:** 위로 82, 140, 170, 190 거쳐 끝 199로, 그 다음 아래로 43, 24, 16: $(199-50)+(199-16) = 149 + 183 = \mathbf{332}$. **C-LOOK up:** 위로 190(가장 큼)까지 스윕, 16(가장 작음)으로 wrap, 그 다음 24, 43: $(190-50)+(190-16)+(43-16) = 140+174+27 = \mathbf{341}$. (또는 구간별로 계산 — 값은 구간별 산술의 반올림 내에서 일치.)

5. **SCAN vs C-SCAN 공정성:** SCAN이 라운드당 모든 cylinder를 두 번 방문하는데도 C-SCAN이 SCAN보다 더 균일한 대기 시간을 주는 이유는?

   > **정답:** SCAN 하에서 **가운데** cylinder는 라운드당 두 번 방문 (올라갈 때 한 번, 내려올 때 한 번), 끝 cylinder는 라운드당 한 번만 방문 — 그래서 끝 cylinder는 서비스 사이에 대략 두 배 오래 기다림. C-SCAN 하에서는 모든 cylinder가 라운드당 정확히 한 번 방문 (wrap 후 head는 시작에 도달할 때까지 아무것도 서비스 안 함), 그래서 모든 cylinder가 같은 서비스 간 간격을 봄. SCAN의 *평균* 대기가 더 짧지만 대기 시간의 **분산** 이 더 높음 — QoS 민감 워크로드에는 균일 지연이 더 낮은 평균 지연보다 낫다.

6. **Parity vs ECC 구분:** 친구가 "ECC는 그냥 더 많은 비트의 parity 야" 라고 한다. 다듬어라.

   > **정답:** Parity는 데이터 워드당 *한* 비트를 추가, *홀수* 개 비트 오류 탐지를 허용하지만 정정은 못함 (잘못된 것은 알지만 어디인지는 모름). ECC는 *여러* check 비트를 추가하여 나쁜 비트(들)의 위치를 재구성 — 실제 **정정**, 단순 탐지가 아닌 — 가능. Hamming 코드는 워드당 1 비트 정정; Reed-Solomon과 LDPC는 블록당 많은 비트 정정. "더 많은 비트" 표현은 기술적으로는 맞지만 질적 도약은 탐지에서 정정으로, 이는 시스템이 할 수 있는 것을 근본적으로 변화 (soft 오류를 조용히 고치기 vs I/O 실패 보고).

7. **부트 시퀀스:** 전원 on에서 OS 커널 실행까지의 단계를 MBR/GPT, GRUB, 펌웨어 이름을 들며 차례로 설명하라.

   > **정답:** (1) **전원 on** → CPU가 마더보드 NVM에 저장된 **펌웨어** (BIOS 또는 UEFI) 실행 시작. (2) 펌웨어가 하드웨어 초기화 (POST), 그 다음 부트 디스크에서 **부트 블록** 읽음 — 레거시 BIOS에서는 sector 0 (**MBR**), UEFI에서는 EFI System Partition의 파일 (**GPT** 포맷 디스크). (3) 부트 블록이 **1차 부트로더** (MBR에 대해 매우 작음, ~440 바이트) 포함. (4) 1차 부트로더가 디스크 다른 곳에서 **2차 부트로더** (Linux에서는 **GRUB2**) 적재. (5) GRUB이 메뉴 제시 후 **OS 커널** (예: `vmlinuz`) 더하기 **initrd** (최소 드라이버 RAM 디스크)를 메모리에 적재. (6) GRUB이 커널의 진입점으로 제어 이전. (7) 커널이 페이징 설정, 실제 root 파일시스템 마운트 (필요하면 initrd의 드라이버 사용), `init` / `systemd` 시작 — OS가 실행 중.

8. **현대 RAM에서의 swap 크기:** 32 GB RAM의 현대 시스템이 여전히 swap에서 이득을 보는 경우와 역효과인 경우는?

   > **정답:** **유익:** (a) **Hibernation** — 커널이 전원 off 전 RAM 내용을 swap에 씀; swap 없이는 hibernation 불가능. (b) **메모리 압박 꼬리** — 잘 자원 공급된 시스템에서도 가끔의 폭증(거대한 컴파일, 큰 DB 임포트)이 RAM을 잠깐 초과 가능; swap이 OOM kill 대신 이들을 느리게라도 완료시킴. (c) **차가운 페이지** — 오래 idle인 프로세스 페이지를 디스크로 옮겨 RAM을 활성 워크로드에 사용 가능. **역효과:** (a) 어떤 swap 활동도 대화형 반응성을 파괴하는 시스템 — OOM이 범인을 죽이는 게 나음. (b) 불필요한 쓰기가 수명을 단축하는 고내구성 민감 flash 시스템 (이게 모바일 OS가 swap 회피하는 *이유*, 12주차). (c) 워크로드가 진짜 RAM 필요할 때 RAM 추가 대체로서의 swap — swap이 시스템을 기술적으로 살릴 뿐 사용 불가능하게 느림.

9. **MTBF 수학:** 디스크당 MTBF = $10^5$ 시간, MTTR = 20 시간인 4-디스크 RAID 5 어레이. 독립성 하 어레이의 데이터 손실 MTBF는?

   > **정답:** RAID 5는 1 디스크 장애 견딤. 데이터 손실은 첫 장애의 20시간 수리 창 동안 두 번째 장애 필요. 디스크당 장애율 = 시간당 $1 / 10^5$. 4 디스크 중 임의의 것이 시간당 장애날 확률 ≈ $4 / 10^5$. 첫 장애 가정 시 **남은 3 디스크** 중 임의의 것이 20시간 내 장애날 확률 ≈ $3 \cdot 20 / 10^5 = 6 \cdot 10^{-4}$. 결합 위험률: $(4/10^5) \cdot (6 \cdot 10^{-4}) = 2.4 \cdot 10^{-8}$ 시간당. MTBF ≈ $1 / (2.4 \cdot 10^{-8}) \approx 4.2 \cdot 10^7$ 시간 $\approx$ **4,750 년**. 단서: 스트레스 받는 재구축 조건에서 3 생존 디스크에 대한 독립성은 1보다 훨씬 약한 가정; 실세계 MTBF는 극적으로 더 나쁨, 그것이 정확히 RAID 6 (2 장애 견딤)가 다루는 장애 모드.

10. **바쁜 DB에 RAID 5 vs RAID 10:** 금융 데이터베이스가 많은 작은 랜덤 쓰기 수행. 어떤 RAID 레벨을 골라야 하며 왜?

    > **정답:** **RAID 10.** 각 RAID 5 작은 쓰기는 데이터 *및* parity 블록 모두에 **read-modify-write** 사이클 필요 — 논리 쓰기당 4 디스크 접근, parity 계산이 임계 경로. 랜덤 쓰기 무거운 워크로드에서는 미러링 대비 달성 가능 IOPS를 반 또는 4분의 1로. **RAID 10** 은 미러 쌍의 각 디스크에 병렬로 씀 — 논리 쓰기당 2 쓰기, read-modify-write 없음, parity 병목 없음. 절충: RAID 10은 raw 용량의 50% 사용 (RAID 5의 $(N-1)/N$ 대비), 처리율에 대한 디스크 비용을 더 지불. OLTP 데이터베이스에는 IOPS 절약이 비용을 압도적으로 정당화.

11. **ZFS checksum 트릭:** ZFS가 블록 checksum을 블록 자체가 아닌 *부모 포인터* 에 저장하는 이유는? 어떤 공격/장애에 대해 방어하는가?

    > **정답:** checksum이 블록 내부에 산다면, 데이터와 checksum 둘 다 뒤집는 하드웨어 손상 (가능 — 둘 다 같은 디스크 sector / 페이지에) 이 탐지 안 됨, 손상된 블록이 자기 자신의 손상된 checksum 에 대해 "유효" 하기 때문. checksum을 **부모 포인터** — 다른 온디스크 위치, 다른 블록에 — 에 저장함으로써 데이터 블록의 손상이 그 checksum도 손상시킬 수 없음. 읽기 시 ZFS가 부모 포인터 fetch, checksum 찾고, 그 다음 데이터 블록 읽고 검증. 방어는 비트 부패 너머로 확장 — 잘못된 데이터를 조용히 반환하는 **펌웨어 버그**, 바이트를 잘못된 sector에 두는 **misdirected write**, 컨트롤러가 ack 했지만 디스크에 결코 도달하지 않은 **lost write** — RAID 단독으로는 보이지 않는 장애 모드.

12. **저장 부착 선택:** 스타트업이 8 웹 서버와 2 배치 처리 서버에 의해 접근 가능한 공유 저장이 필요. 이 사용 사례에 NAS, SAN, 클라우드 스토리지 비교.

    > **정답:** **NAS** (예: NFS) 가 자연스러운 적합: 10 서버가 LAN 위로 파일 공유; 설치 단순; 비용 낮음; 표준 `mount -t nfs`. 성능은 웹 페이로드엔 괜찮지만 큰 파일을 동시에 읽는 배치 작업에서 병목 가능. **SAN** 은 전용 FC/iSCSI 링크로 블록 수준 접근 줄 것 — 10 서버에 과잉이고 비쌈; SAN은 높은 IOPS 요구가 있는 100+ 서버 (DB 클러스터, 가상화 팜)에서 빛난다. **클라우드 스토리지** (예: S3)는 애플리케이션이 이미 클라우드 네이티브이거나 단일 데이터센터 너머의 내구성이 필요할 때 적합; API 기반 접근 모델이 앱 코드 변경 요구 (mount 없음), GB당 가격이 단일 선불 구매가 아닌 사용량에 따라 확장. 이 규모의 스타트업에는: hot 워크로드에는 NAS, 백업과 크고 드물게 접근되는 자산에는 클라우드 스토리지. SAN은 트래픽이 10× 증가하기 전까지는 과잉.
