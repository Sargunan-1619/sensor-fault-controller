# Embedded Sensor Monitoring & Fault Controller (Project 1)

A modular, safety-critical Embedded C sensor monitoring and fault controller running on Windows via MSYS2 UCRT64 GCC, designed in accordance with MISRA-C and AUTOSAR architectural principles.

---

## 1. Project Overview

This project implements a deterministic thermal sensor monitoring and fault controller:
- **`STATE_NORMAL`**: Temperature $< 60.0^\circ\text{C}$ and healthy sensor telemetry.
- **`STATE_WARNING`**: Temperature $60.0^\circ\text{C}$ through $79.99^\circ\text{C}$ and healthy sensor telemetry.
- **`STATE_FAULT`**: Temperature $\ge 80.0^\circ\text{C}$ **or** invalid sensor telemetry (sensor disconnected, communication timeout, out-of-physical range, or arithmetic NaN/Inf).

The architecture strictly decouples core control logic (`src/sensor_controller.c`) from input/output and CLI interfaces (`src/main.c`), enabling both standalone deployment and comprehensive native unit testing (`tests/test_sensor_controller.c`).

---

## 2. Key Architectural Features

- **Layered Modular Design:** Pure logic engine isolated from hardware abstraction and application layers.
- **Zero Dynamic Memory Allocation:** Uses $0$ bytes of heap (`malloc`/`free`); static and stack allocation guarantee deterministic execution and zero memory fragmentation.
- **Defensive Telemetry Validation:** Validates IEEE-754 arithmetic (`isnan`, `isinf`), physical sensor transducer limits ($-40.0^\circ\text{C}$ to $+150.0^\circ\text{C}$), and hardware status flags (`DISCONNECTED`, `COMM_TIMEOUT`).
- **Thermal Hysteresis & Safety Latching:** Over-temperature faults ($\ge 80.0^\circ\text{C}$) latch in `STATE_FAULT` until temperature drops below the safe recovery threshold ($< 75.0^\circ\text{C}$), preventing relay or contactor chattering.
- **Deterministic Evaluation:** State transitions execute in strictly $O(1)$ constant time.
- **Standardized Process Exit Codes:**
  - `0`: Successful execution, state is `NORMAL` or `WARNING`.
  - `2`: State is `FAULT` (for automated CI/CD and validation pipelines).
  - `1`: CLI invocation syntax error.

---

## 3. Directory Layout

```text
sensor_fault_controller/
├── .gitignore                   # Excludes build artifacts and temporary files
├── README.md                    # Project documentation and real execution records
├── build.ps1                    # Deterministic PowerShell build runner
├── include/
│   └── sensor_controller.h      # Public API contracts, state enums, and thresholds
├── src/
│   ├── sensor_controller.c      # Core FSM, validation, and hysteresis engine
│   └── main.c                   # CLI application wrapper and exit code handler
├── tests/
│   └── test_sensor_controller.c # 31-case native C unit and boundary test runner
├── docs/
│   ├── requirements.md          # Formal requirements, boundary rules, and test matrix
│   └── architecture.md          # Architectural specification and Mermaid FSM diagrams
└── build/                       # Output directory for compiled binaries (.gitignore'd)
    ├── sensor_controller.exe    # Compiled application executable
    └── test_runner.exe          # Compiled unit test executable
```

---

## 4. Prerequisites & Toolchain

- **Operating System:** Windows 10 / 11 (64-bit)
- **Compiler:** MSYS2 UCRT64 GCC 16.2.0 (`C:\msys64\ucrt64\bin\gcc.exe`)
- **Language Standard:** ISO C11 (`-std=c11`)
- **Compilation Flags:** `-Wall -Wextra -Wpedantic -std=c11 -O2 -Iinclude`
- **Shell:** Windows PowerShell 5.1+

> **Zero System Modification:** Build scripts prepend `C:\msys64\ucrt64\bin` strictly to the local process session. No persistent Windows User or System `PATH` modifications are required.

---

## 5. Build and Execution Instructions

### Building from PowerShell
To compile both the application (`sensor_controller.exe`) and test runner (`test_runner.exe`):
```powershell
.\build.ps1
```

For a clean rebuild:
```powershell
.\build.ps1 -Clean
```

