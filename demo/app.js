/**
 * @file app.js
 * @brief WebAssembly controller driver and interactive telemetry synchronizer.
 * Executes the real C11 sensor controller logic client-side via WebAssembly.
 */

// Embedded Base64 fallback ensures execution in all browser contexts
const EMBEDDED_WASM_BASE64 = "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c3RhdHVzAHNlbnNvcl9jb250cm9sbGVyX3Byb2Nlc3MAd2FzbV9jb250cm9sbGVyX3Byb2Nlc3MAc3RhdHVzX2ZsYWdzAGNoYXIAQzovVXNlcnMvY3NhcmcvT25lRHJpdmUvRGVza3RvcC9zZW5zb3ItZmF1bHQtY29udHJvbGxlci1kZW1vAF9Cb29sAHdhc21fZ2V0X2ZhdWx0X3N0cmluZwB3YXNtX2dldF9zdGF0dXNfc3RyaW5nAHNlbnNvcl9zdGF0dXNfdG9fc3RyaW5nAHN5c3RlbV9zdGF0ZV90b19zdHJpbmcAZmF1bHRfY29kZV90b19zdHJpbmcAd2FzbV9nZXRfc3RhdGVfc3RyaW5nAHJlYWRpbmcAd2FzbV9nZXRfY29udGV4dF9zaXplAGN1cnJlbnRfc3RhdGUAc2Vuc29yX2NvbnRyb2xsZXJfZ2V0X3N0YXRlAHdhc21fZ2V0X3N0YXRlAHdhc21fZ2V0X3RlbXBlcmF0dXJlAGxhc3RfdmFsaWRfdGVtcF9jAHRlbXBlcmF0dXJlX2MAY19zcmMvc2Vuc29yX2NvbnRyb2xsZXIuYwB3YXNtL3dhc21fYnJpZGdlLmMAX19BUlJBWV9TSVpFX1RZUEVfXwBTRU5TT1JfRVJSX0NPTU1fVElNRU9VVABTVEFURV9GQVVMVABTRU5TT1JfRVJSX09VVF9PRl9CT1VORFMARkFVTFRfU0VOU09SX0NPTU1fRVJST1IARkFVTFRfT1ZFUl9URU1QAFNUQVRFX05PUk1BTABTRU5TT1JfT0sAU1RBVEVfV0FSTklORwBGQVVMVF9OT05FAEZBVUxUX1NFTlNPUl9PVkVSX1JBTkdFAEZBVUxUX1NFTlNPUl9VTkRFUl9SQU5HRQBTRU5TT1JfRVJSX0RJU0NPTk5FQ1RFRABGQVVMVF9TRU5TT1JfRElTQ09OTkVDVEVEAFNFTlNPUl9FUlJfSU5WQUxJRF9EQVRBAEZBVUxUX1NFTlNPUl9JTlZBTElEX0RBVEEAY2xhbmcgdmVyc2lvbiAxOC4xLjYgKGh0dHBzOi8vZ2l0aHViLmNvbS96aWdsYW5nL3ppZy1ib290c3RyYXAgOThiYzZiZjRmYzQwMDk4ODhkMzM5NDFkYWY2YjYwMGQyMGE0MmE1NikAAN8ICy5kZWJ1Z19saW5lTgAAAAQASAAAAAEBAfsODQABAQEBAAAAAQAAAWNfc3JjAABzZW5zb3JfY29udHJvbGxlci5jAAEAAHNlbnNvcl9jb250cm9sbGVyLmgAAQAAAP0DAAAEAMkAAAABAQH7Dg0AAQEBAQAAAAEAAAFjX3NyYwBDOlxVc2Vyc1xjc2FyZ1wuZ2VtaW5pXGFudGlncmF2aXR5LWlkZVxzY3JhdGNoXHppZ190b29sc1x6aWctd2luZG93cy14ODZfNjQtMC4xMy4wXGxpYlxpbmNsdWRlAHdhc20AAHNlbnNvcl9jb250cm9sbGVyLmgAAQAAc3RkaW50LmgAAgAAd2FzbV9icmlkZ2UuYwADAABzZW5zb3JfY29udHJvbGxlci5jAAEAAAAEBAUYCgAFAgMAAAADFQEFHIQFGHIFF0sFGHMFF+cFGHEFFk4EAwUBA3Z0AgEAAQEEBAUWCgAFAkIAAAADIgEGA10ISgUJBgMx1gYDT4IFGwYDM1gFHKsFGa4EAwUBA2UIEgYDZyAEBAUbBgM6WAUcqwUZrgQDBQEDXggSBgNnIAQEBRwGA8AAWAU1CD0FHQYgBRsgBRkGrwQDBQEDVQgSBgNnIAQEBRsGA8oAWAUcqwUZrgQDBQEDTggSBgNnIAQEBScGA9AAIAUcyQUbSwUccwUZ5gQDBQEDRggSBgNnIAQEBSAGA9gAngUJBiAFHAY9BRtLBRxzBRnmBAMFAQO+fwgSBgNnIAQEBSAGA98AngUJBiAFHAY9BRtLBRxzBRnmBAMFAQO3fwgSBgNnIAQEBRwGA+cAIAUgAwkIPAUJBiADkH88BRsGA/MASgUcqwUZrgQDBQEDpX8IEgYDZyAEBAUwBgP2AKwGA4p/PAP2AKwFHAY+BRtLBRxzBAMFAQOhf6wGA2cgBAQFJQYD+wCeBQ4GIAUcBj4FG+UEAwUBA5t/dAYDZyAEBAUcBgOCASAFG0sFHHMEAwUBA5d/rAIBAAEBBAQFDgoABQKVAgAAA50BAQUgBgggBQkgA+J+WAUkBgOfAZAFAAYD4X4gBRsGA6QBrAYD3H50BAMFBQYDHiACAwABAQQEBREKAAUCzwIAAAOLAQEEAwUFA5h/kAIBAAEBBAQFEQoABQLbAgAAA5MBAQQDBQUDln+QAgEAAQEEAwUdCgAFAucCAAADLwEFBQaQAgEAAQEEAwUdCgAFAvMCAAADNQEFBQaQAgEAAQEEAwUdCgAFAv8CAAADOwEFBQaQAgEAAQEEAwAFAgoDAAADwAABBQUKPQIBAAEBBAMABQIPAwAAA8YAAQQEBQUKA+YACBIGA9N+PAOtAUoD037IBAMGA8gAIAIDAAEBBAMABQI5AwAAA8wAAQQEBQUKA+oACBIGA8l+PAO3AUoDyX7IBAMGA84AIAIDAAEBBAMABQJjAwAAA9IAAQQEBQUKA/IACBIGA7t+PAPFAUoDu37IBAMGA9QAIAIDAAEBAM4CBG5hbWUAFxZzZW5zb3JfY29udHJvbGxlci53YXNtAY0CDAAUd2FzbV9jb250cm9sbGVyX2luaXQBF3dhc21fY29udHJvbGxlcl9wcm9jZXNzAht3YXNtX2NvbnRyb2xsZXJfY2xlYXJfZmF1bHQDDndhc21fZ2V0X3N0YXRlBA53YXNtX2dldF9mYXVsdAUUd2FzbV9nZXRfdGVtcGVyYXR1cmUGFXdhc21fZ2V0X3NhbXBsZV9jb3VudAcUd2FzbV9nZXRfZmF1bHRfY291bnQIFXdhc21fZ2V0X2NvbnRleHRfc2l6ZQkVd2FzbV9nZXRfc3RhdGVfc3RyaW5nChV3YXNtX2dldF9mYXVsdF9zdHJpbmcLFndhc21fZ2V0X3N0YXR1c19zdHJpbmcHEgEAD19fc3RhY2tfcG9pbnRlcgkKAQAHLnJvZGF0YQCJAQlwcm9kdWNlcnMCCGxhbmd1YWdlAQNDMTEADHByb2Nlc3NlZC1ieQEFY2xhbmdaMTguMS42IChodHRwczovL2dpdGh1Yi5jb20vemlnbGFuZy96aWctYm9vdHN0cmFwIDk4YmM2YmY0ZmM0MDA5ODg4ZDMzOTQxZGFmNmI2MDBkMjBhNDJhNTYpACwPdGFyZ2V0X2ZlYXR1cmVzAisPbXV0YWJsZS1nbG9iYWxzKwhzaWduLWV4dA==";

