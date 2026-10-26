# Requirements Specification: Embedded Sensor Monitoring & Fault Controller

**Project:** Project 1 — Embedded Sensor Monitoring & Fault Controller  
**Language Standard:** ISO C11 (`-std=c11`)  
**Target Architecture:** Modular Host Simulation (x86_64 Windows via MSYS2 UCRT64 GCC) designed for cross-platform microcontroller portability (ARM Cortex-M / RISC-V).  

---

## 1. System Overview

The system is a safety-critical Embedded Sensor Monitoring and Fault Controller designed to continuously monitor thermal sensor telemetry, validate incoming analog-to-digital or digital sensor readings, evaluate strict operational safety thresholds, and transition between operational states deterministically.

---

## 2. Operating States and Threshold Definitions

The controller operates as a deterministic Finite State Machine (FSM) with three operational states:

| State | Operational Condition | Temperature Range ($T$) | Sensor Health Status |
| :--- | :--- | :--- | :--- |
| **`STATE_NORMAL`** | Nominal continuous operation | $T < 60.0^\circ\text{C}$ (e.g., $-40.0^\circ\text{C} \le T < 60.0^\circ\text{C}$) | Valid (`SENSOR_OK`) |
| **`STATE_WARNING`** | Elevated thermal condition; requires throttling/alerting | $60.0^\circ\text{C} \le T \le 79.9^\circ\text{C}$ | Valid (`SENSOR_OK`) |
| **`STATE_FAULT`** | Critical over-temperature OR sensor malfunction | $T \ge 80.0^\circ\text{C}$ **OR** invalid sensor input | `SENSOR_ERR_*` OR $T \ge 80.0^\circ\text{C}$ |

### 2.1 Boundary Rules (Exact Transitions)
- **Normal to Warning Boundary:**
  - $T = 59.9^\circ\text{C} \implies \text{NORMAL}$
  - $T = 60.0^\circ\text{C} \implies \text{WARNING}$
- **Warning to Fault Boundary:**
  - $T = 79.9^\circ\text{C} \implies \text{WARNING}$
  - $T = 80.0^\circ\text{C} \implies \text{FAULT}$
- **Physical Sensor Limits:**
  - Valid physical operating range: $-40.0^\circ\text{C} \le T \le 150.0^\circ\text{C}$.
  - Any reading $T < -40.0^\circ\text{C}$ or $T > 150.0^\circ\text{C}$ is treated as out-of-range sensor failure $\implies \text{FAULT}$.
- **Invalid Sensor Inputs:**
  - Sensor disconnected / open circuit (`SENSOR_DISCONNECTED`).
  - ADC read timeout / bus error (`SENSOR_COMM_ERROR`).
  - Corrupted / NaN / Inf floating-point payload (`SENSOR_DATA_INVALID`).
  - Any invalid sensor input immediately forces `STATE_FAULT`.

---

## 3. Functional Requirements (FR)

- **FR-01: Sensor Telemetry Ingestion & Validation**
  The controller shall accept a structured reading containing:
  - Sensor raw/converted temperature value (`float`).
  - Sensor hardware status flags (`SensorStatus` enum).
  - Validation shall check for physically plausible values, NaN/Inf, and hardware error flags before state evaluation.

- **FR-02: Deterministic State Evaluation**
  State evaluation shall be a pure, deterministic function. Given identical inputs and current state, the resulting state and fault code shall be identical.

- **FR-03: Fault Latching & Safety Recovery Policy**
  - **Thermal Fault Latching:** When `STATE_FAULT` is triggered due to critical over-temperature ($T \ge 80.0^\circ\text{C}$), the system shall latch in `STATE_FAULT` until temperature drops below the safe recovery threshold ($T < 75.0^\circ\text{C}$, providing $5.0^\circ\text{C}$ hysteresis) or an explicit reset command is issued.
  - **Hardware Fault Latching:** Hardware disconnection or communication errors latch in `STATE_FAULT` until sensor health is restored and acknowledged.