### Running the Native C Unit Test Suite
```powershell
.\build\test_runner.exe
```

#### Actual Test Suite Execution Output (31/31 Passing)
```text
=================================================================
  Embedded Sensor Monitoring & Fault Controller - C Unit Tests   
=================================================================

--- Test Suite 1: Normal Operating Range (< 60°C) ---
  [PASS] TC-01: Room Temperature (25°C) -> STATE_NORMAL
  [PASS] TC-02: Freezing Point (0°C) -> STATE_NORMAL
  [PASS] TC-03: Sub-zero Operating (-20°C) -> STATE_NORMAL
  [PASS] TC-04: Integer Boundary High (59°C) -> STATE_NORMAL
  [PASS] TC-05: Strict Boundary Float (59.9°C) -> STATE_NORMAL

--- Test Suite 2: Warning Range (60°C through 79°C) ---
  [PASS] TC-06: Warning Lower Boundary (60°C) -> STATE_WARNING
  [PASS] TC-07: Warning Mid-range (70°C) -> STATE_WARNING
  [PASS] TC-08: Warning High Integer Boundary (79°C) -> STATE_WARNING
  [PASS] TC-09: Warning High Float Boundary (79.9°C) -> STATE_WARNING

--- Test Suite 3: Fault Over-Temperature (>= 80°C) ---
  [PASS] TC-10: Fault Exact Boundary (80°C) -> STATE_FAULT (FAULT_OVER_TEMP)
  [PASS] TC-11: Extreme Thermal Runaway (120°C) -> STATE_FAULT (FAULT_OVER_TEMP)

--- Test Suite 4: Invalid Sensor Inputs & Fault Handling ---
  [PASS] TC-12: Hardware Disconnection -> STATE_FAULT (FAULT_SENSOR_DISCONNECTED)
  [PASS] TC-13: Bus Timeout / ADC Comm Error -> STATE_FAULT (FAULT_SENSOR_COMM_ERROR)
  [PASS] TC-14: Sensor Under Physical Limit (-45°C) -> STATE_FAULT (FAULT_SENSOR_UNDER_RANGE)
  [PASS] TC-15: Sensor Over Physical Limit (160°C) -> STATE_FAULT (FAULT_SENSOR_OVER_RANGE)
  [PASS] TC-16: Arithmetic NaN Telemetry -> STATE_FAULT (FAULT_SENSOR_INVALID_DATA)
  [PASS] TC-17: NULL Context Defense -> Handled safely without crash
  [PASS] TC-18: NULL Reading Defense -> STATE_FAULT (FAULT_SENSOR_INVALID_DATA)

--- Test Suite 5: State Recovery & Fault Reset ---
  [PASS] TC-19: Initial Over-Temp Trigger -> STATE_FAULT
  [PASS] TC-20: Clear Rejected When T >= 75°C (Hysteresis Check) -> Stays STATE_FAULT
  [PASS] TC-21: Clear Accepted When T < 75°C -> Transitions to STATE_NORMAL

--- Test Suite 6: Fault Priority & Hardware Flags ---
  [PASS] TC-22: Fault Priority: SENSOR_ERR_DISCONNECTED takes precedence over Over-Temp
  [PASS] TC-23: Fault Priority: SENSOR_ERR_COMM_TIMEOUT takes precedence over Over-Temp
  [PASS] TC-24: Hardware SENSOR_ERR_OUT_OF_BOUNDS Low -> FAULT_SENSOR_UNDER_RANGE
  [PASS] TC-25: Hardware SENSOR_ERR_OUT_OF_BOUNDS High -> FAULT_SENSOR_OVER_RANGE

--- Test Suite 7: Continuous Dynamic State Transitions ---
  [PASS] TC-26: Dynamic Seq 1: Initial (25°C) -> STATE_NORMAL
  [PASS] TC-27: Dynamic Seq 2: Rise to Warning (65°C) -> STATE_WARNING
  [PASS] TC-28: Dynamic Seq 3: Cooled (50°C) -> Recovers to STATE_NORMAL
  [PASS] TC-29: Dynamic Seq 4: Thermal Spike (85°C) -> STATE_FAULT
  [PASS] TC-30: Dynamic Seq 5: Partial Cool (76°C) -> Latched in STATE_FAULT
  [PASS] TC-31: Dynamic Seq 6: Safe Cool (45°C) -> Auto-recovers to STATE_NORMAL

=================================================================
  Test Summary: 31 / 31 Passed (100% Pass Rate)
=================================================================
```