const STATE_NAMES = ["STATE_NORMAL", "STATE_WARNING", "STATE_FAULT"];
const FAULT_NAMES = [
  "FAULT_NONE",
  "FAULT_OVER_TEMP",
  "FAULT_SENSOR_UNDER_RANGE",
  "FAULT_SENSOR_OVER_RANGE",
  "FAULT_SENSOR_DISCONNECTED",
  "FAULT_SENSOR_COMM_ERROR",
  "FAULT_SENSOR_INVALID_DATA"
];
const STATUS_NAMES = ["OK", "DISCONNECTED", "COMM_TIMEOUT", "OUT_OF_BOUNDS", "INVALID_DATA"];

class ControllerApp {
  constructor() {
    this.wasm = null;
    this.currentTemp = 25.0;
    this.currentStatus = 0;
    this.autoStreamTimer = null;
    this.logHistory = [];
    this.startTime = Date.now();
  }

  async init() {
    try {
      await this.loadWasmModule();
      this.bindDomEvents();
      this.wasm.wasm_controller_init();
      this.updateSimulation();
      this.log("[WASM ENGINE]", "Compiled C11 controller active in WebAssembly sandbox.", "success");
    } catch (err) {
      console.error("Initialization error:", err);
      this.log("[ERROR]", "Failed to initialize WebAssembly engine: " + err.message, "danger");
    }
  }