- **FR-04: Structured Status Reporting**
  The system shall generate structured telemetry outputs containing:
  - Current state (`NORMAL`, `WARNING`, `FAULT`).
  - Processed temperature reading (formatted to 2 decimal places).
  - Active fault code (`NONE`, `OVER_TEMPERATURE`, `UNDER_RANGE`, `OVER_RANGE`, `DISCONNECTED`, `COMM_ERROR`, `DATA_INVALID`).

- **FR-05: Strict Separation of Logic and I/O**
  The core controller (`sensor_controller.c`) shall contain zero console I/O (`printf`), file operations, or platform-specific syscalls. All I/O is delegated to the CLI wrapper (`main.c`) or test runners.

---

## 4. Non-Functional Requirements (NFR)

- **NFR-01: ISO C11 Compliance**
  Code must strictly adhere to the C11 standard (`-std=c11`).
- **NFR-02: Zero Dynamic Memory Allocation**
  No `malloc`, `calloc`, `realloc`, or `free` calls. All state, structures, and buffers must be statically or stack allocated to prevent memory leaks and fragmentation in long-running embedded loops.
- **NFR-03: Strict Compiler Diagnostics**
  Code must compile with zero warnings using:
  `-Wall -Wextra -Wpedantic -std=c11 -O2` under MSYS2 UCRT64 GCC.
- **NFR-04: Execution Time Determinism**
  Every evaluation cycle shall execute in constant time $O(1)$ without unbounded loops or blocking operations.

---

## 5. Verification Test Matrix

