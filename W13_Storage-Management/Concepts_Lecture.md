# Week 13 Lecture — Storage Management

> **Last Updated:** 2026-05-26
>
> Silberschatz, Operating System Concepts Ch 11 (Mass-Storage Structure)

> **Prerequisites**: Week 11 (main memory) and Week 12 (virtual memory) — swap-space management in §7 plugs directly into the demand-paging machinery from Week 12; the EAT / page-fault analysis there motivates why disk access time matters here. A computer-architecture background in DMA, controllers, and bus topology helps with §8. Basic discrete-math vocabulary: array, modular arithmetic (for striping), XOR (for parity).
>
> **Learning Objectives**:
> 1. Describe the **physical structure** of an HDD (platter, track, sector, cylinder, head, arm, spindle) and explain head crashes
> 2. Decompose **access time** into seek time, rotational latency, and transfer time, and compute typical totals
> 3. Distinguish **HDD** from **NVM/SSD** along mechanical, performance, and durability axes
> 4. Explain **NAND Flash** specifics: page-level R/W, block-level erase, no overwrite; and the controller mechanisms **FTL, garbage collection, over-provisioning, wear leveling, TRIM, write amplification**
> 5. Compare storage **connection interfaces** (SATA, NVMe, SAS, USB, Fibre Channel) and the meaning of **LBA** addressing (CHS, CAV vs CLV)
> 6. Execute and compare **HDD-scheduling algorithms**: FCFS, SCAN (elevator), C-SCAN, LOOK, C-LOOK
> 7. Describe **error detection and correction** mechanisms: parity, CRC, ECC, and their use in HDD/SSD
> 8. Walk through **disk formatting** (low-level → partitioning → logical formatting), volume management, **boot blocks**, and **bad-block handling** (sector sparing, sector slipping)
> 9. Describe **swap-space management** on Linux (partition vs file, swap map)
> 10. Distinguish **storage attachment** methods: host-attached, NAS, SAN, cloud storage
> 11. Explain **RAID 0/1/4/5/6/10** in terms of striping, mirroring, parity, fault tolerance, and use cases
> 12. Compute **MTBF for mirrored arrays** and explain when the independence assumption breaks
> 13. State what RAID does **not** protect against and how **ZFS-style checksums** close those gaps

---

## Table of Contents

