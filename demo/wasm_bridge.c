/**
 * @file wasm_bridge.c
 * @brief WebAssembly entry points and exports for the C11 Sensor Controller.
 */

#include "sensor_controller.h"

#define WASM_EXPORT __attribute__((visibility("default")))

static ControllerContext_t g_controller_ctx;

WASM_EXPORT
void wasm_controller_init(void)
{
    sensor_controller_init(&g_controller_ctx);
}

WASM_EXPORT
void wasm_controller_process(float temperature_c, uint32_t status_flags)
{
    SensorReading_t reading;
    reading.temperature_c = temperature_c;
    reading.status = (SensorStatus_t)status_flags;
    sensor_controller_process(&g_controller_ctx, &reading);
}

WASM_EXPORT
int32_t wasm_controller_clear_fault(void)
{
    return (int32_t)sensor_controller_clear_fault(&g_controller_ctx);
}

WASM_EXPORT
int32_t wasm_get_state(void)
{
    return (int32_t)sensor_controller_get_state(&g_controller_ctx);
}

WASM_EXPORT
int32_t wasm_get_fault(void)
{
    return (int32_t)sensor_controller_get_fault(&g_controller_ctx);
}

WASM_EXPORT
float wasm_get_temperature(void)
{
    return g_controller_ctx.last_valid_temp_c;
}

WASM_EXPORT
uint32_t wasm_get_sample_count(void)
{
    return g_controller_ctx.sample_count;
}

WASM_EXPORT
uint32_t wasm_get_fault_count(void)
{
    return g_controller_ctx.fault_count;
}

WASM_EXPORT
uint32_t wasm_get_context_size(void)
{
    return (uint32_t)sizeof(ControllerContext_t);
}

WASM_EXPORT
const char* wasm_get_state_string(int32_t state)
{
    return system_state_to_string((SystemState_t)state);
}

WASM_EXPORT
const char* wasm_get_fault_string(int32_t fault)
{
    return fault_code_to_string((FaultCode_t)fault);
}

WASM_EXPORT
const char* wasm_get_status_string(int32_t status)
{
    return sensor_status_to_string((SensorStatus_t)status);
}
