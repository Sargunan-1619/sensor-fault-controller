/**
 * @file sensor_controller.h
 * @brief Public interface and contracts for the Sensor Monitoring & Fault Controller.
 * 
 * Complies with ISO C11. Statically typed, zero dynamic memory allocation.
 */

#ifndef SENSOR_CONTROLLER_H
#define SENSOR_CONTROLLER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* Configuration & Threshold Constants                                        */
/* ========================================================================== */

/** Below 60.0°C: NORMAL operational range */
#define TEMP_WARN_THRESHOLD_C     (60.0f)

/** 80.0°C and above: FAULT condition */
#define TEMP_FAULT_THRESHOLD_C    (80.0f)

/** Hysteresis threshold to clear thermal fault (5°C below fault trigger) */
#define TEMP_FAULT_RECOVERY_C     (75.0f)

/** Physical sensor plausibility boundaries */
#define SENSOR_PHYSICAL_MIN_C     (-40.0f)
#define SENSOR_PHYSICAL_MAX_C     (150.0f)

/* ========================================================================== */
/* Type Definitions & Enums                                                   */
/* ========================================================================== */

/**
 * @brief Hardware-level telemetry health status from sensor / ADC driver.
 */
typedef enum {
    SENSOR_OK = 0,               /**< Sensor hardware and communication healthy */
    SENSOR_ERR_DISCONNECTED,     /**< Hardware open circuit or sensor unplugged */
    SENSOR_ERR_COMM_TIMEOUT,     /**< I2C/SPI bus timeout or ADC conversion fail */
    SENSOR_ERR_OUT_OF_BOUNDS,    /**< Reading outside physical capability (-40..150 C) */
    SENSOR_ERR_INVALID_DATA      /**< Malformed telemetry payload (NaN, Inf) */
} SensorStatus_t;

/**
 * @brief System operational state of the controller.
 */
typedef enum {
    STATE_NORMAL = 0,            /**< Nominal temperature (< 60°C) and healthy sensor */
    STATE_WARNING,               /**< Elevated temperature (60°C..79.9°C), healthy sensor */
    STATE_FAULT                  /**< Critical temperature (>= 80°C) OR sensor fault */
} SystemState_t;

/**
 * @brief Specific fault identification code when in STATE_FAULT.
 */
typedef enum {
    FAULT_NONE = 0,              /**< No fault present */
    FAULT_OVER_TEMP,             /**< Temperature >= 80.0°C */
    FAULT_SENSOR_UNDER_RANGE,    /**< Temperature reading < -40.0°C */
    FAULT_SENSOR_OVER_RANGE,     /**< Temperature reading > 150.0°C */
    FAULT_SENSOR_DISCONNECTED,   /**< Sensor disconnected signal received */
    FAULT_SENSOR_COMM_ERROR,     /**< Communication or ADC conversion failure */
    FAULT_SENSOR_INVALID_DATA    /**< NaN or Inf floating point value received */
} FaultCode_t;

/**
 * @brief Input telemetry sample presented to the controller.
 */
typedef struct {
    float temperature_c;         /**< Converted temperature in degrees Celsius */
    SensorStatus_t status;       /**< Hardware health status */
} SensorReading_t;

/**
 * @brief Controller runtime context representing internal state and telemetry.
 */
typedef struct {
    SystemState_t current_state; /**< Active operational state */
    FaultCode_t active_fault;    /**< Active fault code (if current_state == STATE_FAULT) */
    float last_valid_temp_c;     /**< Cached last-known valid temperature reading */
    uint32_t sample_count;       /**< Total telemetry samples processed */
    uint32_t fault_count;        /**< Cumulative count of fault occurrences */
} ControllerContext_t;

/* ========================================================================== */
/* Function Prototypes                                                        */
/* ========================================================================== */

/**
 * @brief Initialize the controller context to deterministic initial state.
 * @param ctx Pointer to controller context (must not be NULL).
 */
void sensor_controller_init(ControllerContext_t *ctx);

/**
 * @brief Process an incoming sensor reading and update state machine deterministically.
 * @param ctx Pointer to controller context (must not be NULL).
 * @param reading Pointer to incoming sensor telemetry (must not be NULL).
 */
void sensor_controller_process(ControllerContext_t *ctx, const SensorReading_t *reading);

/**
 * @brief Retrieve current operational state.
 * @param ctx Pointer to controller context.
 * @return Active SystemState_t (returns STATE_FAULT if ctx is NULL).
 */
SystemState_t sensor_controller_get_state(const ControllerContext_t *ctx);

/**
 * @brief Retrieve active fault code.
 * @param ctx Pointer to controller context.
 * @return Active FaultCode_t (returns FAULT_SENSOR_INVALID_DATA if ctx is NULL).
 */
FaultCode_t sensor_controller_get_fault(const ControllerContext_t *ctx);

/**
 * @brief Attempt to clear a latched fault state.
 * @details Can only be cleared if the latest temperature is below recovery threshold (75°C).
 * @param ctx Pointer to controller context.
 * @return true if fault was successfully cleared to STATE_NORMAL, false otherwise.
 */
bool sensor_controller_clear_fault(ControllerContext_t *ctx);

/**
 * @brief Convert SystemState_t enum value to a human-readable constant string.
 */
const char* system_state_to_string(SystemState_t state);

/**
 * @brief Convert FaultCode_t enum value to a human-readable constant string.
 */
const char* fault_code_to_string(FaultCode_t fault);

/**
 * @brief Convert SensorStatus_t enum value to a human-readable constant string.
 */
const char* sensor_status_to_string(SensorStatus_t status);

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_CONTROLLER_H */
