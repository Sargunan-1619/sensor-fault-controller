/**
 * @file sensor_controller.c
 * @brief Implementation of the Sensor Monitoring & Fault Controller logic.
 * 
 * Embedded Design Principles Applied:
 * - Zero dynamic memory allocation (deterministic memory footprint).
 * - Zero standard I/O (pure logic testable anywhere).
 * - Full input validation (defensive against NaN, Inf, and out-of-bounds telemetry).
 * - Hysteresis and fault latching for safety-critical thermal monitoring.
 * - MISRA-C / ISO C11 compliant style.
 */

#include "sensor_controller.h"
#include <math.h>

void sensor_controller_init(ControllerContext_t *ctx)
{
    if (ctx == NULL) {
        return;
    }

    ctx->current_state = STATE_NORMAL;
    ctx->active_fault = FAULT_NONE;
    ctx->last_valid_temp_c = 25.0f;
    ctx->sample_count = 0U;
    ctx->fault_count = 0U;
}

void sensor_controller_process(ControllerContext_t *ctx, const SensorReading_t *reading)
{
    if (ctx == NULL) {
        return;
    }

    ctx->sample_count++;

    /* ---------------------------------------------------------------------- */
    /* 1. Defensive Validation: Hardware Status & Data Integrity Checks        */
    /* ---------------------------------------------------------------------- */

    if (reading == NULL) {
        ctx->current_state = STATE_FAULT;
        ctx->active_fault = FAULT_SENSOR_INVALID_DATA;
        ctx->fault_count++;
        return;
    }

    /* Check hardware status flags from telemetry */
    if (reading->status == SENSOR_ERR_DISCONNECTED) {
        ctx->current_state = STATE_FAULT;
        ctx->active_fault = FAULT_SENSOR_DISCONNECTED;
        ctx->fault_count++;
        return;
    }

    if (reading->status == SENSOR_ERR_COMM_TIMEOUT) {
        ctx->current_state = STATE_FAULT;
        ctx->active_fault = FAULT_SENSOR_COMM_ERROR;
        ctx->fault_count++;
        return;
    }

    if (reading->status == SENSOR_ERR_OUT_OF_BOUNDS) {
        ctx->current_state = STATE_FAULT;
        ctx->active_fault = (reading->temperature_c < SENSOR_PHYSICAL_MIN_C)
                            ? FAULT_SENSOR_UNDER_RANGE
                            : FAULT_SENSOR_OVER_RANGE;
        ctx->fault_count++;
        return;
    }

    if (reading->status == SENSOR_ERR_INVALID_DATA) {
        ctx->current_state = STATE_FAULT;
        ctx->active_fault = FAULT_SENSOR_INVALID_DATA;
        ctx->fault_count++;
        return;
    }

    /* Check floating-point arithmetic validity (detect NaN or Infinity) */
    if (isnan(reading->temperature_c) || isinf(reading->temperature_c)) {
        ctx->current_state = STATE_FAULT;
        ctx->active_fault = FAULT_SENSOR_INVALID_DATA;
        ctx->fault_count++;
        return;
    }

    /* Check physical transducer plausibility limits (-40°C to +150°C) */
    if (reading->temperature_c < SENSOR_PHYSICAL_MIN_C) {
        ctx->current_state = STATE_FAULT;
        ctx->active_fault = FAULT_SENSOR_UNDER_RANGE;
        ctx->fault_count++;
        return;
    }

    if (reading->temperature_c > SENSOR_PHYSICAL_MAX_C) {
        ctx->current_state = STATE_FAULT;
        ctx->active_fault = FAULT_SENSOR_OVER_RANGE;
        ctx->fault_count++;
        return;
    }

    /* Reading passed all data-integrity checks; cache valid reading */
    ctx->last_valid_temp_c = reading->temperature_c;

    /* ---------------------------------------------------------------------- */
    /* 2. State Evaluation Logic based on Operational Thresholds & Hysteresis */
    /*    - Below 60°C: NORMAL                                                */
    /*    - 60°C to 79.9°C: WARNING                                           */
    /*    - 80°C and above: FAULT                                             */
    /* ---------------------------------------------------------------------- */

    if (reading->temperature_c >= TEMP_FAULT_THRESHOLD_C) {
        /* Critical over-temperature condition (>= 80.0°C) */
        ctx->current_state = STATE_FAULT;
        ctx->active_fault = FAULT_OVER_TEMP;
        ctx->fault_count++;
    } 
    else if (ctx->current_state == STATE_FAULT && reading->temperature_c >= TEMP_FAULT_RECOVERY_C) {
        /* Safety Latch: System was in FAULT and has not cooled below recovery threshold (< 75°C) */
        ctx->current_state = STATE_FAULT;
        ctx->active_fault = FAULT_OVER_TEMP;
    }
    else if (reading->temperature_c >= TEMP_WARN_THRESHOLD_C) {
        /* Elevated warning condition (60.0°C .. 79.9°C) */
        ctx->current_state = STATE_WARNING;
        ctx->active_fault = FAULT_NONE;
    } 
    else {
        /* Nominal operating condition (< 60.0°C) */
        ctx->current_state = STATE_NORMAL;
        ctx->active_fault = FAULT_NONE;
    }
}