---

## 6. Real Example Executions & Outputs

All outputs below are captured directly from actual terminal execution on the compiled binary:

### Nominal Normal Range ($T < 60^\circ\text{C}$)
```powershell
.\build\sensor_controller.exe 25.0
# STATE: NORMAL | TEMP: 25.00 C | FAULT: NONE
# Exit code: 0

.\build\sensor_controller.exe 59.0
# STATE: NORMAL | TEMP: 59.00 C | FAULT: NONE
# Exit code: 0

.\build\sensor_controller.exe 59.99
# STATE: NORMAL | TEMP: 59.99 C | FAULT: NONE
# Exit code: 0
```

### Warning Range ($60^\circ\text{C} \le T \le 79.99^\circ\text{C}$)
```powershell
.\build\sensor_controller.exe 60.0
# STATE: WARNING | TEMP: 60.00 C | FAULT: NONE
# Exit code: 0

.\build\sensor_controller.exe 79.0
# STATE: WARNING | TEMP: 79.00 C | FAULT: NONE
# Exit code: 0

.\build\sensor_controller.exe 79.99
# STATE: WARNING | TEMP: 79.99 C | FAULT: NONE
# Exit code: 0
```

### Thermal Over-Temperature Fault ($T \ge 80^\circ\text{C}$)
```powershell
.\build\sensor_controller.exe 80.0
# STATE: FAULT | TEMP: 80.00 C | FAULT: OVER_TEMP
# Exit code: 2

.\build\sensor_controller.exe 125.0
# STATE: FAULT | TEMP: 125.00 C | FAULT: OVER_TEMP
# Exit code: 2
```

### Physical Range Faults ($T < -40^\circ\text{C}$ or $T > 150^\circ\text{C}$)
```powershell
.\build\sensor_controller.exe -45.0
# STATE: FAULT | TEMP: -45.00 C | FAULT: SENSOR_UNDER_RANGE
# Exit code: 2

.\build\sensor_controller.exe 165.0
# STATE: FAULT | TEMP: 165.00 C | FAULT: SENSOR_OVER_RANGE
# Exit code: 2
```

### Hardware Fault Injections
```powershell
.\build\sensor_controller.exe 25.0 DISCONNECTED
# STATE: FAULT | TEMP: 25.00 C | FAULT: SENSOR_DISCONNECTED
# Exit code: 2

.\build\sensor_controller.exe 25.0 COMM_TIMEOUT
# STATE: FAULT | TEMP: 25.00 C | FAULT: SENSOR_COMM_ERROR
# Exit code: 2
```

### Malformed / Non-Numeric Inputs
```powershell
.\build\sensor_controller.exe 60abc
# STATE: FAULT | TEMP: INVALID | FAULT: SENSOR_INVALID_DATA
# Exit code: 2

.\build\sensor_controller.exe INVALID
# STATE: FAULT | TEMP: INVALID | FAULT: SENSOR_INVALID_DATA
# Exit code: 2

.\build\sensor_controller.exe NAN
# STATE: FAULT | TEMP: INVALID | FAULT: SENSOR_INVALID_DATA
# Exit code: 2
```

---

## 7. Scope & Limitations of Host-Based Simulation

- **Target vs. Host Environment:** This software was compiled and executed under host-based simulation (x86_64 Windows using MSYS2 UCRT64 GCC) to enable automated desktop testing. **No physical microcontroller or hardware sensors were utilized.**
- **Hardware Peripherals:** On a target microcontroller (e.g., STM32, ESP32, NXP), the telemetry inputs would be provided by an ADC driver ISR or an I2C/SPI temperature sensor driver (e.g., TMP117, MAX31865) rather than CLI arguments.
- **Floating Point Considerations:** Target architectures lacking a hardware FPU (e.g., ARM Cortex-M0/M0+) should replace floating-point operations with fixed-point integers (e.g., milli-degrees Celsius scaled as `int32_t`).