  async loadWasmModule() {
    let wasmBytes;
    try {
      const response = await fetch("sensor_controller.wasm");
      if (!response.ok) throw new Error("HTTP " + response.status);
      wasmBytes = await response.arrayBuffer();
    } catch (fetchErr) {
      console.warn("Direct fetch failed, falling back to embedded Base64 payload:", fetchErr);
      const binaryString = atob(EMBEDDED_WASM_BASE64);
      wasmBytes = new Uint8Array(binaryString.length);
      for (let i = 0; i < binaryString.length; i++) {
        wasmBytes[i] = binaryString.charCodeAt(i);
      }
    }

    const wasmModule = await WebAssembly.instantiate(wasmBytes, {});
    this.wasm = wasmModule.instance.exports;
  }

  bindDomEvents() {
    const tempSlider = document.getElementById("tempSlider");
    const tempInput = document.getElementById("tempInput");

    tempSlider.addEventListener("input", (e) => {
      this.currentTemp = parseFloat(e.target.value);
      tempInput.value = this.currentTemp.toFixed(1);
      this.updateSimulation();
    });

    tempInput.addEventListener("change", (e) => {
      let val = parseFloat(e.target.value);
      if (isNaN(val)) val = 25.0;
      val = Math.max(-45, Math.min(165, val));
      this.currentTemp = val;
      tempSlider.value = val;
      tempInput.value = val.toFixed(1);
      this.updateSimulation();
    });

    // Preset buttons
    document.querySelectorAll(".btn-preset").forEach((btn) => {
      btn.addEventListener("click", () => {
        this.currentTemp = parseFloat(btn.dataset.temp);
        this.currentStatus = parseInt(btn.dataset.status, 10);
        tempSlider.value = this.currentTemp;
        tempInput.value = this.currentTemp.toFixed(1);
        this.setActiveErrorButton("btnErrNone");
        this.updateSimulation();
      });
    });

    // Error injection buttons
    document.querySelectorAll(".btn-err").forEach((btn) => {
      btn.addEventListener("click", () => {
        const statusVal = parseInt(btn.dataset.status, 10);
        this.currentStatus = statusVal;
        this.setActiveErrorButton(btn.id);
        if (statusVal === 4) {
          this.currentTemp = NaN;
        } else if (statusVal === 3) {
          this.currentTemp = -45.0;
        }
        this.updateSimulation();
      });
    });

    // Clear Fault / Reset
    document.getElementById("btnResetFault").addEventListener("click", () => {
      const cleared = this.wasm.wasm_controller_clear_fault();
      if (cleared) {
        this.currentStatus = 0;
        this.setActiveErrorButton("btnErrNone");
        this.log("[RESET ACTION]", "Fault successfully cleared. Transitioned to NORMAL state.", "success");
      } else {
        this.log("[RESET REJECTED]", "Cannot clear fault: temperature must be below 75.0°C recovery threshold.", "danger");
      }
      this.updateSimulation(false);
    });

    // Re-initialize
    document.getElementById("btnReinit").addEventListener("click", () => {
      this.wasm.wasm_controller_init();
      this.currentTemp = 25.0;
      this.currentStatus = 0;
      tempSlider.value = 25.0;
      tempInput.value = "25.0";
      this.setActiveErrorButton("btnErrNone");
      this.log("[RE-INIT]", "Controller memory context reset to initial state.", "dim");
      this.updateSimulation();
    });

    // Auto-stream toggle
    document.getElementById("autoStreamToggle").addEventListener("change", (e) => {
      if (e.target.checked) {
        this.startAutoStream();
      } else {
        this.stopAutoStream();
      }
    });
  }

  setActiveErrorButton(activeId) {
    document.querySelectorAll(".btn-err").forEach((btn) => {
      if (btn.id === activeId) {
        btn.classList.add("active");
      } else {
        btn.classList.remove("active");
      }
    });
  }

  startAutoStream() {
    if (this.autoStreamTimer) return;
    let step = 0;
    this.autoStreamTimer = setInterval(() => {
      step++;
      // Sine wave simulated thermal cycle: 20°C to 88°C
      const simTemp = 50 + 35 * Math.sin(step * 0.1);
      this.currentTemp = Math.round(simTemp * 10) / 10;
      document.getElementById("tempSlider").value = this.currentTemp;
      document.getElementById("tempInput").value = this.currentTemp.toFixed(1);
      this.updateSimulation(true);
    }, 100); // 10 Hz
    this.log("[STREAM]", "10 Hz simulated thermal generator active.", "dim");
  }