SystemState_t sensor_controller_get_state(const ControllerContext_t *ctx)
{
    if (ctx == NULL) {
        return STATE_FAULT;
    }
    return ctx->current_state;
}

FaultCode_t sensor_controller_get_fault(const ControllerContext_t *ctx)
{
    if (ctx == NULL) {
        return FAULT_SENSOR_INVALID_DATA;
    }
    return ctx->active_fault;
}

bool sensor_controller_clear_fault(ControllerContext_t *ctx)
{
    if (ctx == NULL) {
        return false;
    }

    /* Can only safely clear if temperature is below safe recovery threshold */
    if (ctx->last_valid_temp_c < TEMP_FAULT_RECOVERY_C) {
        if (ctx->last_valid_temp_c >= TEMP_WARN_THRESHOLD_C) {
            ctx->current_state = STATE_WARNING;
        } else {
            ctx->current_state = STATE_NORMAL;
        }
        ctx->active_fault = FAULT_NONE;
        return true;
    }

    return false;
}

const char* system_state_to_string(SystemState_t state)
{
    switch (state) {
        case STATE_NORMAL:  return "NORMAL";
        case STATE_WARNING: return "WARNING";
        case STATE_FAULT:   return "FAULT";
        default:            return "UNKNOWN";
    }
}

const char* fault_code_to_string(FaultCode_t fault)
{
    switch (fault) {
        case FAULT_NONE:                  return "NONE";
        case FAULT_OVER_TEMP:             return "OVER_TEMP";
        case FAULT_SENSOR_UNDER_RANGE:    return "SENSOR_UNDER_RANGE";
        case FAULT_SENSOR_OVER_RANGE:     return "SENSOR_OVER_RANGE";
        case FAULT_SENSOR_DISCONNECTED:   return "SENSOR_DISCONNECTED";
        case FAULT_SENSOR_COMM_ERROR:     return "SENSOR_COMM_ERROR";
        case FAULT_SENSOR_INVALID_DATA:   return "SENSOR_INVALID_DATA";
        default:                          return "UNKNOWN_FAULT";
    }
}

const char* sensor_status_to_string(SensorStatus_t status)
{
    switch (status) {
        case SENSOR_OK:                   return "OK";
        case SENSOR_ERR_DISCONNECTED:     return "DISCONNECTED";
        case SENSOR_ERR_COMM_TIMEOUT:     return "COMM_TIMEOUT";
        case SENSOR_ERR_OUT_OF_BOUNDS:    return "OUT_OF_BOUNDS";
        case SENSOR_ERR_INVALID_DATA:     return "INVALID_DATA";
        default:                          return "UNKNOWN_STATUS";
    }
}