| Test ID | Test Category | Input Value / Condition | Expected State | Expected Fault Code |
| :--- | :--- | :--- | :--- | :--- |
| **TC-01** | Nominal Room Temp | $T = 25.0^\circ\text{C}$, Status = OK | `NORMAL` | `FAULT_NONE` |
| **TC-02** | Freezing Point | $T = 0.0^\circ\text{C}$, Status = OK | `NORMAL` | `FAULT_NONE` |
| **TC-03** | Sub-Zero Operating | $T = -20.0^\circ\text{C}$, Status = OK | `NORMAL` | `FAULT_NONE` |
| **TC-04** | Normal Integer Boundary | $T = 59.0^\circ\text{C}$, Status = OK | `NORMAL` | `FAULT_NONE` |
| **TC-05** | Normal Float Boundary | $T = 59.9^\circ\text{C}$, Status = OK | `NORMAL` | `FAULT_NONE` |
| **TC-06** | Warning Lower Boundary | $T = 60.0^\circ\text{C}$, Status = OK | `WARNING` | `FAULT_NONE` |
| **TC-07** | Warning Nominal Mid | $T = 70.0^\circ\text{C}$, Status = OK | `WARNING` | `FAULT_NONE` |
| **TC-08** | Warning Integer High | $T = 79.0^\circ\text{C}$, Status = OK | `WARNING` | `FAULT_NONE` |
| **TC-09** | Warning Float High | $T = 79.9^\circ\text{C}$, Status = OK | `WARNING` | `FAULT_NONE` |
| **TC-10** | Fault Lower Boundary | $T = 80.0^\circ\text{C}$, Status = OK | `FAULT` | `FAULT_OVER_TEMP` |
| **TC-11** | Extreme Over-Temp | $T = 120.0^\circ\text{C}$, Status = OK | `FAULT` | `FAULT_OVER_TEMP` |
| **TC-12** | Sensor Disconnected | $T = 25.0^\circ\text{C}$, Status = DISCONNECTED | `FAULT` | `FAULT_SENSOR_DISCONNECTED` |
| **TC-13** | Bus / Comm Timeout | $T = 25.0^\circ\text{C}$, Status = COMM_TIMEOUT | `FAULT` | `FAULT_SENSOR_COMM_ERROR` |
| **TC-14** | Physical Under-Range | $T = -45.0^\circ\text{C}$, Status = OK | `FAULT` | `FAULT_SENSOR_UNDER_RANGE` |
| **TC-15** | Physical Over-Range | $T = 160.0^\circ\text{C}$, Status = OK | `FAULT` | `FAULT_SENSOR_OVER_RANGE` |
| **TC-16** | Arithmetic NaN | $T = \text{NaN}$, Status = OK | `FAULT` | `FAULT_SENSOR_INVALID_DATA` |
| **TC-17** | Defensive NULL Context | `ctx = NULL`, Valid Reading | Safe Return (No crash) | N/A |
| **TC-18** | Defensive NULL Reading | Valid Context, `reading = NULL` | `FAULT` | `FAULT_SENSOR_INVALID_DATA` |
| **TC-19** | Initial Over-Temp Trigger | $T = 85.0^\circ\text{C}$, Status = OK | `FAULT` | `FAULT_OVER_TEMP` |
| **TC-20** | Hysteresis Rejection | $T = 78.0^\circ\text{C}$ (Attempts clear when $T \ge 75^\circ\text{C}$) | `FAULT` (Latched) | `FAULT_OVER_TEMP` |
| **TC-21** | Hysteresis Acceptance | $T = 50.0^\circ\text{C}$ (Attempts clear when $T < 75^\circ\text{C}$) | `NORMAL` | `FAULT_NONE` |
| **TC-22** | Priority: Disconnect vs Over-Temp | $T = 95.0^\circ\text{C}$, Status = DISCONNECTED | `FAULT` | `FAULT_SENSOR_DISCONNECTED` |
| **TC-23** | Priority: Comm Timeout vs Over-Temp | $T = 85.0^\circ\text{C}$, Status = COMM_TIMEOUT | `FAULT` | `FAULT_SENSOR_COMM_ERROR` |
| **TC-24** | Hardware Out-of-Bounds Low | $T = -42.0^\circ\text{C}$, Status = OUT_OF_BOUNDS | `FAULT` | `FAULT_SENSOR_UNDER_RANGE` |
| **TC-25** | Hardware Out-of-Bounds High | $T = 155.0^\circ\text{C}$, Status = OUT_OF_BOUNDS | `FAULT` | `FAULT_SENSOR_OVER_RANGE` |
| **TC-26** | Dynamic Transition 1 | Initial $T = 25.0^\circ\text{C}$ | `NORMAL` | `FAULT_NONE` |
| **TC-27** | Dynamic Transition 2 | Thermal Rise $T = 65.0^\circ\text{C}$ | `WARNING` | `FAULT_NONE` |
| **TC-28** | Dynamic Transition 3 | Cooling $T = 50.0^\circ\text{C}$ | `NORMAL` | `FAULT_NONE` |
| **TC-29** | Dynamic Transition 4 | Thermal Spike $T = 85.0^\circ\text{C}$ | `FAULT` | `FAULT_OVER_TEMP` |
| **TC-30** | Dynamic Transition 5 | Partial Cool $T = 76.0^\circ\text{C}$ | `FAULT` (Latched) | `FAULT_OVER_TEMP` |
| **TC-31** | Dynamic Transition 6 | Safe Recovery $T = 45.0^\circ\text{C}$ | `NORMAL` | `FAULT_NONE` |

---

## 6. Limitations of Host-Based Simulation

While host-based compilation on Windows via MSYS2 GCC provides high-speed automated verification, the following embedded domain limitations apply:
1. **No Hardware Registers / Bus Clocks:** Peripheral registers (e.g., `ADC1->DR`, `I2C1->CR1`, SPI DMA buffers) are simulated via software data structures.
2. **Floating Point Unit (FPU) Representation:** Host x86_64 features IEEE-754 hardware floating-point units. On lower-end microcontrollers (e.g., ARM Cortex-M0/M3), floating-point math requires software emulation or fixed-point integer scaling ($Q$-format / milli-degrees Celsius).
3. **Interrupt Latency & RTOS Preemption:** Host execution is run sequentially in user-space processes; real-world embedded targets experience ISR preemption, tick jitters, and priority inversion considerations.
