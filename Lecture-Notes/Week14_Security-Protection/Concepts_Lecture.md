# Week 14 Lecture — Security and Protection

[Repository Guide](../../README.md) | [English](Concepts_Lecture.md) | [한국어](Concepts_Lecture.ko.md)

> **Last Updated:** 2026-10-18
>
> Operating System Concepts, Silberschatz, Ch 16, 17

> **Learning Objectives:**
> 1. Distinguish **Security** from **Protection** and explain the **CIA triad** (confidentiality, integrity, availability) plus extra violation types (theft of service, DoS)
> 2. Distinguish **threat** from **attack** and list major attacks (masquerading, replay, MITM, session hijacking, privilege escalation)
> 3. Explain the **four-layer security model**, the weakest link, the human factor (social engineering), and the **attack surface**
> 4. Classify **malware** (Trojan, Spyware, Ransomware, Back door, Logic bomb) and explain how the **principle of least privilege** governs the blast radius
> 5. Explain the principles and defenses of **code injection**, including buffer overflows (stack layout and return-address overwriting), shellcode and NOP sleds, and SQL injection
> 6. Distinguish **viruses** (by type) from **worms** along host requirement, propagation, and damage
> 7. Distinguish **network threats** (sniffing, spoofing, MITM, DoS/DDoS, SYN flood, port scanning) and explain the zombie and botnet concepts
> 8. Explain **symmetric/asymmetric** crypto (AES, RSA), **hashing** (SHA, salt), **digital signatures/certificates**, the **TLS handshake**, and the **key-distribution problem**
> 9. Explain the **three authentication factors**, password vulnerabilities, **OTP**, and **MFA**
> 10. Explain the **goals of protection** and core principles (least privilege, compartmentalization, defense in depth), and distinguish **need-to-know** from **least-privilege**
> 11. Explain **protection rings** (Intel rings, ARM Exception Levels, TrustZone) and **domains and domain switching** (setuid)
> 12. Compare the **access matrix** and its implementations (global table, **ACL**, **capability**, lock-key) and explain **revocation**
> 13. Distinguish **access-control models** (RBAC, DAC vs MAC, SELinux, Linux capabilities)
> 14. Explain **sandboxing** (SECCOMP-BPF, Seatbelt) and **code signing** (SIP, Entitlements)

---

## Table of Contents