  stopAutoStream() {
    if (this.autoStreamTimer) {
      clearInterval(this.autoStreamTimer);
      this.autoStreamTimer = null;
      this.log("[STREAM]", "10 Hz simulated telemetry stopped.", "dim");
    }
  }

  updateSimulation(processReading = true) {
    if (processReading) {
      this.wasm.wasm_controller_process(this.currentTemp, this.currentStatus);
    }

    const state = this.wasm.wasm_get_state();
    const fault = this.wasm.wasm_get_fault();
    const lastTemp = this.wasm.wasm_get_temperature();
    const sampleCount = this.wasm.wasm_get_sample_count();
    const faultCount = this.wasm.wasm_get_fault_count();

    // 1. Update Gauge
    const dispVal = isNaN(this.currentTemp) ? "NaN" : this.currentTemp.toFixed(1);
    document.getElementById("dispTemp").textContent = dispVal;

    // Gauge range: -40 to 150 = 190 deg span
    const normalizedPercent = Math.max(0, Math.min(100, ((this.currentTemp + 40) / 190) * 100));
    const gaugeFill = document.getElementById("gaugeFill");
    gaugeFill.style.width = normalizedPercent + "%";
    gaugeFill.className = "gauge-bar-fill";
    if (state === 1) gaugeFill.classList.add("warning");
    if (state === 2) gaugeFill.classList.add("fault");

    // 2. Update FSM Nodes
    document.getElementById("nodeNormal").classList.toggle("active", state === 0);
    document.getElementById("nodeWarning").classList.toggle("active", state === 1);
    document.getElementById("nodeFault").classList.toggle("active", state === 2);

    // 3. Update Relay Status
    const relayEl = document.getElementById("relayStatus");
    if (state === 2) {
      relayEl.textContent = "RELAY: TRIPPED";
      relayEl.className = "relay-indicator tripped";
    } else {
      relayEl.textContent = "RELAY: ARMED";
      relayEl.className = "relay-indicator";
    }

    // 4. Update Metrics
    const metricState = document.getElementById("metricState");
    metricState.textContent = STATE_NAMES[state] || "UNKNOWN";
    metricState.className = "metric-value " + (state === 0 ? "text-normal" : state === 1 ? "text-warning" : "text-fault");

    const metricFault = document.getElementById("metricFault");
    metricFault.textContent = FAULT_NAMES[fault] || "UNKNOWN";
    metricFault.className = "metric-value " + (fault === 0 ? "" : "text-fault");

    document.getElementById("metricStatus").textContent = "SENSOR_" + (STATUS_NAMES[this.currentStatus] || "UNKNOWN");
    document.getElementById("metricFaultCount").textContent = faultCount;
    document.getElementById("evalCounter").textContent = "Samples: " + sampleCount;

    // 5. Update Hysteresis Callout
    const hysBadge = document.getElementById("hysBadge");
    const hysText = document.getElementById("hysText");
    if (state === 2) {
      hysBadge.textContent = "SAFETY LATCH ACTIVE";
      hysBadge.className = "hysteresis-badge latched";
      if (this.currentTemp >= 75.0) {
        hysText.textContent = "System latched in FAULT. Temperature (" + dispVal + "°C) is at or above 75.0°C recovery threshold. Fault clear attempts will be rejected.";
      } else {
        hysText.textContent = "Temperature (" + dispVal + "°C) is below 75.0°C threshold. Safe to clear fault via 'Clear Fault / Reset Controller'.";
      }
    } else {
      hysBadge.textContent = "LATCH DISARMED";
      hysBadge.className = "hysteresis-badge";
      hysText.textContent = "System operating in nominal range. Over-temperature faults (≥ 80°C) will latch safely and refuse reset until temperature drops below 75.0°C recovery threshold.";
    }

    // 6. Log entry
    const relStr = state === 2 ? "TRIPPED" : "ARMED";
    const logMsg = `STATE: ${STATE_NAMES[state].replace("STATE_", "")} | TEMP: ${dispVal} C | FAULT: ${FAULT_NAMES[fault].replace("FAULT_", "")} | RELAY: ${relStr}`;
    this.log("[C11 EVAL]", logMsg, state === 2 ? "danger" : state === 1 ? "dim" : "success");
  }

  log(tag, msg, type = "") {
    const term = document.getElementById("terminalLog");
    const elapsed = ((Date.now() - this.startTime) / 1000).toFixed(2);
    const line = document.createElement("div");
    line.className = "log-line " + (type ? "text-" + type : "");
    line.textContent = `[+${elapsed}s] ${tag} ${msg}`;
    term.appendChild(line);

    // Keep max 60 lines
    while (term.children.length > 60) {
      term.removeChild(term.firstChild);
    }
    term.scrollTop = term.scrollHeight;
  }
}

document.addEventListener("DOMContentLoaded", () => {
  const app = new ControllerApp();
  app.init();
});
