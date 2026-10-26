# C11 Sensor Fault Controller — WebAssembly Interactive Demo

[![C Standard](https://img.shields.io/badge/C%20Standard-ISO%20C11-00599C?logo=c)](../include/sensor_controller.h)
[![WebAssembly Core](https://img.shields.io/badge/WebAssembly-7.4%20KB%20Freestanding-654FF0?logo=webassembly)](sensor_controller.wasm)
[![RAM Footprint](https://img.shields.io/badge/RAM%20Footprint-20%20Bytes%20(0B%20Heap)-success)](../src/sensor_controller.c)
[![Unit Test Status](https://img.shields.io/badge/Wasm%20Verification-31%20Passed%20%2F%200%20Failed-brightgreen)](verify_wasm.js)
[![Vercel Ready](https://img.shields.io/badge/Deployment-Vercel%20Static-000000?logo=vercel)](vercel.json)
[![License](https://img.shields.io/badge/License-MIT-green)](../LICENSE)

An interactive, recruiter-ready engineering demonstration of the **[C11 Sensor Fault Controller](https://github.com/Sargunan-1619/sensor-fault-controller)**. Built as a client-side application compiled directly to freestanding WebAssembly, suitable for zero-cost deployment on Vercel's global edge network.

---

## 📌 Project Overview & Core Philosophy

This demo showcases the execution of safety-critical embedded C11 firmware directly inside the web browser sandbox via WebAssembly.

> [!IMPORTANT]
> **Zero JavaScript Business Logic Duplication:** The web interface does **not** reimplement the finite state machine, temperature threshold evaluation, or safety latching in JavaScript. The actual controller logic in `src/sensor_controller.c` is compiled directly to a 7.4 KB freestanding WebAssembly binary (`demo/sensor_controller.wasm`). The browser acts solely as a telemetry driver and visualizer across the Wasm memory boundary.

### Key Verified Features
- **Deterministic Finite State Machine (FSM):** Real-time visualization of transitions between `STATE_NORMAL` ($T < 60^\circ\text{C}$), `STATE_WARNING` ($60^\circ\text{C} \le T < 80^\circ\text{C}$), and `STATE_FAULT` ($T \ge 80^\circ\text{C}$ / hardware error flags).
- **Thermal Hysteresis & Safety Latching:** Demonstrates safety-critical latching where over-temperature trips ($\ge 80^\circ\text{C}$) refuse to reset until the simulated temperature cools strictly below the $75.0^\circ\text{C}$ recovery threshold.
- **Hardware Fault Injections:** Interactive simulation of sensor disconnection (`SENSOR_ERR_DISCONNECTED`), bus communication timeout (`SENSOR_ERR_COMM_TIMEOUT`), physical transducer limit breaches ($-45^\circ\text{C}$ and $160^\circ\text{C}$), and arithmetic `NaN` telemetry.
- **10 Hz Real-Time Telemetry Streamer:** Simulated continuous sinusoidal thermal loop for observing active state evaluation and fault recovery dynamics.
- **Embedded RAM & Diagnostics Inspector:** Live monitoring of struct memory footprint (`sizeof(ControllerContext_t) = 20 B`, 0 bytes heap), monotonic sample counters, and cumulative fault counters.

---

## ⚠️ Scope & Hardware Simulation Disclosure

- **Software Simulation Only:** All temperature values, communication bus timeouts, and disconnection flags evaluated by this interface are **software-simulated inputs**.
- **No Physical Hardware:** Testing and demonstration are conducted in host desktop and browser sandbox environments; **no physical microcontroller, ADC, or temperature transducer hardware was utilized.**
- **Embedded Portability:** On a target microcontroller (e.g., STM32, ESP32, NXP), the telemetry inputs would be provided by an ADC driver ISR or an I2C/SPI digital sensor driver (e.g., TMP117, MAX31865) rather than JavaScript sliders.

---

## 🏗️ WebAssembly Architecture & Freestanding Compilation

```
+-----------------------------------------------------------------------------------------+
|                                    Client Web Browser                                   |
|                                                                                         |
|   +---------------------------------------------------------------------------------+   |
|   |                        User Interface (HTML5 / CSS3 / app.js)                   |   |
|   |  - Real-Time Thermal Gauge                      - 10 Hz Auto-Stream Simulator   |   |
|   |  - Glowing FSM State Nodes                      - Interactive Preset Controls   |   |
|   +---------------------------------------------------------------------------------+   |
|                                   |                               ^                     |
|                   (Simulated Telemetry: float, uint32)            | (State & Fault Enums)
|                                   v                               |                     |
|   +---------------------------------------------------------------------------------+   |
|   |                    WebAssembly Sandbox (sensor_controller.wasm)                  |   |
|   |                                                                                 |   |
|   |   +-----------------------------+               +----------------------------+  |   |
|   |   |     demo/wasm_bridge.c      | ------------> |   src/sensor_controller.c  |  |   |
|   |   |  (Exported C ABI Functions) |               |     (Original ISO C11)     |  |   |
|   |   +-----------------------------+               +----------------------------+  |   |
|   |                  |                                             |                |   |
|   |                  +----------------------+----------------------+                |   |
|   |                                         v                                       |   |
|   |                        +----------------------------------+                     |   |
|   |                        |  Static Context: 20 Bytes RAM    |                     |   |
|   |                        |  Heap Allocation: 0 Bytes        |                     |   |
|   |                        +----------------------------------+                     |   |
|   |   +-----------------------------------------------------------------------------+   |
+-----------------------------------------------------------------------------------------+
```

### Why Freestanding Wasm?
- **No OS / WASI Imports Required:** Compiled with `-target wasm32-freestanding -nostdlib -O3`, producing a self-contained binary executable in any modern browser without external runtime polyfills.
- **Tiny Memory Footprint:** Exactly 20 bytes of static linear memory are used for the `ControllerContext_t` struct.
- **Instant Global Edge Deployment:** Served statically from Vercel's global CDN with zero serverless cold starts.

---

## 📁 Demo Directory Structure

```text
sensor-fault-controller/demo/
├── index.html                 # Semantic HTML5 industrial engineering dashboard
├── style.css                  # Modern dark-mode industrial CSS styles
├── app.js                     # Wasm loader and interactive telemetry driver
├── sensor_controller.wasm     # Compiled freestanding WebAssembly binary (7.4 KB)
├── wasm_bridge.c              # Exported WebAssembly ABI bridge functions
├── math.h                     # Freestanding math header (mapping to compiler builtins)
├── verify_wasm.js             # Automated Node.js test runner (31 C scenarios)
├── build_wasm.ps1             # Automated PowerShell script for Wasm compilation
├── vercel.json                # Vercel configuration for demo/ as project root
├── favicon.svg                # Microcontroller SVG icon
└── README.md                  # Demo-specific documentation
```

---

## 🧪 Verified Test Evidence (31 / 31 Passing)

All 31 original unit test scenarios from Project 1's native test runner (`tests/test_sensor_controller.c`) are verified against the compiled WebAssembly binary using `tests/verify_wasm.js`:

```text
======================================================================
 WebAssembly Unit Test Runner (31 Original C Test Scenarios)
 Target Binary: public/sensor_controller.wasm (7335 bytes)
======================================================================
Context RAM Size: 20 bytes (Zero Dynamic Allocation verified)

--- Test Suite 1: Normal Operating Range (< 60°C) ---
  [PASS] TC-01: Room Temperature (25°C)
  [PASS] TC-02: Freezing Point (0°C)
  [PASS] TC-03: Sub-zero Operating (-20°C)
  [PASS] TC-04: Integer Boundary High (59°C)
  [PASS] TC-05: Strict Boundary Float (59.9°C)

--- Test Suite 2: Warning Range (60°C through 79°C) ---
  [PASS] TC-06: Warning Lower Boundary (60°C)
  [PASS] TC-07: Warning Mid-range (70°C)
  [PASS] TC-08: Warning High Integer Boundary (79°C)
  [PASS] TC-09: Warning High Float Boundary (79.9°C)

--- Test Suite 3: Fault Over-Temperature (>= 80°C) ---
  [PASS] TC-10: Fault Exact Boundary (80°C)
  [PASS] TC-11: Extreme Thermal Runaway (120°C)

--- Test Suite 4: Invalid Sensor Inputs & Fault Handling ---
  [PASS] TC-12: Hardware Disconnection
  [PASS] TC-13: Bus Timeout / ADC Comm Error
  [PASS] TC-14: Sensor Under Physical Limit (-45°C)
  [PASS] TC-15: Sensor Over Physical Limit (160°C)
  [PASS] TC-16: Arithmetic NaN Telemetry
  [PASS] TC-17: Context Struct Size Defense (20 Bytes)
  [PASS] TC-18: Explicit Invalid Telemetry Flag

--- Test Suite 5: State Recovery & Fault Reset ---
  [PASS] TC-19: Initial Over-Temp Trigger (85°C)
  [PASS] TC-20: Clear Rejected When T >= 75°C (Hysteresis Check)
  [PASS] TC-21: Clear Accepted When T < 75°C

--- Test Suite 6: Fault Priority & Hardware Flags ---
  [PASS] TC-22: Fault Priority: DISCONNECTED over Over-Temp
  [PASS] TC-23: Fault Priority: COMM_TIMEOUT over Over-Temp
  [PASS] TC-24: Hardware OUT_OF_BOUNDS Low -> UNDER_RANGE
  [PASS] TC-25: Hardware OUT_OF_BOUNDS High -> OVER_RANGE

--- Test Suite 7: Continuous Dynamic State Transitions ---
  [PASS] TC-26: Dynamic Seq 1: Initial (25°C) -> NORMAL
  [PASS] TC-27: Dynamic Seq 2: Rise to Warning (65°C) -> WARNING
  [PASS] TC-28: Dynamic Seq 3: Cooled (50°C) -> NORMAL
  [PASS] TC-29: Dynamic Seq 4: Thermal Spike (85°C) -> FAULT
  [PASS] TC-30: Dynamic Seq 5: Partial Cool (76°C) -> Latched FAULT
  [PASS] TC-31: Dynamic Seq 6: Safe Cool (45°C) -> Auto-recovers to NORMAL
======================================================================
 Test Summary: 31 / 31 Passed (100% Pass Rate)
======================================================================
```

---

## 🚀 Getting Started & Local Usage

### Prerequisites
- Modern Web Browser (Chrome, Edge, Firefox, Safari) with WebAssembly support
- Python 3.x (for running a local HTTP server)
- Node.js (for running the headless verification suite)
- *(Optional for recompiling Wasm)*: Zig 0.13.0 or LLVM/Clang with wasm32 target

### 1. Launching the Web Demo Locally
```powershell
# From the repository root, serve the demo/ directory:
python -m http.server 8090 --directory demo
```
Then open in your browser:
👉 **`http://127.0.0.1:8090`**

### 2. Running the WebAssembly Verification Suite
```bash
# From repository root:
node demo/verify_wasm.js
```

### 3. Recompiling WebAssembly from C Source
To recompile `src/sensor_controller.c` and `demo/wasm_bridge.c` into `demo/sensor_controller.wasm`:
```powershell
# From demo directory:
cd demo
powershell -ExecutionPolicy Bypass -File build_wasm.ps1
```

---

## ☁️ Deploying to Vercel (Step-by-Step)

This repository supports two seamless Vercel deployment configurations:

### Option A: Root Repository Deployment (Automatic)
The root `vercel.json` designates `"outputDirectory": "demo"`.
1. In Vercel, click **Add New Project** and import **`sensor-fault-controller`**.
2. Keep the default settings:
   - **Framework Preset:** `Other`
   - **Root Directory:** `./`
3. Click **Deploy**. Vercel will automatically serve `demo/` as the site root and apply Wasm MIME headers.

### Option B: Dedicated Demo Root Directory
Alternatively, you can configure Vercel to point directly to `demo`:
1. In Vercel, import **`sensor-fault-controller`**.
2. Under **Project Settings** > **Root Directory**, click **Edit** and enter `demo`.
3. In `demo/`, the included `demo/vercel.json` provides the required `application/wasm` headers.
4. Click **Deploy**.

---

## 📄 Related Repositories

- **Project 1 — C11 Sensor Fault Controller:** [https://github.com/Sargunan-1619/sensor-fault-controller](https://github.com/Sargunan-1619/sensor-fault-controller)
- **Project 2 — Python Firmware Validation Framework:** [https://github.com/Sargunan-1619/firmware-validation-framework](https://github.com/Sargunan-1619/firmware-validation-framework)

---

## 📄 License

MIT License. Designed and maintained for robust embedded firmware verification.