- [1. HDD Structure and Access Time](#1-hdd-structure-and-access-time)
  - [1.1 Mass-Storage Overview](#11-mass-storage-overview)
  - [1.2 HDD Physical Structure](#12-hdd-physical-structure)
  - [1.3 Rotation Speed and Capacity](#13-rotation-speed-and-capacity)
  - [1.4 Access Time = Seek + Rotational + Transfer](#14-access-time--seek--rotational--transfer)
  - [1.5 Concrete Access-Time Example](#15-concrete-access-time-example)
- [2. NVM Devices — SSD and NAND Flash](#2-nvm-devices--ssd-and-nand-flash)
  - [2.1 HDD vs SSD](#21-hdd-vs-ssd)
  - [2.2 NAND Flash Characteristics](#22-nand-flash-characteristics)
  - [2.3 FTL, Garbage Collection, Over-Provisioning](#23-ftl-garbage-collection-over-provisioning)
  - [2.4 Wear Leveling, TRIM, Write Amplification](#24-wear-leveling-trim-write-amplification)
  - [2.5 NVM Scheduling](#25-nvm-scheduling)
  - [2.6 Volatile Memory as Storage (RAM Drives)](#26-volatile-memory-as-storage-ram-drives)
- [3. Connection and Address Mapping](#3-connection-and-address-mapping)
  - [3.1 Connection Interfaces](#31-connection-interfaces)
  - [3.2 Logical Block Address (LBA)](#32-logical-block-address-lba)
- [4. HDD Scheduling Algorithms](#4-hdd-scheduling-algorithms)
  - [4.1 Goals and Request Structure](#41-goals-and-request-structure)
  - [4.2 FCFS](#42-fcfs)
  - [4.3 SCAN — The Elevator Algorithm](#43-scan--the-elevator-algorithm)
  - [4.4 C-SCAN — Circular SCAN](#44-c-scan--circular-scan)
  - [4.5 LOOK and C-LOOK](#45-look-and-c-look)
  - [4.6 Comparison on the Same Queue](#46-comparison-on-the-same-queue)
  - [4.7 Linux I/O Schedulers](#47-linux-io-schedulers)
- [5. Error Detection and Correction](#5-error-detection-and-correction)
  - [5.1 Parity, CRC, ECC](#51-parity-crc-ecc)
  - [5.2 ECC in Practice](#52-ecc-in-practice)
- [6. Storage Device Management](#6-storage-device-management)
  - [6.1 Drive Formatting](#61-drive-formatting)
  - [6.2 Partitions and Volumes](#62-partitions-and-volumes)
  - [6.3 Boot Block](#63-boot-block)
  - [6.4 Bad-Block Management](#64-bad-block-management)
- [7. Swap-Space Management](#7-swap-space-management)
  - [7.1 Purpose and Sizing](#71-purpose-and-sizing)
  - [7.2 Location — Partition vs File](#72-location--partition-vs-file)
  - [7.3 Linux Swap Internal Structure](#73-linux-swap-internal-structure)
- [8. Storage Attachment](#8-storage-attachment)
  - [8.1 Overview](#81-overview)
  - [8.2 NAS](#82-nas)
  - [8.3 SAN](#83-san)
  - [8.4 Cloud Storage](#84-cloud-storage)
- [9. RAID](#9-raid)
  - [9.1 Purpose and Building Blocks](#91-purpose-and-building-blocks)
  - [9.2 Reliability via Redundancy — MTBF Math](#92-reliability-via-redundancy--mtbf-math)
  - [9.3 Performance via Striping](#93-performance-via-striping)
  - [9.4 RAID 0 — Striping Only](#94-raid-0--striping-only)
  - [9.5 RAID 1 — Mirroring](#95-raid-1--mirroring)
  - [9.6 RAID 4 — Striping + Dedicated Parity](#96-raid-4--striping--dedicated-parity)
  - [9.7 RAID 5 — Distributed Parity](#97-raid-5--distributed-parity)
  - [9.8 RAID 6 — Dual Parity](#98-raid-6--dual-parity)
  - [9.9 RAID 0+1 vs RAID 1+0 (RAID 10)](#99-raid-01-vs-raid-10-raid-10)
  - [9.10 Comprehensive Comparison](#910-comprehensive-comparison)
  - [9.11 Hot Spare and Implementation Layers](#911-hot-spare-and-implementation-layers)
  - [9.12 What RAID Doesn't Protect Against — ZFS](#912-what-raid-doesnt-protect-against--zfs)
- [10. Lab — Disk-Scheduling Simulator](#10-lab--disk-scheduling-simulator)
- [Summary](#summary)
- [Self-Check Questions](#self-check-questions)

---

<br>

## 1. HDD Structure and Access Time

### 1.1 Mass-Storage Overview

| Category                | Examples                  | Characteristics                               |
|-------------------------|---------------------------|-----------------------------------------------|
| **Secondary Storage**   | HDD, SSD                  | Primary nonvolatile working storage           |
| **Tertiary Storage**    | Magnetic tape, optical disk | Slow but high capacity, used for backup     |
| **NVS (Nonvolatile Storage)** | All nonvolatile devices  | Data persists across power-off              |

This week's goals:
- Understand the physical structure of storage devices.
- Analyze the I/O scheduling algorithms the OS uses to order requests.
- Survey the storage device management services the OS provides, including **RAID**.

### 1.2 HDD Physical Structure

```
    Platter 1  ──── Head 1 (top surface)
                ──── Head 2 (bottom surface)
    Platter 2  ──── Head 3
                ──── Head 4
    Platter 3  ──── Head 5
                ──── Head 6
         ↑
       Spindle        ←── Disk arm (moves all heads together)
```

| Component | Description                                                                 |
|-----------|-----------------------------------------------------------------------------|
| **Platter** | Rigid disk where data is stored (both sides usable)                       |
| **Track**   | Concentric ring on the platter surface                                     |
| **Sector**  | Smallest logical unit of a track (historically 512 B, transitioning to 4 KB) |
| **Cylinder**| Set of tracks at the same radius across all platters                       |

- The **read-write head** flies over each platter surface on a very thin air (or helium) cushion.
- All heads attach to a **single disk arm** and move **simultaneously**, so seek time is paid once per arm move, not once per platter.
- Platter surfaces are coated with magnetic material; data is recorded and read magnetically.

> **Head crash.** A catastrophic event where the head physically touches the platter surface — destroying the magnetic coating and rendering the data unrecoverable. The entire disk must be replaced. RAID (§9) is the primary defense at the system level.

### 1.3 Rotation Speed and Capacity

The platter spins at a fixed **RPM** (revolutions per minute):

| RPM    | Typical use case            | Time per revolution |
|--------|-----------------------------|---------------------|
| 5,400  | Laptops, low power          | 11.1 ms             |
| 7,200  | Desktops, general servers   | 8.3 ms              |
| 10,000 | High-performance servers    | 6.0 ms              |
| 15,000 | Enterprise SAS              | 4.0 ms              |

- **Transfer rate** = data rate between drive and host. Actual transfer is always less than the nominal headline figure.
- Drive controllers include a **built-in DRAM buffer** as a cache for prefetched / write-back data.
- Common platter diameters: 1.8 to 3.5 inches.

### 1.4 Access Time = Seek + Rotational + Transfer

$$
\text{Access time} = \text{Seek time} + \text{Rotational latency} + \text{Transfer time}
$$

| Component             | Description                                                          | Magnitude         |
|-----------------------|----------------------------------------------------------------------|-------------------|
| **Seek time**         | Time to move the arm to the desired cylinder                         | Largest (~ ms)    |
| **Rotational latency**| Time to wait for the desired sector to rotate under the head         | Medium (~ ms)     |
| **Transfer time**     | Time to actually read or write the data                              | Smallest (~ μs)   |

```
  Seek time           Rotational latency       Transfer time
  ──────────→         ──────────────────→      ─────→
  arm movement         sector rotation wait     data transfer
```

**Seek time dominates** for random I/O. The goal of HDD scheduling (§4) is to **minimize total seek distance** by reordering requests.

### 1.5 Concrete Access-Time Example

For a 7,200 RPM drive:

- 1 revolution = $60 / 7{,}200 = 8.33$ ms.
- Average rotational latency = $8.33 / 2 = \mathbf{4.17\ ms}$ (we wait on average half a revolution).

Typical breakdown:

```
  Average seek time:          8 ms
  Average rotational latency: 4.17 ms
  Transfer time (4 KB):       ~0.01 ms
  ──────────────────────────────────
  Total access time:          ~12.18 ms
```

- Seek time is roughly **66%** of the total.
- That allows roughly **$1000 / 12.18 \approx 82$ random I/Os per second** — the "82 IOPS" that defines an HDD.
- **Sequential I/O is much faster** because the seek and rotation costs are amortized across a long contiguous run — HDD has a huge gap between random and sequential performance.

> **The single most important number on this slide is 82 IOPS.** It explains why databases use indices (to turn random access into sequential), why filesystems batch writes (to amortize seek), and why SSDs feel transformative — they kill the seek penalty entirely.

---

<br>

## 2. NVM Devices — SSD and NAND Flash

### 2.1 HDD vs SSD

| Property                       | HDD                          | SSD                            |
|--------------------------------|------------------------------|--------------------------------|
| Mechanical parts               | Yes (arm, spindle)           | None                           |
| Seek time                      | Yes                          | None                           |
| Rotational latency             | Yes                          | None                           |
| Random vs sequential gap       | Large                        | Small                          |
| Speed                          | Slow                         | Fast                           |
| Durability                     | Mechanical wear              | Limited write cycles           |
| Power consumption              | High                         | Low                            |
| Cost per GB                    | Cheap                        | Expensive                      |

Form factors: 2.5"/3.5" SATA SSDs, M.2 NVMe drives soldered to the motherboard, USB sticks, smartphone built-in storage, eMMC, etc. — physically diverse but all built on **NAND Flash** memory.

### 2.2 NAND Flash Characteristics

Three operations with very different units and speeds:

| Operation | Unit                                      | Speed                |
|-----------|-------------------------------------------|----------------------|
| Read      | **Page** (typically 4 KB – 16 KB)         | Fastest              |
| Write     | **Page**                                  | Slower than read     |
| Erase     | **Block** (group of many pages)           | Slowest              |

**Critical asymmetry — no overwrite.**

- A page that already holds data cannot be directly overwritten.
- To rewrite a page, the entire enclosing block must first be **erased**, then written.

This single property cascades into all the controller cleverness below.

- **Lifespan:** approximately $10^5$ program-erase cycles per cell (varies widely by product class).
- **DWPD** (Drive Writes Per Day): how many times the *entire* drive capacity can be overwritten per day for the rated lifespan — a standard endurance metric.

### 2.3 FTL, Garbage Collection, Over-Provisioning

**Flash Translation Layer (FTL).** The on-controller software that bridges what the OS sees (LBA, simple block reads/writes) and what NAND flash demands (page-level R/W, block-level erase, no overwrite).

- Maintains a **logical block → physical page** mapping.
- Tracks valid / invalid pages.
- The OS just reads/writes LBAs; everything below is the controller's problem.

**Garbage Collection.** When the drive fills up with invalid (rewritten) pages, the GC routine:

```
  Block A: [Valid][Invalid][Invalid][Valid]   → copy valid pages out
  Block B: [Valid][Valid][Valid][Free]          ← into here
  Block A: [Erased]                             → available for reuse
```

GC consumes bandwidth and contributes to write amplification.

**Over-Provisioning.** Reserve ~20% of the drive's physical capacity as spare space invisible to the user. Used as scratch space during GC and as a buffer for wear leveling. Improves both **performance** (GC has somewhere to put valid data) and **lifespan** (write cycles spread over a larger physical pool).

### 2.4 Wear Leveling, TRIM, Write Amplification

**Wear leveling.** Without it, hot blocks (frequently rewritten) would wear out long before cold blocks. The controller **distributes erases evenly** across all blocks:

```
  Without wear leveling:           With wear leveling:
  Block 0: ████████ (heavy)        Block 0: ████ (balanced)
  Block 1: █        (idle)         Block 1: ████ (balanced)
  Block 2: ██████   (heavy)        Block 2: ████ (balanced)
  Block 3: █        (idle)         Block 3: ████ (balanced)
```

**TRIM.** When a file is deleted at the filesystem level, the OS sends a TRIM command telling the SSD controller "block X is no longer in use." The controller can then mark those pages invalid eagerly, run GC ahead of time, and avoid copying soon-to-be-discarded data. Without TRIM, the SSD has no way to distinguish valid user data from filesystem-deleted-but-still-on-flash data.

**Write amplification.** A single logical write from the OS can internally trigger multiple reads, writes, and erases (because of GC moving live data out of the way). Write amplification factor (WAF) measures this overhead — a WAF of 3 means a 4 KB host write becomes 12 KB of actual NAND traffic. Lower is better for both performance and lifespan.

> **The big-picture deal:** the OS sees a tidy block device. Under the hood, the controller is constantly garbage-collecting, remapping, and shuffling pages. The reason SSDs feel "fast and durable" instead of "slow and fragile" is that all this work is hidden by the FTL.

### 2.5 NVM Scheduling

SSDs have **no seek**, so the HDD scheduling algorithms in §4 are largely irrelevant — there's nothing to reorder for arm movement.

| Property            | HDD                       | SSD                                       |
|---------------------|---------------------------|-------------------------------------------|
| Random read         | Slow (seek required)      | Fast (~ 100K IOPS)                        |
| Sequential read     | Relatively fast           | Fast                                      |
| Random write        | Slow                      | Fast but **variable** (GC dependence)     |
| Sequential write    | Relatively fast           | Fast                                      |

- SSDs typically use a simple **FCFS** policy (Linux NOOP scheduler).
- Adjacent requests are **merged** to make larger, more efficient transfers.
- Read service times are uniform, but **write service times are non-uniform** — they depend on garbage collection, current utilization, and wear-leveling state.
- HDD: hundreds of IOPS. SSD: **hundreds of thousands of IOPS**.

### 2.6 Volatile Memory as Storage (RAM Drives)

A **RAM drive** uses a portion of DRAM as a block device. A device driver allocates a region of DRAM and presents it to the filesystem layer as if it were a disk.

- The fastest possible storage — orders of magnitude beyond SSDs.
- **Volatile**: data is lost on system restart.
- Use cases: high-speed temporary file storage, cross-program data sharing, build-system caches.

| OS      | RAM drive mechanism                    |
|---------|----------------------------------------|
| Linux   | `/dev/ram`, `tmpfs` (e.g., `/tmp`)      |
| macOS   | `diskutil` command                     |
| Windows | Third-party tools                      |

Linux's **initrd** uses a RAM drive as a temporary root filesystem during early boot, before the real root device is available.

---

<br>

## 3. Connection and Address Mapping

### 3.1 Connection Interfaces

| Interface                | Characteristics                                                            |
|--------------------------|----------------------------------------------------------------------------|
| **SATA**                 | General purpose, low cost, up to ~600 MB/s, most common                    |
| **NVMe (PCIe)**          | High speed, low latency, optimized for SSDs, direct PCIe connection        |
| **SAS**                  | Server-grade, high reliability, cheaper than Fibre Channel                 |
| **USB**                  | External storage, universal connectivity                                   |
| **Fibre Channel (FC)**   | SAN environments, high-speed dedicated networked storage                   |

- **Host Bus Adapter (HBA)** = bus controller on the *computer* side.
- **Device Controller** = built-in controller in each *storage device*.

```
  CPU → HBA → bus (SATA / NVMe / SAS) → Device Controller → Media
                                                ↑
                                      DRAM cache (controller-local)
```

> **Why NVMe is dramatically faster than SATA:** NVMe rides directly on the **PCIe** bus instead of going through the SATA protocol stack. That means lower latency, much higher queue depth (64 K commands per queue, 64 K queues), and parallelism that matches what modern SSDs can actually deliver.

### 3.2 Logical Block Address (LBA)

The OS does not care about cylinders or pages. It addresses the device as a **1-dimensional array of logical blocks** numbered 0, 1, 2, ...

```
  Application       OS              Storage device
  ──────────→  ──────────→       ──────────────────
  File name     LBA number         Physical location
               (0, 1, 2, …)       HDD: Cylinder-Head-Sector (CHS)
                                  NVM: Chip-Block-Page
```

**LBA → CHS mapping on HDD.** Sector 0 is the first sector of the first track of the outermost cylinder. Subsequent LBAs spiral inward across tracks → cylinders. The real mapping is messier — bad-sector replacement, variable sector counts per track, etc.

**CAV vs CLV** (two ways to record data on rotating media):

| Method | Description                                                            | Used in |
|--------|------------------------------------------------------------------------|---------|
| **CAV (Constant Angular Velocity)** | Constant RPM; outer tracks hold more sectors    | HDD     |
| **CLV (Constant Linear Velocity)**  | Constant data density; rotates slower at outer edges | CD/DVD  |

---

<br>

## 4. HDD Scheduling Algorithms

### 4.1 Goals and Request Structure

Goals of disk scheduling:
- **Minimize access time** (especially seek time).
- **Maximize data transfer bandwidth**.
- Reorder the request queue to reduce arm movement distance.

Information attached to each I/O request:
1. Direction (input / output).
2. Open file handle.
3. Target memory address (where to deliver / fetch the bytes).
4. Data size.

When the disk is busy, new requests accumulate in a **pending queue**. The OS is free to reorder the queue for performance, as long as it doesn't violate ordering constraints (e.g., a write before a dependent read). Modern drives use LBA, so closer LBAs are assumed to be closer physically — close enough for scheduling.

> **Worked-example setup used throughout:**
> Request queue: `98, 183, 37, 122, 14, 124, 65, 67`
> Head start: 53
> Disk cylinders: 0 – 199

### 4.2 FCFS

**First-Come, First-Served.** Process requests in their arrival order.

```
  head
   53 → 98 → 183 → 37 → 122 → 14 → 124 → 65 → 67
```

Total head movement:
$|53-98| + |98-183| + |183-37| + |37-122| + |122-14| + |14-124| + |124-65| + |65-67|$
$= 45 + 85 + 146 + 85 + 108 + 110 + 59 + 2 = \mathbf{640\ cylinders}$.

| Advantages           | Disadvantages                            |
|----------------------|------------------------------------------|
| Simple to implement  | Head wildly swings back and forth        |
| Fair (no reordering) | Total seek distance is large             |
| No starvation        | Poor performance                         |

Notice the **122 → 14 → 124** segment: the arm crosses almost the whole disk twice. Reordering to "37, 14 then 122, 124" would be vastly more efficient — exactly what the next algorithms do.

### 4.3 SCAN — The Elevator Algorithm

Move in one direction to the end of the disk, servicing requests in order, then **reverse direction** and service the remaining ones — exactly like a building elevator.

Same queue, head 53, initial direction *decreasing*:

```
  head
   53 → 37 → 14 → [0] → 65 → 67 → 98 → 122 → 124 → 183
                    ↑
                reverse direction after reaching end
```

Total movement:
$(53-37) + (37-14) + (14-0) + (0-65) + (65-67) + \dots$
$= 16 + 23 + 14 + 65 + 2 + 31 + 24 + 2 + 59 = \mathbf{236\ cylinders}$

Much better than FCFS's 640.

> **Why "elevator"?** A real elevator with one going up, one going down doesn't bounce randomly between floors — it scans up to the top, then down to the bottom. Same idea.

### 4.4 C-SCAN — Circular SCAN

SCAN's downside: positions near the middle get serviced **twice** per round (once each direction), while positions at the ends get serviced only once. Waiting time is uneven.

**C-SCAN** services requests in **one direction only**. On reaching the end, the head **immediately jumps back to the start** without servicing anything during the return.

Same queue, direction *increasing*:

```
  53 → 65 → 67 → 98 → 122 → 124 → 183 → [199]
                                            ↓ (jump back, no service)
  [0] → 14 → 37
```

| Property             | SCAN                | C-SCAN                                        |
|----------------------|---------------------|-----------------------------------------------|
| Service direction    | Bidirectional       | Unidirectional                                |
| Waiting time         | Middle is favored   | **Uniform across positions**                  |
| Implementation       | Simple              | Slightly more complex                         |

C-SCAN treats cylinders as a **circular list**, wrapping from the last cylinder back to the first.

### 4.5 LOOK and C-LOOK

The "go all the way to the end" of SCAN / C-SCAN wastes movement when there are no requests near the end. **LOOK** and **C-LOOK** reverse / wrap as soon as there are no further requests in the current direction:

```
  SCAN:    53 → 37 → 14 → [0] → 65 → …    (goes all the way to 0)
  LOOK:    53 → 37 → 14 →        65 → …   (reverses at 14, the last request)

  C-SCAN:  53 → 65 → … → 183 → [199] → [0] → 14 → 37
  C-LOOK:  53 → 65 → … → 183 →               14 → 37
```

| Algorithm | Goes to disk end? | Bidirectional service? |
|-----------|-------------------|------------------------|
| SCAN      | Yes               | Yes                    |
| LOOK      | No                | Yes                    |
| C-SCAN    | Yes               | No (one-way)           |
| C-LOOK    | No                | No (one-way)           |

In practice, **LOOK / C-LOOK** are used much more often than SCAN / C-SCAN.

### 4.6 Comparison on the Same Queue

Same setup as above: queue `98, 183, 37, 122, 14, 124, 65, 67`, head 53, disk 0–199.

| Algorithm     | Total Movement | Characteristics                              |
|---------------|----------------|----------------------------------------------|
| FCFS          | **640**        | Fair but inefficient                         |
| SCAN (down)   | **236**        | Bidirectional service, efficient             |
| C-SCAN (up)   | **382**        | Uniform waiting time across cylinders        |
| LOOK (down)   | **208**        | Improved SCAN, no end-of-disk excursion      |
| C-LOOK (up)   | **330**        | Improved C-SCAN, no end-of-disk excursion    |

**Algorithm choice heuristics:**

- **Heavily-loaded systems:** SCAN, C-SCAN (built-in starvation prevention).
- **Uniform response time needed:** C-SCAN, C-LOOK.
- **Light load / simple systems:** FCFS (when the queue is short, all algorithms perform similarly).

### 4.7 Linux I/O Schedulers

**Deadline scheduler.**
- Separate read and write queues; **reads prioritized** (writes can be batched, reads block the user).
- Within each queue, sort by LBA (C-SCAN-like) for efficient batches.
- Each request has a **deadline** (default 500 ms) — guarantees no request waits forever.
- An FCFS queue runs in parallel to backstop old requests.

**NOOP scheduler.**
- FCFS plus adjacent-request merging — no reordering.
- Suitable for CPU-bound systems and especially for SSDs, where there is no seek to optimize.

**CFQ (Completely Fair Queueing).**
- Default for SATA HDDs in some distributions.
- Three priority queues: Real-time > Best-effort > Idle.
- Maintains per-process queues to exploit per-process locality of reference.

---

<br>

## 5. Error Detection and Correction

### 5.1 Parity, CRC, ECC

Stored or transmitted data can pick up bit errors. The defenses:

| Technique       | Function                          | Description                                                            |
|-----------------|-----------------------------------|------------------------------------------------------------------------|
| **Parity bit**  | Detection                         | Detects single-bit errors, cannot correct                              |
| **CRC**         | Integrity verification            | Cyclic Redundancy Check, polynomial-based, detects multi-bit errors    |
| **ECC**         | Detection + Correction            | Error-Correcting Code; can locate and correct small numbers of bit errors |

```
  Parity (even parity):
  Data: 10110101 → number of 1s = 5 (odd)
  Parity bit = 1  → 10110101 [1]   (total 1s becomes even)

  ECC:
  Data + multiple check bits → locates and corrects bad bit positions
  Soft error: correctable by ECC → recoverable
  Hard error: too many bad bits to correct → data loss

  CRC:
  Treat data as polynomial; store remainder mod a fixed generator polynomial
  Most widely used in network protocols
```

Modern storage devices have **built-in ECC** for automatic error detection and correction at the sector / page level.

### 5.2 ECC in Practice

**HDD:**
- On write, ECC is computed and stored alongside each sector.
- On read, ECC is recomputed and compared.
- Mismatch → corruption detected. If only a few bits are off, ECC corrects automatically.

**SSD (NVM):**
- ECC is stored per page.
- If a page produces *frequent* correctable errors, the controller marks it **bad** and stops writing to it.

```
  Write:  Data → [ECC calc] → [Data + ECC] → Storage
  Read:   Storage → [Data + ECC] → [ECC recalc]
                                       │
                       Match? ──→ Yes: return normally
                                └→ No:  correctable? ──→ Yes: Soft Error (correct, return)
                                                     └→ No:  Hard Error (I/O error reported)
```

**Consumer vs Enterprise.** Enterprise products use more powerful ECC schemes (Reed–Solomon, LDPC) trading more redundancy bits for higher correction capability — necessary for the higher reliability data centers demand.

---

<br>

## 6. Storage Device Management

### 6.1 Drive Formatting

Three-step process to make a fresh disk usable:

```
  Low-level Formatting  →  Partitioning  →  Logical Formatting
  (create physical       (logically divide   (create file system)
   sector structure)      the disk)
```

| Step                    | Description                                                              |
|-------------------------|--------------------------------------------------------------------------|
| **Low-level format**    | Creates header + data area + trailer (ECC) per sector. Done at factory   |
| **Partitioning**        | Divides the disk into one or more partitions (Linux `fdisk`, etc.)       |
| **Logical formatting**  | Creates a file system on a partition (FAT, ext4, NTFS, APFS, …)          |

**Low-level format layout:**

```
  ┌──────────┬────────────┬──────────┐
  │  Header  │  Data area │  Trailer │
  │ (sector#)│ (512B / 4KB)│  (ECC)  │
  └──────────┴────────────┴──────────┘
```

Most drives ship with low-level formatting already done at the factory. Users only do partitioning + logical formatting.

### 6.2 Partitions and Volumes

**Partition.** A logically divided region of a disk that can be used as an independent device. Linux: `/dev/sda1`, `/dev/sda2`, ...

**Volume.** Either a single partition or a logical storage unit combining multiple partitions / devices.
- **Linux LVM** can combine multiple disks into a single volume.
- **ZFS** integrates volume management and the file system into one layer.

```
  Physical disk
  ┌──────────┬──────────┬──────────┐
  │  Part 1  │  Part 2  │  Part 3  │
  │  (ext4)  │  (swap)  │  (ext4)  │
  └──────────┴──────────┴──────────┘

  Linux LVM:
  ┌── PV1 ──┐ ┌── PV2 ──┐
  │ Disk 1   │ │ Disk 2   │  →  Volume Group (VG)  →  Logical Volume (LV)
  └──────────┘ └──────────┘
```

**Raw I/O.** Direct access to a block device with **no file system on top**. Databases (especially Oracle) sometimes prefer this — they manage their own layout for performance reasons.

### 6.3 Boot Block

Boot sequence:

1. Power on → **firmware** (bootstrap loader stored in NVM flash on the motherboard) executes.
2. Firmware reads the **boot block** (MBR or GPT) from the boot storage device.
3. The boot block's code loads and executes the **OS kernel**.

- **Boot disk / system disk:** the disk containing the boot partition.
- Default Linux bootstrap loader: **GRUB2**.
- Firmware itself can be compromised by malware → real security concern in the supply chain.

### 6.4 Bad-Block Management

**Causes of bad blocks:**
- Manufacturing defects (shipped bad).
- Physical damage during use (head crash, etc.).
- Aging-related degradation of magnetic material.

**Method 1 — Sector sparing (forwarding).**
- Controller replaces bad sectors with **spare sectors** reserved during low-level formatting.
- Spares within the same cylinder are preferred (minimal seek penalty for the remap).

```
  LBA 87 request → controller checks ECC → BAD sector!
  → Automatically redirected to spare sector
  → All future LBA 87 requests map to the spare
```

**Method 2 — Sector slipping.**
- When a sector goes bad, shift all subsequent sectors by one position.
- Maintains physical layout continuity — but takes a long time to perform.

**NVM devices.** Bad pages are recorded in a table and excluded from future allocation. No seek concerns; remapping is essentially free.

---

<br>

## 7. Swap-Space Management

### 7.1 Purpose and Sizing

**Purpose.** Disk space that extends virtual memory: when physical memory is full, pages are sent to disk (**swap out**) and brought back when needed (**swap in**). Modern OSes swap at **page granularity** (Week 12), not whole processes.

**Sizing.**
- *Under-allocation:* the system runs out of swap and aborts processes (or crashes).
- *Over-allocation:* wastes disk space but is otherwise harmless.
- Old Linux rule of thumb: 2× physical memory.
- Modern systems: much less, because RAM is plentiful.

```
  Physical memory          Swap space (disk)
  ┌────────────────┐       ┌────────────────┐
  │  Active pages  │       │  Swapped-out   │
  │                │ ←──→  │  pages         │
  │                │       │                │
  └────────────────┘       └────────────────┘
```

### 7.2 Location — Partition vs File

| Method              | Advantages                            | Disadvantages                         |
|---------------------|---------------------------------------|---------------------------------------|
| Separate raw partition | Fast (no file-system overhead)     | Fixed size, low flexibility           |
| File-based (swap file) | Easy to resize                     | File-system overhead                  |

**Linux** supports both, and can distribute multiple swap spaces across different disks (better I/O bandwidth):

```bash
# Create and activate a swap partition
mkswap /dev/sda2
swapon /dev/sda2

# Create and activate a swap file
dd if=/dev/zero of=/swapfile bs=1M count=4096
mkswap /swapfile
swapon /swapfile
```

**Windows** uses a single managed file `pagefile.sys`; size is automatic.

### 7.3 Linux Swap Internal Structure

- Only **anonymous memory** is swap-eligible (code pages can be re-read from their executable file).
- Swap area is organized as an array of 4 KB **page slots**.
- **Swap map**: an array of integer counters, one per page slot.

| Counter value | Meaning                                       |
|---------------|-----------------------------------------------|
| 0             | Slot is empty (available)                     |
| 1             | One process is using that swapped page        |
| N (> 1)       | N processes share that page (shared memory)   |

When the counter drops to 0, the slot is reclaimable. This is essentially a tiny reference count for swap pages, mirroring the page-table-level COW logic from Week 12.

---

<br>

## 8. Storage Attachment

### 8.1 Overview

| Method          | Description                                     | Protocol              |
|-----------------|-------------------------------------------------|-----------------------|
| **Host-Attached** | Direct connection via SATA / SAS / NVMe         | AHCI, NVMe            |
| **NAS**         | File-system access over a network               | NFS, CIFS/SMB         |
| **Cloud Storage** | Internet-based storage service                  | REST API (S3, etc.)   |
| **SAN**         | Block-level access over a **dedicated** network | Fibre Channel, iSCSI  |

```
  Host-Attached: Server ──[SATA / SAS / NVMe]──→ Disk
  NAS:           Server ──[LAN (TCP/IP)]──→ NAS device (file-level)
  SAN:           Server ──[Dedicated network]──→ Storage array (block-level)
  Cloud:         Server ──[Internet]──→ Cloud provider (API)
```

### 8.2 NAS

**Network-Attached Storage.** File-level access over a general LAN.

- Protocols: **NFS** (UNIX / Linux), **CIFS / SMB** (Windows).
- RPC-based communication.
- File locking is supported, so multiple clients can safely share files.

**iSCSI.** Delivers SCSI protocol over IP networks for **block-level** access (unlike NFS/CIFS which is file-level). The host treats remote storage as if directly attached.

NAS pros: easy to set up, low cost.
NAS cons: storage I/O competes with normal LAN traffic for bandwidth; can become a bottleneck under load.

### 8.3 SAN

**Storage-Area Network.** Connects servers and storage over a **dedicated** high-performance network.

| Item              | NAS                  | SAN                   |
|-------------------|----------------------|-----------------------|
| Access level      | File (file-level)    | Block (block-level)   |
| Network           | General LAN          | **Dedicated**         |
| Protocol          | NFS, CIFS            | FC, iSCSI             |
| Cost              | Low                  | Expensive             |
| Performance       | Average              | **High**              |
| Flexibility       | Low                  | High (dynamic alloc.) |

SAN allows storage to be dynamically allocated and deallocated to specific servers. **JBOD** (Just a Bunch of Disks) is the simplest disk-array configuration on a SAN — no RAID, just raw enclosures.

### 8.4 Cloud Storage

Remote storage over the **Internet**.

| Property         | NAS                       | Cloud Storage                  |
|------------------|---------------------------|--------------------------------|
| Network          | LAN                       | **Internet / WAN**             |
| Access method    | File-system protocol      | **API-based** (REST / SDK)     |
| Failure handling | Hangs on disconnect       | App pauses and resumes         |
| Cost model       | Equipment purchase        | **Usage-based billing**        |

Representative services: Amazon S3, Microsoft OneDrive / Azure Blob, Google Cloud Storage, Apple iCloud, Dropbox.

**Why API-based?** Wide-area networks have high, variable latency and frequent disconnections. The file-system protocols (NFS/CIFS) are optimized for LAN conditions and behave poorly across the public Internet — REST APIs are explicitly designed for retries, eventual consistency, and disconnection-tolerant client code.

---

<br>

## 9. RAID

### 9.1 Purpose and Building Blocks

**RAID** = **R**edundant **A**rray of **I**ndependent **D**isks. Combine multiple physical disks to improve:
- **Reliability** — survive disk failures via redundancy.
- **Performance** — parallel I/O across disks.

Three techniques are mixed and matched across RAID levels:

| Technique  | Description                                             | What it buys                  |
|------------|---------------------------------------------------------|-------------------------------|
| **Striping** | Distribute data across multiple disks                  | Performance                   |
| **Mirroring** | Store duplicate copies of data                         | Reliability                   |
| **Parity** | Compute and store recovery info (XOR, etc.)             | Space-efficient reliability   |

```
  Striping:   [A1][A2][A3][A4]  →  Disk0[A1]  Disk1[A2]  Disk2[A3]  Disk3[A4]
  Mirroring:  [A1]              →  Disk0[A1]  Disk1[A1]   (identical copy)
  Parity:     [A1][A2][P]       →  Disk0[A1]  Disk1[A2]  Disk2[A1 XOR A2]
```

### 9.2 Reliability via Redundancy — MTBF Math

**MTBF (Mean Time Between Failures)** of a single disk ≈ 100,000 hours.

For an **array of 100 disks**, the probability of *some* disk failing in a given hour is 100× higher → array MTBF = $100{,}000 / 100 = 1{,}000$ hours, about **42 days**.

Adding more disks *raises* the failure rate of the array — which is why redundancy is mandatory in large arrays.

**Mirrored volume MTBF (independence assumed):**
- Per-disk MTBF: 100,000 hours.
- Mean time to repair (MTTR): 10 hours.
- Data is lost only if the second disk dies during the 10-hour repair window of the first:

$$
\text{MTBF}_{\text{data loss}} = \frac{(10^5)^2}{2 \cdot 10} = 5 \cdot 10^8 \text{ hours} \approx \mathbf{57{,}000\ years}.
$$

**Caveats** — the independence assumption breaks in practice:
- Power surges, natural disasters, manufacturing-batch defects → simultaneous failures.
- Aging disks have correlated failure timing (one failure foreshadows others).
- During a long rebuild, the surviving disks are stressed and more likely to fail.

These are real reasons RAID 6 (§9.8) was invented — to tolerate the second failure during the rebuild window.

### 9.3 Performance via Striping

**Bit-level striping.** Distribute the bits of each byte across multiple drives. With 8 drives, bit $i$ goes to drive $i$. Every access uses all drives in parallel → 8× transfer rate, but no parallelism for separate small accesses.

**Block-level striping (most common).** With $N$ drives, block $i$ goes to drive $i \bmod N$.

```
  4-Drive Block Striping:

  Block 0 → Disk 0    Block 1 → Disk 1
  Block 2 → Disk 2    Block 3 → Disk 3
  Block 4 → Disk 0    Block 5 → Disk 1
  ...
```

Two parallelism goals:
1. **Throughput improvement** — many small requests can be served simultaneously by different disks.
2. **Response-time reduction** — one large request is split across disks and runs in parallel.

### 9.4 RAID 0 — Striping Only

```
  Disk 0    Disk 1    Disk 2    Disk 3
  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐
  │  A1  │  │  A2  │  │  A3  │  │  A4  │
  │  A5  │  │  A6  │  │  A7  │  │  A8  │
  │  A9  │  │ A10  │  │ A11  │  │ A12  │
  └──────┘  └──────┘  └──────┘  └──────┘
```

| Property           | Detail                                              |
|--------------------|-----------------------------------------------------|
| Minimum disks      | 2                                                   |
| Usable capacity    | $N \times$ disk size (100%)                         |
| Read performance   | $N\times$                                           |
| Write performance  | $N\times$                                           |
| Fault tolerance    | **None** — any disk failure loses all data          |
| Use case           | Performance-critical, loss-tolerant (scratch space) |

Pure performance optimization, no reliability. If one disk fails, **everything** is lost.

### 9.5 RAID 1 — Mirroring

```
  Disk 0    Disk 1    Disk 2    Disk 3
  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐
  │  A1  │  │  A1  │  │  A3  │  │  A3  │
  │  A2  │  │  A2  │  │  A4  │  │  A4  │
  └──────┘  └──────┘  └──────┘  └──────┘
   mirror pair 1       mirror pair 2
```

| Property           | Detail                                                     |
|--------------------|------------------------------------------------------------|
| Minimum disks      | 2                                                          |
| Usable capacity    | $N/2 \times$ disk size (50%)                               |
| Read performance   | Improved (can read from either side)                       |
| Write performance  | Unchanged (must write to both)                             |
| Fault tolerance    | 1 disk per mirror pair                                     |
| Rebuild            | Fastest — simple copy                                      |

Simplest redundancy. Twice the cost, but very fast rebuild and high reliability.

### 9.6 RAID 4 — Striping + Dedicated Parity

```
  Disk 0    Disk 1    Disk 2    Parity
  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐
  │  A1  │  │  A2  │  │  A3  │  │  Ap  │   Ap = A1 ⊕ A2 ⊕ A3
  │  B1  │  │  B2  │  │  B3  │  │  Bp  │
  │  C1  │  │  C2  │  │  C3  │  │  Cp  │
  └──────┘  └──────┘  └──────┘  └──────┘
```

| Property         | Detail                            |
|------------------|-----------------------------------|
| Minimum disks    | 3                                 |
| Usable capacity  | $(N-1) \times$ disk size          |
| Fault tolerance  | 1 disk                            |
| Recovery         | Reverse-XOR using parity          |

**Recovery:** if Disk 1 dies, $A_2 = A_1 \oplus A_3 \oplus A_p$.

**Problem — parity bottleneck.** Every write also writes the parity disk → the parity disk gets hammered while the data disks idle. Small writes need read-modify-write (4 disk accesses). This is why RAID 4 is rare in practice.

### 9.7 RAID 5 — Distributed Parity

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

**Difference from RAID 4:** the parity block is **distributed across all disks** (one per stripe row). No single parity disk → no bottleneck.

| Property         | Detail                                  |
|------------------|-----------------------------------------|
| Minimum disks    | 3                                       |
| Usable capacity  | $(N-1) \times$ disk size                |
| Fault tolerance  | 1 disk                                  |
| Use              | **Most widely deployed parity RAID**     |

### 9.8 RAID 6 — Dual Parity

```
  Disk 0    Disk 1    Disk 2    Disk 3    Disk 4    Disk 5
  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐
  │  A1  │  │  A2  │  │  A3  │  │  A4  │  │  Ap  │  │  Aq  │
  │  B1  │  │  B2  │  │  B3  │  │  Bp  │  │  Bq  │  │  B4  │
  │  C1  │  │  C2  │  │  Cp  │  │  Cq  │  │  C3  │  │  C4  │
  └──────┘  └──────┘  └──────┘  └──────┘  └──────┘  └──────┘
```

| Property         | Detail                                            |
|------------------|---------------------------------------------------|
| Minimum disks    | 4                                                 |
| Usable capacity  | $(N-2) \times$ disk size                          |
| Fault tolerance  | **2 disks**                                       |
| Dual parity      | P (XOR) + Q (Galois-field arithmetic)             |

- P and Q must be computed by **different** mathematical operations — if they were identical, the redundancy wouldn't give extra recovery information.
- Write overhead is higher than RAID 5 (two parity updates per write).
- **Standard for large-capacity arrays**, where the long rebuild time after a single failure makes a second failure during rebuild a realistic threat.

### 9.9 RAID 0+1 vs RAID 1+0 (RAID 10)

Combinations of RAID 0 (striping) and RAID 1 (mirroring) at different layering orders.

- **RAID 0+1:** stripe first, then mirror the stripes. A single disk failure breaks the entire mirror's stripe — half the array goes offline.
- **RAID 1+0 (RAID 10):** mirror first, then stripe across the mirror pairs. A single disk failure only breaks one mirror — the rest of the array is intact, and only one disk needs rebuilding.

**RAID 10 is preferred** because the blast radius of a single failure is smaller and the rebuild target is a single disk → faster recovery.

### 9.10 Comprehensive Comparison

| Level   | Configuration                       | Reliability     | Performance         | Min Disks | Usable Capacity |
|---------|--------------------------------------|-----------------|---------------------|-----------|-----------------|
| RAID 0  | Striping only                        | None            | Highest             | 2         | 100%            |
| RAID 1  | Mirroring                            | 1 disk          | Read improved       | 2         | 50%             |
| RAID 4  | Striping + dedicated parity          | 1 disk          | Parity bottleneck   | 3         | $(N-1)/N$       |
| RAID 5  | Striping + distributed parity        | 1 disk          | Improved over 4     | 3         | $(N-1)/N$       |
| RAID 6  | RAID 5 + dual parity                 | **2 disks**     | Higher write overhead | 4       | $(N-2)/N$       |
| RAID 10 | Mirror + stripe (1+0)                | 1 per pair      | Excellent R/W       | 4         | 50%             |

**Choosing a RAID level:**
- **RAID 0:** maximum performance, data loss acceptable.
- **RAID 1:** fast rebuild + high reliability, cost is acceptable.
- **RAID 5:** space-efficient, mid-scale storage.
- **RAID 6:** large arrays, highest reliability.
- **RAID 10:** both performance *and* reliability (DB servers, virtualization hosts).

### 9.11 Hot Spare and Implementation Layers

**Hot spare.** A disk that holds **no data** but is **automatically deployed** as a replacement when a failure occurs. In RAID 5 with a hot spare, the rebuild starts immediately without human intervention.

**RAID implementation layers:**

| Layer                  | Description                                                          |
|------------------------|----------------------------------------------------------------------|
| **Software RAID**      | Implemented in OS kernel or volume manager (Linux `md` / `mdadm`)     |
| **HBA**                | Hardware RAID controller card                                         |
| **Storage Array**      | Dedicated controller inside an external storage device (most common)  |
| **SAN Interconnect**   | RAID provided by a virtualization device between hosts and storage    |

**Additional features:**
- **Snapshot:** captures the file-system state at a specific point in time (cheap thanks to copy-on-write).
- **Replication:** automatically copies data to a remote site for disaster recovery.
  - *Synchronous:* acknowledge after both sides have written → no data loss, but higher latency.
  - *Asynchronous:* batch periodically → some data loss possible if the source dies, but much faster.

### 9.12 What RAID Doesn't Protect Against — ZFS

RAID protects against **physical media errors**. It does *not* protect against:
- File-pointer / metadata errors caused by software bugs.
- **Torn writes** — incomplete writes during power loss.
- RAID controller bugs / failures.
- Silent bit rot beyond what ECC catches.

**ZFS** takes a different approach:
- Stores **checksums for every block** (data and metadata).
- Crucially, the checksum is stored in the **parent pointer**, not in the block itself — so corrupted blocks can't lie about their checksum.

On read:
- Data → recompute checksum → compare against parent's stored checksum.
- Mismatch → corruption detected.
- If a mirror or parity copy exists, automatically recover from the intact copy.

ZFS also integrates **volume management and the file system** into a unified storage pool — eliminating the volume-manager/filesystem layering mismatch that complicates traditional stacks.

---

<br>

## 10. Lab — Disk-Scheduling Simulator

Implement FCFS, SCAN, C-SCAN, LOOK, C-LOOK in Python and compare the head movement on the canonical request queue.

**Inputs:** request queue (list of cylinder numbers), current head position, disk size, initial direction (for SCAN/C-SCAN).

```python
def fcfs(requests, head):
    """FCFS: process requests in arrival order"""
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
    """SCAN: one direction to the end, then reverse"""
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
    """C-SCAN: service one direction, wrap to start"""
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

LOOK and C-LOOK are the same with the end-of-disk excursions removed.

**Driver and expected output:**

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

Expected totals: **FCFS 640, SCAN 236, C-SCAN 382, LOOK 208, C-LOOK 330**.

**Extension:** visualize the head movement path for each algorithm with matplotlib.

---

<br>

## Summary

| Topic                  | Key Content                                                          |
|------------------------|----------------------------------------------------------------------|
| **HDD structure**      | Platter, track, sector, cylinder, RPM                                |
| **Access time**        | Seek + rotational latency + transfer (seek dominates)                |
| **NVM / SSD**          | NAND Flash, FTL, garbage collection, wear leveling, TRIM             |
| **NAND specifics**     | Page-level R/W, block-level erase, no overwrite                      |
| **HDD scheduling**     | FCFS, SCAN, C-SCAN, LOOK, C-LOOK                                     |
| **Linux schedulers**   | Deadline, NOOP, CFQ                                                  |
| **Error handling**     | Parity, CRC, ECC; soft vs hard errors                                |
| **Formatting**         | Low-level → partitioning → logical formatting                        |
| **Boot block**         | MBR/GPT → bootstrap → OS kernel                                       |
| **Bad blocks**         | Sector sparing, sector slipping                                      |
| **Swap space**         | Virtual-memory extension; partition vs file; per-slot reference count|
| **Storage attachment** | Host-attached, NAS, SAN, cloud                                        |
| **RAID**               | Striping, mirroring, parity for performance and reliability           |
| **RAID levels**        | 0, 1, 4, 5, 6, 10 — choose by use case                                |
| **Beyond RAID**        | ZFS-style checksums catch what RAID doesn't                          |

**Key takeaways:**
- **Seek time is the dominant cost on HDDs**, which is why scheduling, indexing, and SSDs all matter.
- **SSDs are fundamentally different storage devices**, not just "fast HDDs" — the no-overwrite property reshapes the entire controller stack (FTL, GC, wear leveling).
- **HDD scheduling = elevator analogy** — SCAN and LOOK turn random arm flailing into smooth one-way sweeps; choose based on whether you care about uniform latency (C-variants) or minimum total movement.
- **RAID levels are about trading capacity for reliability / performance** — there is no "best" level, only the right level for the workload.
- **RAID's blind spots motivate ZFS-style end-to-end checksums** — physical-media redundancy doesn't catch corruption above the device layer.
- **Swap, RAID, and storage attachment all funnel through the same OS storage stack** — what's exposed to user-space is just "a block device," but everything underneath is interesting.

> "All storage is a lie about flat, infinite, reliable arrays of bytes. The OS's job is to keep the lie credible."

**Next week:** Security and Protection — CIA triad, threats, cryptography, access control, authentication.

---

<br>

## Self-Check Questions

1. **Access-time arithmetic:** For a 10,000 RPM drive with 5 ms average seek time and 4 KB sectors, compute the expected access time and the resulting random IOPS.

   > **Answer:** 1 revolution = $60{,}000 / 10{,}000 = 6$ ms; average rotational latency = $6 / 2 = 3$ ms. Transfer time at ~150 MB/s for 4 KB ≈ 0.027 ms (negligible). Total access time ≈ $5 + 3 + 0.027 \approx 8\ \text{ms}$. Random IOPS = $1000 / 8 \approx 125$. SSDs by comparison hit 100K+ IOPS — the ~1000× gap explains why "is it on SSD?" is the first question for any latency-sensitive workload.

2. **Why no overwrite?** Explain why NAND Flash cannot overwrite a page directly, and the cascade of consequences (FTL, garbage collection, write amplification).

   > **Answer:** A NAND cell's program operation can only flip bits in one direction (typically 1 → 0 within a page). To set bits back to 1 requires an **erase**, which physically resets a large block (many pages) — slow and stressful on the cells. So a "rewrite" of a page must (1) write the new contents to a different free page, (2) mark the old page invalid in the FTL mapping, (3) eventually, when many pages in a block are invalid, the **garbage collector** copies the still-valid pages elsewhere and erases the block. The mapping layer (**FTL**) hides this from the OS. The downside: one logical write triggers extra reads/writes/erases internally → **write amplification**, and SSD endurance is measured in write cycles, so amplification directly shortens lifespan.

3. **TRIM importance:** Without TRIM, why does an "old" SSD's write performance degrade?

   > **Answer:** Without TRIM, when the user deletes a file, the OS just updates filesystem metadata — the SSD has no way to know that those LBAs hold garbage. From the SSD's perspective, every block is "in use" with valid-looking data. So when the user later writes to a "free" LBA, the SSD must (1) read the existing block's data, (2) modify it, (3) erase the old block, (4) write the new version — heavy read-modify-erase-write cycles. With TRIM, the OS tells the SSD "these LBAs are now garbage" → the controller can mark those pages invalid immediately, run GC during idle time, and respond to subsequent writes without on-demand erase work. Result: TRIM-enabled SSDs maintain near-fresh write performance over their lifetime.

4. **HDD scheduling trace:** Given queue `82, 170, 43, 140, 24, 16, 190` and head 50, disk 0–199, compute the total movement under FCFS, SCAN (initial direction up), and C-LOOK (direction up).

   > **Answer:** **FCFS:** $|50-82|+|82-170|+|170-43|+|43-140|+|140-24|+|24-16|+|16-190| = 32+88+127+97+116+8+174 = \mathbf{642}$. **SCAN up:** sweep up through 82, 140, 170, 190 then to end 199, then down to 43, 24, 16: $(199-50)+(199-16) = 149 + 183 = \mathbf{332}$. **C-LOOK up:** sweep up to 190 (the largest), wrap to 16 (the smallest), then 24, 43: $(190-50)+(190-16)+(43-16) = 140+174+27 = \mathbf{341}$. (Alternatively compute via segments — values will match within rounding to the per-segment arithmetic.)

5. **SCAN vs C-SCAN fairness:** Why does C-SCAN give more uniform waiting times than SCAN, even though SCAN visits every cylinder twice per round?

   > **Answer:** Under SCAN, cylinders in the **middle** are visited twice per round (once on the way up, once on the way down), while cylinders at the **ends** are visited once per round — so end-cylinders wait roughly twice as long between services. Under C-SCAN, every cylinder is visited exactly once per round (after the wrap-around the head doesn't service anything until it reaches the start again), so all cylinders see the same inter-service gap. Even though SCAN's *average* wait is shorter, the **variance** in wait times is higher — and for QoS-sensitive workloads, uniform latency beats lower-average latency.

6. **Parity vs ECC distinction:** A friend says "ECC is just parity with more bits." Refine this.

   > **Answer:** Parity adds *one* bit per data word, allowing detection of any *odd* number of bit errors but no correction (you know something is wrong, not where). ECC adds *multiple* check bits arranged so that the location of the bad bit(s) can be reconstructed — enabling actual **correction**, not just detection. Hamming codes correct 1 bit per word; Reed-Solomon and LDPC correct many bits per block. The "more bits" framing is technically true, but the qualitative leap is from detection to correction, which fundamentally changes what the system can do (silently fix soft errors vs. report an I/O failure).

7. **Boot sequence:** Walk through power-on to OS-kernel-running step by step, naming MBR/GPT, GRUB, and the firmware.

   > **Answer:** (1) **Power on** → CPU starts executing **firmware** stored in motherboard NVM (BIOS or UEFI). (2) Firmware initializes hardware (POST), then reads the **boot block** from the boot disk — sector 0 in legacy BIOS (**MBR**) or an EFI System Partition file in UEFI (**GPT**-formatted disks). (3) The boot block contains the **first-stage bootloader** (very small, ~440 bytes for MBR). (4) The first-stage bootloader loads the **second-stage bootloader** (on Linux, **GRUB2**) from elsewhere on the disk. (5) GRUB presents a menu, then loads the **OS kernel** (e.g., `vmlinuz`) plus an **initrd** (RAM disk with minimal drivers) into memory. (6) GRUB transfers control to the kernel's entry point. (7) Kernel sets up paging, mounts the real root filesystem (using initrd's drivers if needed), starts `init` / `systemd` — the OS is now running.

8. **Swap sizing under modern RAM:** When does a modern system with 32 GB of RAM still benefit from swap, and when is it counterproductive?

   > **Answer:** **Beneficial:** (a) **Hibernation** — the kernel writes RAM contents to swap before powering off; without swap, hibernation is impossible. (b) **Memory pressure tail** — even on a well-provisioned system, occasional bursts (massive compile, large DB import) can briefly exceed RAM; swap lets these complete slowly instead of triggering OOM kill. (c) **Cold pages** — long-idle process pages can be moved to disk so RAM is available for active workloads. **Counterproductive:** (a) On systems where any swap activity destroys interactive responsiveness — better to have OOM kill the offender. (b) On systems with high-endurance-sensitive flash where unnecessary writes shorten lifespan (this is *the* reason mobile OSes avoid swap, Week 12). (c) As a substitute for adding RAM when the workload truly needs the RAM — swap will keep the system technically alive but unusably slow.

9. **MTBF math:** A 4-disk RAID 5 array with per-disk MTBF = $10^5$ hours and MTTR = 20 hours. What is the array's data-loss MTBF under independence?

   > **Answer:** RAID 5 tolerates 1 disk failure. Data loss requires a second failure during the 20-hour repair window of the first. Per-disk failure rate = $1 / 10^5$ per hour. Probability of any of the 4 disks failing in an hour ≈ $4 / 10^5$. Given the first failure, probability that any of the **remaining 3** disks fails within 20 hours ≈ $3 \cdot 20 / 10^5 = 6 \cdot 10^{-4}$. Combined hazard rate: $(4/10^5) \cdot (6 \cdot 10^{-4}) = 2.4 \cdot 10^{-8}$ per hour. MTBF ≈ $1 / (2.4 \cdot 10^{-8}) \approx 4.2 \cdot 10^7$ hours $\approx$ **4,750 years**. The catch: independence is a much weaker assumption with 3 surviving disks under stressed rebuild conditions than with 1; real-world MTBFs are dramatically worse, which is *exactly* the failure mode RAID 6 (tolerating 2 failures) addresses.

10. **RAID 5 vs RAID 10 for a busy DB:** A finance database does many small random writes. Which RAID level should you choose and why?

    > **Answer:** **RAID 10.** Each RAID 5 small write requires a **read-modify-write** cycle on both data and parity blocks — 4 disk accesses per logical write, with the parity computation in the critical path. For random-write-heavy workloads this halves or quarters the achievable IOPS compared to mirroring. **RAID 10** writes to each disk in the mirror pair in parallel — 2 writes per logical write, no read-modify-write, no parity bottleneck. The tradeoff: RAID 10 uses 50% of raw capacity (vs $(N-1)/N$ for RAID 5), so you pay more in disks for the throughput. For OLTP databases, the IOPS savings overwhelmingly justify the cost.

11. **ZFS checksum trick:** Why does ZFS store block checksums in the *parent pointer*, not in the block itself? What attack / failure does this defend against?

    > **Answer:** If the checksum lived inside the block, a hardware corruption that flipped both data and the checksum (which is plausible — both are on the same disk sector / page) would go undetected because the corrupted block would "validate" against its own corrupted checksum. By storing the checksum in the **parent pointer** — a different on-disk location, in a different block — corruption of the data block cannot also corrupt its checksum. On read, ZFS fetches the parent pointer, finds the checksum, then reads the data block and verifies. The defense extends beyond bit rot to **firmware bugs** that silently return wrong data, **misdirected writes** that put bytes in the wrong sector, and **lost writes** that never reach disk despite the controller acknowledging — failure modes RAID alone is blind to.

12. **Storage attachment choice:** A startup needs shared storage accessible by 8 web servers and 2 batch-processing servers. Compare NAS, SAN, and cloud storage for this use case.

    > **Answer:** **NAS** (e.g., NFS) is the natural fit: 10 servers share files over LAN; setup is simple; cost is low; standard `mount -t nfs`. Performance is fine for web payloads but could bottleneck on batch jobs reading large files concurrently. **SAN** would give block-level access with dedicated FC/iSCSI links — overkill and expensive for 10 servers; SAN shines at 100+ servers with high IOPS demands (DB clusters, virtualization farms). **Cloud storage** (e.g., S3) is appropriate if the application is already cloud-native or needs durability beyond a single datacenter; the API-based access model requires app code changes (no `mount`), and per-GB pricing scales with usage rather than a single up-front purchase. For a startup at this scale: NAS for hot workloads, cloud storage for backup and large rarely-accessed assets. SAN is overkill until traffic 10× grows.