- [1. The Security Problem](#1-the-security-problem)
  - [1.1 Security vs Protection](#11-security-vs-protection)
  - [1.2 Three Principles of Security: the CIA Triad](#12-three-principles-of-security-the-cia-triad)
  - [1.3 Threat vs Attack](#13-threat-vs-attack)
  - [1.4 The Four-Layer Security Model](#14-the-four-layer-security-model)
- [2. Program Threats](#2-program-threats)
  - [2.1 Malware Overview](#21-malware-overview)
  - [2.2 Trojan Horse: Login Emulator](#22-trojan-horse-login-emulator)
  - [2.3 Ransomware and the Principle of Least Privilege](#23-ransomware-and-the-principle-of-least-privilege)
  - [2.4 Code Injection: Buffer Overflow](#24-code-injection-buffer-overflow)
  - [2.5 Shellcode and NOP-sled](#25-shellcode-and-nop-sled)
  - [2.6 SQL Injection](#26-sql-injection)
  - [2.7 Viruses](#27-viruses)
  - [2.8 Worms](#28-worms)
- [3. System and Network Threats](#3-system-and-network-threats)
  - [3.1 Attacking Network Traffic: Sniffing, Spoofing, MITM](#31-attacking-network-traffic-sniffing-spoofing-mitm)
  - [3.2 Denial of Service: DoS and DDoS](#32-denial-of-service-dos-and-ddos)
  - [3.3 Port Scanning](#33-port-scanning)
- [4. Cryptography](#4-cryptography)
  - [4.1 Overview](#41-overview)
  - [4.2 Symmetric Encryption](#42-symmetric-encryption)
  - [4.3 Asymmetric Encryption: RSA](#43-asymmetric-encryption-rsa)
  - [4.4 Symmetric vs Asymmetric and Hybrid](#44-symmetric-vs-asymmetric-and-hybrid)
  - [4.5 Hash Functions](#45-hash-functions)
  - [4.6 Hash Application: Password Storage and Salt](#46-hash-application-password-storage-and-salt)
  - [4.7 Digital Signatures](#47-digital-signatures)
  - [4.8 Digital Certificates](#48-digital-certificates)
  - [4.9 TLS](#49-tls)
  - [4.10 The Key-Distribution Problem](#410-the-key-distribution-problem)
- [5. User Authentication](#5-user-authentication)
  - [5.1 Authentication Methods: Three Factors](#51-authentication-methods-three-factors)
  - [5.2 Password Vulnerabilities](#52-password-vulnerabilities)
  - [5.3 One-Time Password (OTP)](#53-one-time-password-otp)
  - [5.4 Multifactor Authentication (MFA)](#54-multifactor-authentication-mfa)
- [6. Goals and Principles of Protection](#6-goals-and-principles-of-protection)
  - [6.1 Goals of Protection and Policy vs Mechanism](#61-goals-of-protection-and-policy-vs-mechanism)
  - [6.2 Key Protection Principles](#62-key-protection-principles)
  - [6.3 The Need-to-Know Principle](#63-the-need-to-know-principle)
- [7. Protection Rings](#7-protection-rings)
  - [7.1 Protection-Ring Structure](#71-protection-ring-structure)
  - [7.2 Intel and ARM Protection Rings](#72-intel-and-arm-protection-rings)
- [8. Domain of Protection](#8-domain-of-protection)
  - [8.1 Domain Structure](#81-domain-structure)
  - [8.2 Domain Switching: setuid](#82-domain-switching-setuid)
- [9. Access Matrix](#9-access-matrix)
  - [9.1 Access Matrix Overview](#91-access-matrix-overview)
  - [9.2 Domain Switching in the Matrix](#92-domain-switching-in-the-matrix)
  - [9.3 Modifying Access Rights: Copy, Owner, Control](#93-modifying-access-rights-copy-owner-control)
- [10. Implementation of the Access Matrix](#10-implementation-of-the-access-matrix)
  - [10.1 Global Table](#101-global-table)
  - [10.2 Access List (ACL): Column-Based](#102-access-list-acl-column-based)
  - [10.3 Capability List: Row-Based](#103-capability-list-row-based)
  - [10.4 Lock-Key Mechanism](#104-lock-key-mechanism)
  - [10.5 Implementation Comparison](#105-implementation-comparison)
  - [10.6 Revocation of Access Rights](#106-revocation-of-access-rights)
- [11. Access-Control Models](#11-access-control-models)
  - [11.1 RBAC: Role-Based Access Control](#111-rbac-role-based-access-control)
  - [11.2 DAC vs MAC](#112-dac-vs-mac)
  - [11.3 MAC: Security Labels and SELinux](#113-mac-security-labels-and-selinux)
  - [11.4 Linux Capabilities](#114-linux-capabilities)
- [12. Sandboxing and Code Signing](#12-sandboxing-and-code-signing)
  - [12.1 Sandboxing](#121-sandboxing)
  - [12.2 Sandbox Profile Example](#122-sandbox-profile-example)
  - [12.3 Code Signing](#123-code-signing)
  - [12.4 System Integrity Protection (SIP)](#124-system-integrity-protection-sip)
- [13. Lab: Implementing an Access-Control System](#13-lab-implementing-an-access-control-system)
- [Summary](#summary)
- [Self-Check Questions](#self-check-questions)

---

<br>

## 1. The Security Problem

### 1.1 Security vs Protection

| Aspect      | Security                                  | Protection                          |
|-------------|-------------------------------------------|-------------------------------------|
| **Definition** | A measure of confidence that the integrity of a system and its data will be preserved | A set of mechanisms that control processes' and users' access to resources |
| **Focus**   | Protecting the system from external threats | Controlling internal resource access |
| **Target**  | Attackers, malware, network threats       | Processes, users, roles             |
| **Scope**   | Physical, network, OS, application        | Domains, access matrix, privileges  |
| **Textbook**| Ch 16                                     | Ch 17                               |

*Table 1. Comparison of security and protection*

- **Security** is the broader, outward-facing notion of confidence that the system will behave trustworthily.
- **Protection** is the concrete *mechanism* inside the OS that realizes that confidence. Protection is a subset of, and the foundation for, security.

### 1.2 Three Principles of Security: the CIA Triad

| Principle                  | Meaning                                       | Violation Example               |
|----------------------------|-----------------------------------------------|---------------------------------|
| **Confidentiality**        | Only authorized users can *access* information | Eavesdropping, information leakage |
| **Integrity**              | Only authorized users can *modify* information | Data tampering, source-code manipulation |
| **Availability**           | Authorized users can *access* resources when needed | DoS attacks, system outages |

*Table 2. The three principles of the CIA triad and violation examples*

Additional types of security violation are as follows.

- **Theft of Service:** unauthorized use of resources (e.g., cryptocurrency mining, spam relay).
- **Denial of Service:** preventing legitimate users from using the system.

> **Key Point:** The CIA triad is the starting point of every security discussion. Whatever the attack, it ultimately breaks one or more of these three. Eavesdropping targets confidentiality, tampering targets integrity, and DoS targets availability.

### 1.3 Threat vs Attack

| Concept          | Definition                                          |
|------------------|-----------------------------------------------------|
| **Threat**       | A potential security violation (a possibility), such as *the discovery of a vulnerability* |
| **Attack**       | An actual attempt to breach security, that is, *a concrete action* |

*Table 3. Definitions of threat and attack*

The major attack types are as follows.

| Attack Type                  | Description                                    |
|------------------------------|------------------------------------------------|
| **Masquerading**             | Impersonating another user or system           |
| **Replay Attack**            | Maliciously repeating a valid data transmission |
| **Man-in-the-Middle**        | Intercepting or modifying data during communication |
| **Session Hijacking**        | Taking over an active communication session    |
| **Privilege Escalation**     | Gaining higher privileges than authorized      |

*Table 4. Major attack types*

### 1.4 The Four-Layer Security Model

![Figure 1. Attack types and prevention methods for each layer of the four-layer security model](../../images/Week14_Figure-16-1.png)

*Figure 1. Attack types and prevention methods for each layer of the four-layer security model*

As textbook Figure 16.1 shows, security must be addressed simultaneously across four layers: Physical → Network → Operating System → Application. In Figure 1, the physical layer counters console access and hardware-based attacks with guards, vaults, and device data encryption, and the network layer counters sniffing, spoofing, and masquerading with encryption, authentication, and filtering. The operating-system layer counters insecure defaults and platform vulnerabilities with patches, reconfiguration, and hardening, and the application layer counters logic bugs, design flaws, and code injections with sandboxing and software restrictions.

- **Weakest link:** security is only as strong as its *weakest layer* (the chain analogy). If one layer is breached, the whole is at risk.
- **Human factor:** attacks targeting *people*, such as social engineering and phishing, are often the most vulnerable link.
- **Attack surface:** the set of points where an attacker can attempt to infiltrate. Reducing the attack surface is a core defensive strategy.

---

<br>

## 2. Program Threats

### 2.1 Malware Overview

**Malware:** software designed to exploit, disable, or damage a system.

| Type                       | Description                                                |
|----------------------------|-----------------------------------------------------------|
| **Trojan Horse**           | Disguised as a legitimate program but executes malicious functions |
| **Spyware**                | Secretly collects user information and sends it externally (adware, keylogger) |
| **Ransomware**             | Encrypts files and demands payment for decryption         |
| **Back Door (Trap Door)**  | A secret access path left by a developer                  |
| **Logic Bomb**             | Malicious code that activates when specific conditions are met |

*Table 5. Types of malware*

- A Trojan-horse variant is the **login emulator** (fake login screen).
- Violating the **principle of least privilege** maximizes malware damage. If a program running with excessive privileges is compromised, the whole system falls.

### 2.2 Trojan Horse: Login Emulator

**Login-emulator attack scenario:**

```text
  1. The attacker leaves a fake login screen running
  2. The victim attempts to log in → "Password error" message is shown
  3. In reality, the ID/PW are sent to the attacker
  4. The fake program exits and the real login prompt appears
  5. The victim thinks "it was a typo" and logs in again (succeeds normally)
```

**Defenses:**
- Use a *non-trappable key sequence* like **Ctrl+Alt+Delete** (Windows). It is a secure attention key (SAK) that a fake program cannot intercept.
- Display session-usage information at session termination (last login time, etc.).
- Verify URL accuracy (phishing prevention).

### 2.3 Ransomware and the Principle of Least Privilege

**Ransomware operation:**

```text
  1. Malware infection (phishing emails, vulnerable services, etc.)
  2. Encrypts files on the system using a strong encryption algorithm
  3. Demands payment (cryptocurrency) in exchange for the decryption key
  4. No guarantee of receiving the key even after payment
```

**The Principle of Least Privilege:**
- Grant programs and users only the **minimum privileges** needed for their tasks.
- Excessive privileges let malware take over the entire system.
- The OS should provide **fine-grained access control**.

### 2.4 Code Injection: Buffer Overflow

Many program threats occur through **code injection**.

**Buffer Overflow:** an attack that overwrites adjacent memory by providing input that exceeds buffer boundaries.

```c
#include <stdio.h>
#include <string.h>
#define BUFFER_SIZE 64

void vulnerable(char *input) {
    char buffer[BUFFER_SIZE];
    strcpy(buffer, input);   // No length check → overflow possible
}

// Safe version
void safe(char *input) {
    char buffer[BUFFER_SIZE];
    strncpy(buffer, input, sizeof(buffer) - 1);  // Length limited
    buffer[sizeof(buffer) - 1] = '\0';
}
```

- `strcpy()`, `sprintf()`, `gets()` are **dangerous functions** because they do not check length.
- Use *size-aware* functions like `strncpy()`, `snprintf()` instead.

**Consequences of a buffer overflow:** the stack layout is shown below, with high addresses at the top and low addresses at the bottom.

```text
  ┌──────────────────┐
  │  Return Address   │  ← (3) overwrite this → control the code flow
  ├──────────────────┤
  │  Saved EBP        │  ← (2) frame pointer can be corrupted
  ├──────────────────┤
  │  Local Variables  │  ← (2) adjacent variables overwritten
  ├──────────────────┤
  │  buffer[64]       │  ← (1) within the padding range: no effect
  └──────────────────┘
         ↑ overflow direction
```

*Figure 2. Stack frame layout and overwrite range in a buffer overflow*

The consequences by overflow size are as follows.
1. **Small overflow:** absorbed by the padding area, with no impact.
2. **Medium overflow:** overwrites adjacent variables and may crash the program.
3. **Large overflow:** **corrupts the return address**, leading to arbitrary code (shellcode) execution.

### 2.5 Shellcode and NOP-sled

**Shellcode:** code injected by the attacker, which typically spawns a shell.

```c
void exploit(void) {
    execl("/bin/sh", "/bin/sh", NULL);   // Spawn a shell
}
```

The attack proceeds as follows.
1. The attacker includes shellcode in user input.
2. A buffer overflow changes the return address to point to the shellcode location.
3. The shellcode executes when the function returns.

**NOP-sled:** since hitting the exact address is difficult, a large number of NOP (no-operation) instructions are placed before the shellcode. If the jump lands anywhere in the NOP region, the CPU *slides* through them and reaches the shellcode.

```text
  [NOP NOP NOP NOP NOP ... NOP] [SHELLCODE]
   ←──── NOP-sled ────────→     ← payload
```

*Figure 3. Layout of a NOP sled and shellcode*

- **Script kiddie:** a novice attacker who uses exploits created by other hackers.

### 2.6 SQL Injection

An attack that inserts SQL statements into user input to manipulate the database.

```text
  Normal input:     username = "alice"
  SQL query:        SELECT * FROM users WHERE name = 'alice'

  Malicious input:  username = "' OR '1'='1"
  SQL query:        SELECT * FROM users WHERE name = '' OR '1'='1'
  → All records are returned (authentication bypass)
```

**Defenses:**
- Use **Prepared Statements** (parameterized queries).
- Apply input **validation** and **sanitization**.
- Grant minimum privileges to DB accounts.

```java
// Safe approach (Prepared Statement)
PreparedStatement stmt = conn.prepareStatement(
    "SELECT * FROM users WHERE name = ?");
stmt.setString(1, username);  // Automatic escaping
```

### 2.7 Viruses

**Virus:** self-replicating code that spreads by **inserting itself** into other programs.

- It **requires a host program** (it cannot run independently).
- It mainly spreads through user actions (executing files, email attachments).

| Virus Type      | Description                                       |
|-----------------|---------------------------------------------------|
| **File Virus**  | Inserts into executable files (parasitic virus)   |
| **Boot Virus**  | Infects the boot sector, executes before the OS loads |
| **Macro Virus** | Inserts into document macros (VBA, etc.), infects Office files |
| **Polymorphic** | Mutates code on each infection → evades signature detection |
| **Encrypted**   | Contains an encrypted virus + decryption code     |
| **Stealth**     | Evades detection by modifying system calls        |
| **Rootkit**     | Infects the OS itself, takes over the whole system |
| **Armored**     | Obfuscated to make analysis difficult             |

*Table 6. Types of viruses*

### 2.8 Worms

**Worm:** malware that propagates **autonomously** through networks.

- It **requires no host program** (it can run independently).
- It spreads automatically over the network and generates massive traffic.

**Virus vs Worm:**

| Property        | Virus                     | Worm                        |
|-----------------|---------------------------|-----------------------------|
| Host program    | Required                  | Not required                |
| Propagation     | User action (file execution) | Automatic via network    |
| Self-replication| Inserts into other programs | Replicates independently  |
| Primary damage  | File corruption, system failure | Network paralysis, resource exhaustion |

*Table 7. Comparison of viruses and worms*

- The **Morris Worm** (1988) was the first internet worm and exploited a buffer overflow.
- Modern worms are used to build botnets (DDoS, spam distribution).

---

<br>

## 3. System and Network Threats

### 3.1 Attacking Network Traffic: Sniffing, Spoofing, MITM

| Attack Type          | Description                                | Classification |
|----------------------|--------------------------------------------|----------------|
| **Sniffing**         | Eavesdropping on network packets to steal information | Passive attack |
| **Spoofing**         | Forging IP/MAC addresses to impersonate a trusted source | Active attack |
| **Man-in-the-Middle**| Intercepting and modifying data during communication | Active attack |

*Table 8. Types and classification of network-traffic attacks*

![Figure 4. Comparison of normal communication, masquerading, and man-in-the-middle](../../images/Week14_Figure-16-6.png)

*Figure 4. Comparison of normal communication, masquerading, and man-in-the-middle*

Figure 4, which corresponds to textbook Figure 16.6, compares standard security attacks. In normal communication the sender and receiver communicate directly; in masquerading the attacker poses as the sender and communicates with the receiver; in man-in-the-middle the attacker sits between the sender and receiver and relays both sides of the communication.

- **Zombie system:** a system hijacked by a hacker and used to conceal the source of attacks.
- **WarDriving:** searching for unprotected WiFi networks to gain access.

> **Note:** **Passive vs active.** Sniffing only *observes* data, so it is a **passive attack** that is hard to detect. Spoofing and MITM *manipulate* data, so they are **active attacks** that leave traces but cause greater damage.

### 3.2 Denial of Service: DoS and DDoS

**DoS:** an attack that disrupts normal service by sending excessive requests to a server.

| Type                  | Description                              |
|-----------------------|------------------------------------------|
| **Resource Exhaustion** | Depleting CPU, memory, bandwidth, etc. |
| **Network Disruption**  | Disabling the network itself           |
| **DDoS**              | Simultaneous attack from multiple zombies (botnet) |

*Table 9. Types of denial-of-service attacks*

**SYN Flood attack:**

```text
  Attacker → [SYN] → Server     (connection request)
  Server   → [SYN-ACK] → ???    (waiting for response)
  ※ ACK never arrives, half-open connections accumulate
  → Server's connection table is exhausted → legitimate connections denied
```

*Figure 5. Message flow of a SYN flood attack*

- DDoS is sometimes combined with **blackmail** (extortion).
- Defenses include rate limiting, upstream filtering, and adding resources.

### 3.3 Port Scanning

**Port Scanning:** probing open ports to discover vulnerable services.

- It is not an attack itself but a **reconnaissance** phase.
- Security administrators also use it for service auditing.

```text
  Attacker → Port 21 (FTP)     → No response (closed)
  Attacker → Port 22 (SSH)     → Response (open!) → check version
  Attacker → Port 80 (HTTP)    → Response (open!) → identify web server
  Attacker → Port 443 (HTTPS)  → Response (open!)
  ...
```

*Figure 6. Probing process of a port scan*

- **Fingerprinting:** a technique that identifies the OS type and service versions to search for known vulnerabilities.
- The key tools are as follows.
  - **nmap:** a network exploration and security-auditing tool.
  - **Metasploit:** a vulnerability exploit-testing framework.

---

<br>

## 4. Cryptography

### 4.1 Overview

**Cryptography:** technology that enables secure communication over untrusted channels (network environments).

The components of a cryptographic algorithm are as follows.
- **K** is the set of keys.
- **M** is the set of plaintext messages.
- **C** is the set of ciphertexts.
- **E**: K → (M → C) is the encryption function.
- **D**: K → (C → M) is the decryption function.

Key property: given a ciphertext $c$, recovering the original message $m$ without the key $k$ must be **computationally infeasible**.

| Type       | Key Structure       | Speed  | Key Exchange |
|------------|---------------------|--------|--------------|
| Symmetric  | Same key            | Fast   | Difficult    |
| Asymmetric | Public/private key pair | Slow | Secure     |

*Table 10. Overview of symmetric and asymmetric encryption*

### 4.2 Symmetric Encryption

Encryption and decryption use the **same key $k$**.

![Figure 7. Secure communication over an insecure medium](../../images/Week14_Figure-16-7.png)

*Figure 7. Secure communication over an insecure medium*

In Figure 7, which corresponds to textbook Figure 16.7, the sender writes a message $m$, and the encryption algorithm E produces the ciphertext $c = E_k(m)$ with the key $k$. The ciphertext travels over an insecure channel that the attacker can observe, and the decryption algorithm D on the receiver's side recovers $m = D_k(c)$ with the same key $k$. The two parties must share the key in advance through key exchange.

| Algorithm | Key Length        | Characteristics                               |
|-----------|-------------------|-----------------------------------------------|
| **DES**   | 56-bit            | No longer secure, vulnerable to brute force   |
| **3DES**  | 168-bit           | DES repeated 3 times: $c = E_{k3}(D_{k2}(E_{k1}(m)))$ |
| **AES**   | 128/192/256-bit   | Current standard, fast and secure             |

*Table 11. Representative symmetric encryption algorithms*

- **Block cipher:** encrypts in fixed-size blocks (e.g., the AES 128-bit block).
- **Stream cipher:** encrypts continuously byte by byte or bit by bit, using XOR operations.
- The drawback is that both sides must share the same key, which is called the **key-exchange problem**.

### 4.3 Asymmetric Encryption: RSA

Encrypt with the public key, decrypt with the private key.

```text
  Plaintext m ──[Public Key ke]──→ Ciphertext c ──[Private Key kd]──→ Plaintext m
                  Encryption                         Decryption
```

*Figure 8. Encryption and decryption flow of asymmetric encryption*

**RSA algorithm** (Rivest, Shamir, Adleman):
- $p, q$ are two large prime numbers, and $N = p \times q$.
- The public key $ke$ and private key $kd$ satisfy $ke \times kd \bmod (p-1)(q-1) = 1$.
- Encryption is $c = m^{ke} \bmod N$.
- Decryption is $m = c^{kd} \bmod N$.

**RSA example** (small numbers):

```text
  p=7, q=13 → N=91, (p-1)(q-1)=72
  ke=5, kd=29   (5×29 mod 72 = 1)
  Encryption: 69^5  mod 91 = 62
  Decryption: 62^29 mod 91 = 69
```

> **Key Point:** RSA's security rests on the assumption that **factoring a large composite is hard**. $N$ is public, but without knowing $p, q$ one cannot compute $kd$.

### 4.4 Symmetric vs Asymmetric and Hybrid

| Property      | Symmetric             | Asymmetric                      |
|---------------|-----------------------|---------------------------------|
| Key structure | One shared key        | Public key + private key pair   |
| Speed         | **Fast**              | Slow (tens to hundreds of times slower) |
| Key exchange  | Requires a secure channel | Public key freely distributed |
| Use cases     | Bulk data encryption  | Authentication, key exchange, small data |
| Representative algorithms | AES, DES  | RSA, ECC                        |

*Table 12. Comparison of symmetric and asymmetric encryption*

**Practical usage (hybrid encryption):**

```text
  1. Exchange a symmetric key securely using asymmetric encryption
  2. Subsequent communication uses (faster) symmetric encryption

  [Exchange AES key via RSA] → [Encrypt/decrypt data via AES]
```

*Figure 9. Two stages of hybrid encryption*

- TLS/SSL uses this approach. It combines the *secure key exchange* of asymmetric encryption with the *fast processing* of symmetric encryption.

### 4.5 Hash Functions

**Hash function $H(m)$:** a function that maps an arbitrary-length message to a **fixed-length hash value** (message digest).

Its properties are as follows.
- **One-way:** recovering the original from the hash value is computationally infeasible.
- **Collision Resistant:** finding two different inputs that produce the same hash is computationally infeasible.
- A 1-bit change in input **completely changes** the hash (avalanche effect).

```text
  "Hello World"  ──[SHA-256]──→ a591a6d40bf420404a011733cfb7b190...
  "Hello World!" ──[SHA-256]──→ 7f83b1657ff1fc53b92dc18148a1d65d...
```

| Algorithm | Output Length | Status            |
|-----------|---------------|-------------------|
| MD5       | 128-bit       | **Not secure**    |
| SHA-1     | 160-bit       | Being phased out  |
| SHA-256   | 256-bit       | **Current standard** |
| SHA-3     | Variable      | Latest standard   |

*Table 13. Output lengths and status of hash algorithms*

### 4.6 Hash Application: Password Storage and Salt

Store hash values instead of **plaintext passwords**.

```text
  Registration:   password → H(password) → store hash in DB
  Authentication: input → H(input) → compare with stored hash
```

*Figure 10. Password registration and authentication with hashes*

**Salt:** a random value appended to the password before hashing for enhanced security.

```text
  Without salt:  H("password123") → abc123...  (same password = same hash)
  With salt:     H("password123" + "x7k9") → def456...
                 H("password123" + "m2p5") → ghi789...
                 (same password, different salts → different hashes)
```

- **Dictionary-attack prevention:** with salt, every dictionary word must be re-hashed *for each salt*, so a precomputed rainbow table is rendered useless.
- The UNIX `/etc/shadow` file stores hashed passwords and is readable only by the superuser.

### 4.7 Digital Signatures

A mechanism for verifying the **integrity** and **origin** of a message.

- It uses public-key cryptography in **reverse**: sign with the private key, verify with the public key.

```text
  Signature creation:     signature = H(m)^ks mod N      (sign with private key ks)
  Signature verification: H(m) =? signature^kv mod N      (verify with public key kv)
```

**MAC (Message Authentication Code):**
- It authenticates with a symmetric key, so only key holders can create and verify a MAC.

**Applications of digital signatures:**
- **Nonrepudiation:** the signer cannot deny the action.
- **Code Signing:** verifies the integrity and origin of programs.
- **Electronic document signing:** used for digital contracts and certificates.

### 4.8 Digital Certificates

An electronic document in which a **Certificate Authority (CA)** vouches for the owner of a public key.

```text
  Certificate:
    Subject:    www.example.com
    Public Key: [RSA 2048-bit key]
    Issuer:     Trusted CA
    Validity:   2025-01-01 ~ 2026-12-31
    Signature:  [Signed with CA's private key]
```

*Figure 11. Example contents of a digital certificate*

**Certificate chain (Chain of Trust):**

```text
  Root CA (built into the browser)
    ↓ signs
  Intermediate CA
    ↓ signs
  Server certificate (www.example.com)
```

*Figure 12. Certificate chain from the root CA to the server certificate*

- **X.509:** the standard format for digital certificates.
- The CA's public key is **pre-installed in the browser**.
- Verifying the public key's authenticity via the certificate prevents MITM attacks.

### 4.9 TLS

The encryption protocol underlying HTTPS and the successor to SSL.

**TLS handshake process:**

```text
  Client                              Server
    │                                    │
    │──── ClientHello (nc) ────────→     │ (1) send random value
    │                                    │
    │←──── ServerHello (ns) ────────     │ (2) server random value +
    │←──── Certificate (certs) ────      │     send certificate
    │                                    │
    │   [Verify certificate]             │ (3) verify CA signature
    │                                    │
    │──── Eke(pms) ────────────→         │ (4) encrypt premaster secret
    │                                    │     with public key and send
    │                                    │
    │   ms = H(nc, ns, pms)              │ (5) both sides compute the
    │   (generate symmetric session key) │     same master secret
    │                                    │
    │←──── [AES encrypted comm] ────→    │ (6) communicate with symmetric key
```

*Figure 13. TLS handshake process*

> **Key Point:** Authenticate the server via its certificate, *securely agree on a symmetric key* via asymmetric crypto, then exchange the actual data with fast symmetric crypto (the hybrid encryption of Section 4.4).

### 4.10 The Key-Distribution Problem

**Symmetric key-exchange problem:**
- $N$ parties communicating pairwise require $N(N-1)/2$ keys.
- A secure channel is needed to deliver keys, and out-of-band exchange has low scalability.

**Asymmetric public-key distribution problem:**

```text
  Man-in-the-Middle on a public key:

  Alice ──→ [Request public key] ──→ Bob
                     ↑
              Mallory sends her
              own public key instead

  Alice encrypts with Mallory's public key
  → Mallory decrypts, re-encrypts with Bob's public key, and forwards
```

*Figure 14. Man-in-the-middle attack on a public key*

**Solution:** authenticate public keys with digital certificates. The CA vouches for the public key's owner, blocking MITM.

---

<br>

## 5. User Authentication

### 5.1 Authentication Methods: Three Factors

The three factors for verifying a user's identity are as follows.

| Factor                       | Category     | Examples                            |
|------------------------------|--------------|-------------------------------------|
| **Something you know**       | Knowledge    | Password, PIN, security questions   |
| **Something you have**       | Possession   | Smart card, OTP token, smartphone   |
| **Something you are**        | Biometrics   | Fingerprint, iris, facial recognition, voice |

*Table 14. The three factors of user authentication*

**Password-based authentication:**
- It is the most common but has various vulnerabilities.
- It involves a trade-off between convenience and security.

### 5.2 Password Vulnerabilities

| Attack Type           | Description                                    |
|-----------------------|------------------------------------------------|
| **Dictionary Attack** | Tries dictionary words and variations in sequence |
| **Brute Force**       | Tries all possible combinations (short passwords are vulnerable) |
| **Social Engineering**| Tricks users into directly revealing passwords |
| **Shoulder Surfing**  | Observing input over someone's shoulder        |
| **Sniffing**          | Stealing passwords from network traffic        |
| **Phishing**          | Inducing password entry via fake websites or emails |
| **Keylogger**         | Malware that records keyboard input            |

*Table 15. Password attack types*

- A 4-digit numeric password has 10,000 possibilities, so it is cracked in about 5,000 attempts on average.
- At 1 ms per attempt, it is cracked in **about 5 seconds**.
- Countermeasures are long passwords, a mix of uppercase, lowercase, digits, and special characters, and attempt limits.

### 5.3 One-Time Password (OTP)

**OTP:** a password usable only once, which prevents sniffing and replay attacks.

```text
  Challenge-Response method:
  1. Server → Client: send challenge (ch)
  2. Client: compute H(password, ch) → send authenticator
  3. Server: perform the same computation → compare results
  → Different challenge each time, so the authenticator differs each time
```

**OTP implementation methods:**
- Hardware tokens (RSA SecurID, etc.) are used.
- Software apps (Google Authenticator, etc.) are used.
- SMS authentication codes are used.

**Two-Factor Authentication (2FA):**
- It combines something you have (token) with something you know (PIN), so if one is compromised, the other still protects.

### 5.4 Multifactor Authentication (MFA)

Combining two or more authentication factors for enhanced security.

```text
  Single factor:  Password only → vulnerable

  2FA examples:   Password (knowledge) + OTP (possession)
                  Password (knowledge) + Fingerprint (biometrics)

  3FA example:    Password (knowledge) + Smart card (possession) + Facial recognition (biometrics)
```

| Auth Level   | Factors | Security Strength | Use Cases             |
|--------------|---------|-------------------|-----------------------|
| Single factor| 1       | Low               | General websites      |
| 2FA          | 2       | Medium            | Online banking        |
| 3FA          | 3       | High              | Military/government systems |

*Table 16. Security strength and use cases by authentication level*

**Biometrics pros and cons:**
- The pros are that biometrics cannot be lost or stolen and are highly unique.
- The cons are that false-recognition rates exist, that they *cannot be changed* (catastrophic if leaked), and that they raise privacy concerns.

---

<br>

## 6. Goals and Principles of Protection

### 6.1 Goals of Protection and Policy vs Mechanism

**Protection:** OS mechanisms that control resource access by processes and users.

Protection is needed for the following reasons.
1. It prevents malicious access violations.
2. It ensures system resources are used only according to defined policies.
3. **Early detection of interface errors** between components improves system reliability.

**Policy vs Mechanism:**

| Aspect    | Description           | Example                            |
|-----------|-----------------------|------------------------------------|
| Policy    | Decides **what** to do | The rule that students can only read their own files |
| Mechanism | Decides **how** to do it | Access matrix, ACL               |

*Table 17. Comparison of policy and mechanism*

- **Separation** of policy and mechanism is important, because the mechanism need not change when the policy changes. The separation of policy and mechanism seen in Week 6 CPU scheduling (the scheduler as policy, the dispatcher as mechanism) reappears directly in protection design.

### 6.2 Key Protection Principles

**Principle of Least Privilege:**
- Grant users and processes only the **minimum privileges** needed for their tasks.
- The UNIX practice of not running as root is a representative example.
- Excessive privileges expand the blast radius of errors or attacks.

**Compartmentalization (isolation):**
- Divide the system into independent zones.
- Prevent a breach in one zone from affecting others.
- Examples include DMZs, virtualization, and containers.

**Defense in Depth (layered defense):**
- Deploy multiple security layers so that if one is breached, the next defends.

```text
  [Firewall] → [IDS/IPS] → [OS Access Control] → [Application Auth] → [Data Encryption]
```

*Figure 15. Layers of defense in depth*

- **Audit Trail:** record access logs for attack detection and analysis.

### 6.3 The Need-to-Know Principle

**Need-to-Know:** a process should only access resources needed for its *current* task.

```text
  When Process P calls Procedure A():
  - A() can only access its local variables and passed parameters
  - It cannot access other variables of Process P

  When Process P invokes the Compiler:
  - The Compiler can only access related files (source, output, etc.)
  - It cannot access arbitrary files
```

| Concept         | Role                                |
|-----------------|-------------------------------------|
| Need-to-Know    | **Policy**: decides what access to allow |
| Least Privilege | **Mechanism**: the means to implement the policy |

*Table 18. Roles of need-to-know and least privilege*

---

<br>

## 7. Protection Rings

### 7.1 Protection-Ring Structure

Protection rings are a hardware-level privilege hierarchy based on the **Bell-LaPadula model**.

![Figure 16. Protection rings arranged as concentric circles](../../images/Week14_Figure-17-1.png)

*Figure 16. Protection rings arranged as concentric circles*

As Figure 16, which corresponds to textbook Figure 17.1, shows, the rings from ring 0 to ring N-1 are arranged as concentric circles, and inner rings have higher privileges.

- Ring $i$ provides only a *subset* of Ring $j$ ($j < i$) functionality.
- **Ring 0** has the highest privilege (full privileges).
- Transition to a higher-privilege ring is only possible through a **gate** such as a system call.

### 7.2 Intel and ARM Protection Rings

| Platform          | Structure           | Description                             |
|-------------------|---------------------|-----------------------------------------|
| **Intel x86**     | Ring 0-3            | Mostly only Ring 0 (kernel) and Ring 3 (user) are used |
| **Intel VT-x**    | Ring -1 added       | For the hypervisor (VMM)                |
| **ARM (early)**   | USR / SVC           | User mode / Supervisor mode             |
| **ARM TrustZone** | Secure / Normal World | Hardware-based secure-zone separation  |
| **ARMv8**         | EL0-EL3             | 4-level Exception Levels                |

*Table 19. Protection-ring structures of Intel and ARM*

![Figure 17. ARMv8 exception levels and transition instructions](../../images/Week14_Figure-17-3.png)

*Figure 17. ARMv8 exception levels and transition instructions*

In Figure 17, which corresponds to textbook Figure 17.3, apps at EL0 (user) enter EL1 (kernel) with the SVC instruction, the kernel enters EL2 (hypervisor) with HVC or SMC, the hypervisor enters EL3 (secure monitor) with SMC, and ERET returns to a lower level. TrustZone's trusted apps and secure kernel run in a secure world separated from the normal OS.

- **TrustZone:** protects on-chip cryptographic keys, which even the kernel cannot access directly.
- Android 5.0 and later actively use TrustZone for password and encryption-key protection.

---

<br>

## 8. Domain of Protection

### 8.1 Domain Structure

**Domain:** the set of resources and privileges a process can access.

```text
  Domain = { <object-name, rights-set>, ... }

  D1 = { <File1, {read}>, <File2, {read, write}> }
  D2 = { <File2, {read}>, <Printer, {write}> }
  D3 = { <File1, {execute}>, <File3, {read}> }
```

**Domain association methods:**
- **Static:** the domain is fixed for the process lifetime, so it may grant excessive privileges.
- **Dynamic:** the process switches between domains, so it can adhere to need-to-know.

**Domain implementations:**
- **User-based:** each user has a domain, and switching occurs on logout and login.
- **Process-based:** each process has a domain, and switching occurs via message passing.
- **Procedure-based:** each procedure has a domain, and switching occurs on function calls.

### 8.2 Domain Switching: setuid

A process moving from one domain to another is called domain switching.

```text
  User Process (Ring 3, User Domain)
       │
       │ system call (syscall instruction)
       ↓
  Kernel (Ring 0, Kernel Domain)    ← domain switch
       │
       │ return
       ↓
  User Process (Ring 3, User Domain)
```

*Figure 18. Domain switching through a system call*

**UNIX setuid mechanism:**
- Setting the setuid bit on an executable makes it run with the **file owner's privileges**.
- For example, `passwd` is owned by root and has setuid set, so regular users can also modify `/etc/shadow` through `passwd`.
- The risk is that if a setuid binary has a vulnerability, **privilege escalation** becomes possible.

**Android Application ID:**
- Each app is assigned a unique UID and GID, which isolates apps from each other.
- Each app owns its data directory `/data/data/<app-name>`.

---

<br>

## 9. Access Matrix

### 9.1 Access Matrix Overview

A model that represents resource access rights as a **matrix**.

| | F1 | F2 | F3 | Printer |
|------|------|------|------|---------|
| **D1** | read | read, write | | |
| **D2** | | read | read | write |
| **D3** | read, write | | read, write | |
| **D4** | read, write | | read, write | |

*Table 20. Example access matrix*

- **Row:** a domain (subject), such as a user, process, or role.
- **Column:** an object, such as a file, device, or memory region.
- **Cell:** access rights, such as read, write, or execute.

In real systems most of the matrix is **empty** (a sparse matrix), so efficient implementation methods are needed (Section 10).

### 9.2 Domain Switching in the Matrix

Include domains as **objects** to control domain switching.

| | F1 | F2 | F3 | Printer | D1 | D2 | D3 | D4 |
|------|------|------|------|---------|------|------|------|------|
| **D1** | read | | read | | | switch | | |
| **D2** | | read, write | | | | | switch | switch |
| **D3** | | | execute | | | | | |
| **D4** | read, write | | read, write | | switch | | | |

*Table 21. Access matrix with domains as objects*

- A `switch` in access(i, j) allows switching from domain $D_i$ to $D_j$.
- For example, $D_2$ can switch to $D_3$ or $D_4$.

### 9.3 Modifying Access Rights: Copy, Owner, Control

**Copy Right (`*`):**
- A right marked with `*`, such as `read*`, can be *copied* to another domain in the same column.
- Transfer means copying and then removing the original.
- Limited Copy means copying without `*`, and the copied right cannot be copied again.

**Owner Right:**
- If access(i, j) includes owner, $D_i$ can add or delete rights in any entry of column $j$.
- A file owner setting other users' access rights is an example of this.

**Control Right:**
- It applies only to domain objects.
- If access(i, j) includes control, $D_i$ can delete access rights in row $j$.

**Confinement Problem:** the problem of ensuring information does not leak outside the execution environment, which is generally **undecidable**.

---

<br>

## 10. Implementation of the Access Matrix

Storing the whole sparse matrix is inefficient, so real systems represent it *partially*.

### 10.1 Global Table

The simplest implementation is a complete table of `<domain, object, rights-set>` triples.

```text
  < D1, F1, {read} >
  < D1, F2, {read, write} >
  < D2, F2, {read} >
  < D2, Printer, {write} >
  < D3, F1, {read, write} >
  < D3, F3, {read, write} >
  ...
```

- **Advantages:** simple to implement.
- **Disadvantages:**
  - The table is very large and **hard to keep in main memory**, so extra I/O is required.
  - Grouping cannot be leveraged. For example, even the rule that all domains can read F1 requires an individual entry per domain.

### 10.2 Access List (ACL): Column-Based

Implemented based on the **columns (objects)** of the matrix. Each object has a (domain, rights) list.

```text
  File1:    [(D1, {read}), (D3, {read, write})]
  File2:    [(D1, {read, write}), (D2, {read})]
  File3:    [(D2, {read}), (D3, {read, write})]
  Printer:  [(D2, {write})]
```

These lists and the lists in Section 10.3 contain only the D1-D3 entries of the access matrix in Section 9.1 and omit the D4 entries.

- **Default Rights:** default access rights can be set for domains not in the list.
- On an access request, the object's ACL is searched to check the domain's rights.
- **Advantages:** directly addresses user needs (specify access rights when creating files).
- **Disadvantages:** it is hard to determine all rights of a specific domain. Also, an ACL search is required on every access, so long lists degrade performance.

### 10.3 Capability List: Row-Based

Implemented based on the **rows (domains)** of the matrix. Each domain has an (object, rights) list.

```text
  D1:  [(File1, {read}), (File2, {read, write})]
  D2:  [(File2, {read}), (File3, {read}), (Printer, {write})]
  D3:  [(File1, {read, write}), (File3, {read, write})]
```

- **Capability:** a *protected pointer* representing access rights to an object.
  - Possessing a capability grants access to the object.
  - Capabilities are managed by the OS; users cannot modify them directly.
- **Methods for protecting capabilities:**
  - **Tag method:** hardware uses a tag bit to distinguish capabilities from regular data.
  - **Separate address space:** capability lists are stored in separate memory accessible only by the OS.

### 10.4 Lock-Key Mechanism

A **compromise** between ACL and Capability List.

```text
  Assign a Lock (bit pattern) to each object:
    File1: Lock = [1010]
    File2: Lock = [0110]

  Assign a Key (bit pattern) to each domain:
    D1: Key = [1010]  → matches File1's Lock → access granted
    D2: Key = [0110]  → matches File2's Lock → access granted
```

- Access is granted if the Key matches the Lock.
- Keys can be freely transferred between domains.
- Changing the Lock enables effective **revocation** of rights.

### 10.5 Implementation Comparison

| Method           | Basis          | Advantages                    | Disadvantages                 |
|------------------|----------------|-------------------------------|-------------------------------|
| **Global Table** | Entire matrix  | Simple implementation         | Large table size, no grouping |
| **ACL**          | Column (object)| Intuitive for users, easy revocation | Hard to determine per-domain rights |
| **Capability**   | Row (domain)   | Fast access verification, process-centric | Difficult revocation       |
| **Lock-Key**     | Matching       | Flexible, easy revocation     | Key-management complexity     |

*Table 22. Comparison of access-matrix implementations*

**Real systems** use a **combination** of ACL and Capability.
- **File open:** access rights are verified via the ACL.
- After verification, a **capability (file descriptor)** is created, and subsequent access is verified quickly via the capability.
- **File close:** the capability is deleted.
- An example is the UNIX file system (open → fd → read/write → close).

### 10.6 Revocation of Access Rights

The considerations when revoking rights are as follows.

| Question | Options                          |
|----------|----------------------------------|
| Timing   | **Immediate** vs **Delayed**     |
| Scope    | **Selective** vs **General**     |
| Extent   | **Partial** vs **Total**         |
| Duration | **Temporary** vs **Permanent**   |

*Table 23. Considerations for revocation*

| Method            | ACL                          | Capability                          |
|-------------------|------------------------------|-------------------------------------|
| Ease of revocation| **Easy**: delete from the list | **Difficult**: must find distributed capabilities |
| Immediate effect  | Yes                          | Depends on the method               |

*Table 24. Comparison of revocation in ACL and capability*

**Capability revocation methods:**
- **Reacquisition:** periodically delete capabilities, requiring reacquisition.
- **Back-pointers:** each object maintains a list of capability pointers (MULTICS).
- **Indirection:** reference indirectly through a global table (CAL).
- **Keys:** invalidate existing capabilities by changing the master key.

---

<br>

## 11. Access-Control Models

### 11.1 RBAC: Role-Based Access Control

Access control through User → **Role** → Permission mapping.

```text
  Users          Roles            Permissions
  ┌──────┐      ┌──────────┐      ┌──────────────────┐
  │ Alice │──→  │  Admin   │──→   │ File: read/write │
  │  Bob  │──→  │  Editor  │──→   │ File: read/write │
  │ Carol │──→  │  Viewer  │──→   │ File: read       │
  └──────┘      └──────────┘      └──────────────────┘
```

*Figure 19. User, role, and permission mapping in RBAC*

**Advantages of RBAC:**
- Management is efficient at the **role level** even with many users.
- The **principle of least privilege** is easy to apply because only necessary roles are assigned.
- When a user leaves the organization, it suffices to remove the user from the role.
- Solaris 10 is a representative example that explicitly supports the principle of least privilege through RBAC.
- It is a *two-step* structure: assign privileges to roles, then assign roles to users.

### 11.2 DAC vs MAC

| Property         | DAC (Discretionary)      | MAC (Mandatory)            |
|------------------|--------------------------|----------------------------|
| Definition       | Resource owner sets access rights | System-enforced access policy |
| Permission setting | User (owner)           | System administrator/policy |
| Root privileges  | Root can access everything | **Root cannot change the policy** |
| Flexibility      | High                     | Low (subject to policy)    |
| Security strength| Relatively low           | High                       |
| Examples         | UNIX file permissions    | SELinux, macOS SIP         |

*Table 25. Comparison of DAC and MAC*

**Limitations of DAC:** owners can arbitrarily set and change permissions, and there is no restriction on root.
**Need for MAC:** even with root privileges, access can be blocked by policy, which is essential for military and government agencies.

### 11.3 MAC: Security Labels and SELinux

The core of MAC is **labels** (security labels).

- Labels are assigned to subjects (processes) and objects (files, devices).
- The policy determines whether access between labels is allowed or denied.

```text
  Security levels:  Unclassified < Secret < Top Secret

  "Secret" user:
    ✓ Can access "Unclassified" files
    ✓ Can access "Secret" files
    ✗ Cannot access "Top Secret" files (cannot even see they exist)
```

**SELinux (Security-Enhanced Linux):**
- It is a MAC implementation in the Linux kernel, developed by the NSA.
- It provides fine-grained access control via policy files.
- It is integrated into most Linux distributions.

**Other implementations:** macOS (TrustedBSD-based) and Windows (Mandatory Integrity Control).

### 11.4 Linux Capabilities

Subdivide root privileges to grant processes only the **necessary permissions**.

```text
  Traditional UNIX:  root = all privileges (all-or-nothing)

  Linux Capabilities:
    CAP_NET_BIND_SERVICE  → bind to ports below 1024
    CAP_SYS_ADMIN         → system administration tasks
    CAP_DAC_OVERRIDE      → override file access controls
    CAP_NET_RAW           → use raw sockets
    ... (managed as bitmasks)
```

**Three bitmasks:**
- **Permitted:** capabilities that are allowed.
- **Effective:** currently active capabilities.
- **Inheritable:** capabilities that can be inherited by child processes.

- Once a capability is **revoked, it cannot be reacquired**.
- It is a direct implementation of the **principle of least privilege**.
- Android also uses Linux capabilities so that system processes avoid running as root.

---

<br>

## 12. Sandboxing and Code Signing

### 12.1 Sandboxing

**Isolating** a program's execution environment to limit its impact on the system.

| Example         | Description                               |
|-----------------|-------------------------------------------|
| **Web Browser** | Isolates each tab or plugin in a separate process |
| **Mobile App**  | Independent filesystem and permission system per app |
| **Container**   | Application-environment isolation via Docker, etc. |
| **Virtual Machine** | Complete isolation at the hardware level |
| **Java/JVM**    | Sandbox restrictions at the virtual-machine level |

*Table 26. Examples of sandboxing*

**Sandboxing implementation methods:**
- **MAC policy-based:** SELinux labels are used (Android).
- **System-call filtering:** SECCOMP-BPF (Linux) limits the system calls a process can invoke.
- **Profile-based:** Apple Seatbelt (macOS) applies dynamic profiles written in the Scheme language, customized per binary.

### 12.2 Sandbox Profile Example

**macOS Sandbox Profile** (Scheme language):

```scheme
(version 1)
(deny default)                              ; Deny all operations by default
(allow file-chroot)                         ; Allow chroot
(allow file-read-metadata (literal "/var")) ; Allow reading /var metadata
(allow sysctl-read)                         ; Allow sysctl reads
(allow mach-per-user-lookup)
(allow mach-lookup
  (global-name "com.apple.system.logger"))  ; Allow access only to logger service
```

**SECCOMP-BPF** (Linux):
- System-call filters are defined in the Berkeley Packet Filter language.
- The filters are applied to processes via the `prctl()` system call.
- Child processes inherit the restrictions on fork.
- The Android Bionic C library applies it automatically to all apps.

### 12.3 Code Signing

Verify code **integrity** and **origin** through **digital signatures**.

```text
  Developer                          User System
  ┌────────┐                        ┌──────────┐
  │ Code   │──[Private Key]──→ Signature
  └────────┘                        │
  Code + Signature ──[Public Key]──→ Integrity/origin check
                                    ├─ Success: allow execution
                                    └─ Failure: block execution or warn
```

*Figure 20. Creation and verification flow of code signing*

| OS          | Implementation                             |
|-------------|--------------------------------------------|
| **Windows** | Driver Signing, Authenticode               |
| **macOS**   | Gatekeeper, SIP (System Integrity Protection) |
| **iOS**     | All apps require signing; Apple signs for App Store distribution |
| **Android** | APK Signing (developer signature)          |

*Table 27. Code-signing implementations by operating system*

- When unsigned code is executed, the system **blocks it or warns**.
- Apple **Entitlements:** permissions declared in an XML plist and included in the code signature.

### 12.4 System Integrity Protection (SIP)

A **system-protection mechanism** introduced in macOS 10.11.

**SIP features:**
- It restricts access to system files and resources.
- **Even the root user** cannot modify system files.
- It allows only code-signed kernel extensions.
- It prevents debugging and tampering of system binaries.

```text
  Traditional UNIX:
    root → can access/modify all files

  With SIP:
    root → can manage other users' files
           can install/remove programs
           ✗ cannot modify system files
           ✗ cannot tamper with kernel extensions
```

- SIP is enforced on all processes at boot time.
- Only Apple-signed binaries with **Entitlements** are exceptions.

> **Note:** SIP applies the MAC thinking of Section 11.2 to macOS. Under DAC, root would be omnipotent, but under SIP (a MAC) even root is bound by policy.

---

<br>

## 13. Lab: Implementing an Access-Control System

The concepts of Sections 9-11 implemented in Python.

**Access Matrix:**

```python
class AccessMatrix:
    def __init__(self):
        self.matrix = {}  # {domain: {object: set(rights)}}

    def add_right(self, domain, obj, right):
        if domain not in self.matrix:
            self.matrix[domain] = {}
        if obj not in self.matrix[domain]:
            self.matrix[domain][obj] = set()
        self.matrix[domain][obj].add(right)

    def check_access(self, domain, obj, right):
        if domain in self.matrix:
            if obj in self.matrix[domain]:
                return right in self.matrix[domain][obj]
        return False

    def revoke_right(self, domain, obj, right):
        if domain in self.matrix:
            if obj in self.matrix[domain]:
                self.matrix[domain][obj].discard(right)
```

**ACL and Capability List** (column-based and row-based, respectively):

```python
class ACL:
    """Column (object) based — (domain, rights) list per object"""
    def __init__(self):
        self.acl = {}  # {object: {domain: set(rights)}}

    def grant(self, obj, domain, right):
        self.acl.setdefault(obj, {}).setdefault(domain, set()).add(right)

    def check(self, obj, domain, right):
        return right in self.acl.get(obj, {}).get(domain, set())

    def revoke(self, obj, domain, right):
        if obj in self.acl and domain in self.acl[obj]:
            self.acl[obj][domain].discard(right)


class CapabilityList:
    """Row (domain) based — (object, rights) list per domain"""
    def __init__(self):
        self.caps = {}  # {domain: {object: set(rights)}}

    def grant(self, domain, obj, right):
        self.caps.setdefault(domain, {}).setdefault(obj, set()).add(right)

    def check(self, domain, obj, right):
        return right in self.caps.get(domain, {}).get(obj, set())
```

**RBAC:**

```python
class RBAC:
    def __init__(self):
        self.roles = {}          # {role: {object: set(rights)}}
        self.user_roles = {}     # {user: set(roles)}

    def add_role(self, role):
        if role not in self.roles:
            self.roles[role] = {}

    def assign_permission(self, role, obj, right):
        if role not in self.roles:
            self.roles[role] = {}
        if obj not in self.roles[role]:
            self.roles[role][obj] = set()
        self.roles[role][obj].add(right)

    def assign_role(self, user, role):
        if user not in self.user_roles:
            self.user_roles[user] = set()
        self.user_roles[user].add(role)

    def check_access(self, user, obj, right):
        if user not in self.user_roles:
            return False
        for role in self.user_roles[user]:
            if role in self.roles and obj in self.roles[role]:
                if right in self.roles[role][obj]:
                    return True
        return False
```

**Simulation:**

```python
# Access Matrix test
am = AccessMatrix()
am.add_right("D1", "File1", "read")
am.add_right("D1", "File2", "read")
am.add_right("D1", "File2", "write")
am.add_right("D2", "File2", "read")
am.add_right("D2", "Printer", "write")

print(am.check_access("D1", "File2", "write"))   # True
print(am.check_access("D2", "File2", "write"))   # False

# RBAC test
rbac = RBAC()
rbac.add_role("admin")
rbac.add_role("editor")
rbac.add_role("viewer")
rbac.assign_permission("admin", "File1", "read")
rbac.assign_permission("admin", "File1", "write")
rbac.assign_permission("viewer", "File1", "read")
rbac.assign_role("Alice", "admin")
rbac.assign_role("Bob", "viewer")

print(rbac.check_access("Alice", "File1", "write"))  # True
print(rbac.check_access("Bob", "File1", "write"))    # False
```

> **Note:** **Extension exercise.** The implementation above can be extended with role addition and deletion, permission revocation, and access logging (audit trail).

---

<br>

## Summary

| Concept | Key Summary |
|---------|-------------|
| Security and protection | Security is an outward-facing measure of confidence in system integrity (Ch 16), while protection is the OS mechanism that realizes it (Ch 17). |
| CIA | The three principles are confidentiality, integrity, and availability, and every attack ultimately breaks one or more of them. |
| Malware | Malware is classified into Trojan, Spyware, Ransomware, Back Door, Logic Bomb, and other types. |
| Code injection | Buffer overflow, SQL injection, and shellcode with NOP sleds are typical, and many program threats stem from code injection and least-privilege violations. |
| Virus vs worm | The two differ in host requirement and propagation method. |
| Network threats | Sniffing, spoofing, DoS/DDoS, MITM, and port scanning target network traffic and service resources. |
| Cryptography | It uses symmetric (AES), asymmetric (RSA), hashing (SHA), and salt, and guarantees integrity, origin, and identity with hashing (+salt), digital signatures, and certificates. |
| TLS | It uses hybrid encryption that combines symmetric (fast) and asymmetric (secure key exchange) encryption, starting with a certificate-based handshake. |
| Authentication | It uses passwords, biometrics, OTP, and MFA, and the authentication factors are something you know, something you have, and something you are. |
| Protection principles | The principles are least privilege, compartmentalization, defense in depth, and need-to-know, and following the two pillars of protection design (policy and mechanism separation, and least privilege), need-to-know (policy) is implemented by least privilege (mechanism). |
| Protection rings | Intel rings, ARM exception levels, and TrustZone are hardware-level privilege hierarchies. |
| Access matrix | It is the abstract model of protection, and real implementations combine ACL (column-based) and capability (row-based), verifying with the ACL on open and handling later accesses quickly via the fd capability. Lock-key is a compromise between the two. |
| RBAC/MAC/DAC | These are role-based, mandatory, and discretionary access control, and MAC, which binds even root by policy, is the strongest. |
| Sandboxing | It isolates processes with SECCOMP-BPF, Apple Seatbelt, and similar mechanisms. |
| Code signing | It verifies code integrity and origin with digital signatures, SIP, and Entitlements. |
| Layered protection mechanisms | RBAC/MAC/DAC, protection rings, sandboxing, and code signing/SIP all implement least privilege and defense in depth at different layers. |
| Defense in depth | Security is only as strong as its weakest link, so if one layer among crypto, authentication, and access control is breached, the next must hold. |

---

<br>

## Self-Check Questions

1. **Security vs Protection, CIA:** Distinguish security from protection, and state which of the CIA triad is violated by eavesdropping, data tampering, and a DoS attack.

   <details>
   <summary>Answer</summary>

   **Security** is a *measure of confidence* that the integrity of a system and its data will be preserved, an outward-facing notion against external threats (attackers, malware, networks; Ch 16). **Protection** is the *internal OS mechanism* (Ch 17) that controls processes' and users' access to resources, the foundation for and a subset of security. The CIA violations are as follows. **Eavesdropping** is unauthorized *access*, so it violates **Confidentiality**; **data tampering** is unauthorized *modification*, so it violates **Integrity**; and **DoS** *blocks* legitimate users' *access*, so it violates **Availability**.

   </details>

2. **Staged consequences of buffer overflow:** Explain how a buffer overflow worsens from harmless to a crash to arbitrary code execution as input size grows, with reference to the stack layout.

   <details>
   <summary>Answer</summary>

   The stack frame is laid out from low addresses as `buffer[64]`, `local variables`, `Saved EBP`, `Return Address` (high addresses), and the overflow overwrites adjacent memory from low to high addresses. (1) **Small overflow:** stops within the padding range, with no effect. (2) **Medium overflow:** overwrites adjacent *local variables* and *Saved EBP* (frame pointer), causing bad values or crashes. (3) **Large overflow:** overwrites the **Return Address**, so on function return control flow jumps to an attacker-chosen address (injected shellcode), resulting in **arbitrary code execution**. The defenses are size-aware functions (`strncpy` instead of `strcpy`) and the stack canaries, ASLR, and NX bit provided by the OS and compiler.

   </details>

3. **Role of the NOP-sled:** Explain why a NOP-sled is needed in a shellcode attack and what it makes easier.

   <details>
   <summary>Answer</summary>

   The attacker must overwrite the return address with the *exact* start address of the shellcode, but stack addresses vary with the environment and ASLR and are hard to hit precisely. A **NOP-sled** is a region of many NOP instructions (which do nothing and proceed to the next instruction) placed before the shellcode. If the jump lands *anywhere* in the NOP region, the CPU slides through the NOPs and eventually reaches the shellcode. So the NOP-sled enlarges the target to hit from a single point to a wide range, raising the attack's success probability. A novice attacker who merely reuses such exploits is called a *script kiddie*.

   </details>

4. **Virus vs Worm:** Compare the two malware types along host requirement, propagation, and primary damage. What does the Morris Worm illustrate?

   <details>
   <summary>Answer</summary>

   A **virus** *requires a host program* (it cannot run independently), *inserts itself* into other programs, spreads mainly through *user actions* (running files, opening attachments), and causes file corruption and system failure. A **worm** *requires no host* (it runs independently), replicates and spreads *autonomously* over the *network*, and causes network paralysis and resource exhaustion. The **Morris Worm (1988)**, the first internet worm, exploited a *buffer overflow* to spread without human intervention. It shows that a code vulnerability can become a channel for autonomous propagation and that network connectivity *amplifies* threats. Modern worms build botnets for DDoS and spam.

   </details>

5. **Symmetric vs Asymmetric and Hybrid:** Compare the speed and key-exchange properties of the two ciphers and explain why TLS combines them.

   <details>
   <summary>Answer</summary>

   **Symmetric encryption** (AES, DES) uses the *same key* for encryption and decryption, so it is **fast**, but both sides must share the key, raising the *key-exchange problem* (it needs a secure channel, and $N$ parties need $N(N-1)/2$ keys). **Asymmetric encryption** (RSA, ECC) uses a *public/private key pair*, so the public key can be freely distributed and *key exchange is secure*, but it is tens to hundreds of times **slower**. **Hybrid (TLS)** takes the best of both. ① It uses the slow but secure *asymmetric cipher* to exchange and agree on a symmetric session key (premaster secret), and ② it handles bulk data with the fast *symmetric cipher (AES)*. The design gains the key-exchange security of asymmetric encryption and the processing performance of symmetric encryption at once.

   </details>

6. **Role of salt:** What attack does salt prevent when storing passwords as hashes, and how?

   <details>
   <summary>Answer</summary>

   Without salt, storing only the hash means *the same password always yields the same hash*. An attacker can therefore use a precomputed table of common-password hashes (a **rainbow table**) to reverse them at once, or instantly identify users with the same hash in a leaked DB. A **salt** is a *random value* appended per password, and the system stores `H(password + salt)`. With different salts, the same password produces different hashes, so the attacker must re-hash the dictionary *for each salt*. As a result, precomputed tables become useless (preventing dictionary and rainbow-table attacks), and users sharing a password get different hashes, isolating exposure. UNIX `/etc/shadow` is the canonical application.

   </details>

7. **Policy vs Mechanism, Need-to-Know vs Least-Privilege:** Explain how the two pairs of concepts correspond.

   <details>
   <summary>Answer</summary>

   **Policy** decides *what* to allow and forbid, and **Mechanism** decides *how* to enforce it. Separating them means a policy change does not require modifying the mechanism (access matrix, ACL). This distinction maps directly onto protection. **Need-to-Know** is the *policy* that a process accesses only the resources needed for its current task, while **Least-Privilege** is the *mechanism (means)* that grants only the minimum privileges to realize that policy. That is, need-to-know (policy) is implemented by least-privilege (mechanism). Both aim to narrow the blast radius when a breach occurs.

   </details>

8. **Protection rings and domain switching:** Explain the Ring 0 and Ring 3 structure and the domain switch that occurs when a user process needs kernel functionality. What is the risk of setuid?

   <details>
   <summary>Answer</summary>

   **Protection rings** are a hardware privilege hierarchy: Ring $i$ has only a subset of Ring $j$'s ($j<i$) functionality, and **Ring 0** has the highest privilege. Intel typically uses only Ring 0 (kernel) and Ring 3 (user). When a user process (Ring 3, User Domain) needs kernel functionality, it can only **domain-switch** to Ring 0 (Kernel Domain) through a **gate (system call)**, returning to Ring 3 afterward. This structure prevents arbitrary entry into a higher ring. **Risk of setuid:** an executable with the setuid bit runs with the *file owner's privileges* (often root; e.g., `passwd` modifying `/etc/shadow`). If that binary has a vulnerability (e.g., a buffer overflow), a regular user can exploit it to seize root privileges, making **privilege escalation** possible.

   </details>

9. **ACL vs Capability:** Compare the pros and cons of implementing the access matrix column-based (ACL) vs row-based (Capability), and explain how real OSes combine them.

   <details>
   <summary>Answer</summary>

   An **ACL** (column, that is, object-based) keeps a (domain, rights) list per object. It lets one intuitively specify who can do what at file creation, and *revocation is easy* (delete from the list). The downsides are that determining *all rights a given domain holds* requires scanning every object, and every access needs a list search. A **Capability** list (row, that is, domain-based) keeps an (object, rights) list per domain. It is process-centric, so *access verification is fast*. The downside is that *revocation is hard* because distributed capabilities must all be found. **Real combination:** on file **open**, rights are verified once via the ACL and a **capability (file descriptor)** is issued, subsequent reads and writes are verified quickly via the fd, and on **close** the capability is deleted. UNIX's open → fd → read/write → close is exactly this pattern.

   </details>

10. **DAC vs MAC, SIP:** Explain the core difference between the two access-control models from the root-privilege angle, and state which one macOS SIP belongs to.

    <details>
    <summary>Answer</summary>

    **DAC (discretionary)** lets the resource *owner* set and change permissions arbitrarily, and **root can access everything** (UNIX file permissions). **MAC (mandatory)** has *system policy* enforce access, and the key point is that **even root cannot change the policy**. A label-based policy (e.g., Unclassified < Secret < Top Secret) decides allow or deny (SELinux). It is essential where access must be blocked even after root compromise, such as in the military and government. **macOS SIP** is a **MAC**. In traditional UNIX root is omnipotent, but under SIP even root *cannot* modify system files or tamper with kernel extensions, and only Apple-signed binaries with Entitlements are exceptions. That is, SIP is MAC-style protection that binds root by policy.

    </details>

11. **Sandboxing implementations:** Compare what SECCOMP-BPF and Apple Seatbelt each restrict and how they achieve isolation.

    <details>
    <summary>Answer</summary>

    **SECCOMP-BPF (Linux)** isolates at the *system-call* level. It defines a whitelist or blacklist of the syscalls a process may invoke in the Berkeley Packet Filter language, applies it with `prctl()`, and the filter is inherited by children on fork (Android Bionic applies it automatically to all apps). It reduces the attack surface by preventing a process from calling disallowed kernel interfaces at all. **Apple Seatbelt (macOS)** is *profile*-based: it applies a policy written in the Scheme language (`(deny default)` followed by `allow` for only the needed operations) to each binary, finely restricting *resource access* such as files, Mach services, and sysctl. Both confine a program's impact on the system through least privilege by allowing only what is needed, but they differ in *layer*: one is a syscall filter and the other a resource-access profile.

    </details>

12. **Synthesize via defense in depth:** Given an attacker targeting a web server, weave a single scenario showing how the mechanisms covered in this unit (from firewall to data encryption) defend in *successive layers*.

    <details>
    <summary>Answer</summary>

    The essence of **Defense in Depth** is that if one layer is breached, the next holds. The scenario is as follows. ① A **firewall and port policy** close unneeded ports, shrinking *port scanning* reconnaissance and mitigating *SYN floods* via rate limiting. ② Traffic that gets through is protected from *sniffing and MITM* by **TLS** (certificate-based server authentication, hybrid crypto). ③ Inputs are blocked from *SQL injection* by **prepared statements and validation** and from *buffer overflow* by size-aware functions, ASLR, and NX. ④ If code execution still succeeds, **sandboxing (SECCOMP, Seatbelt)** and **least privilege (Linux capabilities, non-root)** confine what resources the process can touch. ⑤ Even on a privilege-escalation attempt, **MAC (SELinux, SIP)** binds even root by policy and protects system files. ⑥ Finally, **data-at-rest encryption** and an **audit trail** reduce leakage damage and enable post-incident analysis. Each layer is independent, and the attacker must breach *every* layer in turn. This is the defensive application of the maxim "security is only as strong as its weakest link."

    </details>
