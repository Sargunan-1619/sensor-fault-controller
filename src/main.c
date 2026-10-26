/**
 * @file main.c
 * @brief Standalone CLI application wrapper for Sensor Monitoring & Fault Controller.
 * 
 * Provides an executable interface for manual verification and automated 
 * testing frameworks (such as Project 2).
 * 
 * Exit Codes:
 *   0: Successful evaluation, system in NORMAL or WARNING state.
 *   2: Critical evaluation, system in FAULT state.
 *   1: Command-line syntax error (missing required parameters).
 */

#include "sensor_controller.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

static void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s <temperature_c|INVALID> [STATUS]\n", prog_name);
    fprintf(stderr, "Available STATUS options: OK, DISCONNECTED, COMM_TIMEOUT, OUT_OF_BOUNDS, INVALID\n");
    fprintf(stderr, "Examples:\n");
    fprintf(stderr, "  %s 25.5\n", prog_name);
    fprintf(stderr, "  %s 60.0\n", prog_name);
    fprintf(stderr, "  %s 80.0\n", prog_name);
    fprintf(stderr, "  %s 25.0 DISCONNECTED\n", prog_name);
    fprintf(stderr, "  %s INVALID\n", prog_name);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    SensorReading_t reading;
    reading.temperature_c = 0.0f;
    reading.status = SENSOR_OK;

    bool is_numeric = true;
    char *endptr = NULL;

    /* Check if user explicitly passed a non-numeric/invalid token */
    if (strcasecmp(argv[1], "INVALID") == 0 ||
        strcasecmp(argv[1], "NAN") == 0 ||
        strcasecmp(argv[1], "INF") == 0 ||
        strcasecmp(argv[1], "-INF") == 0 ||
        strcasecmp(argv[1], "+INF") == 0 ||
        strcasecmp(argv[1], "ERROR") == 0) {
        reading.status = SENSOR_ERR_INVALID_DATA;
        is_numeric = false;
    } else {
        /* Parse numerical float value */
        reading.temperature_c = (float)strtod(argv[1], &endptr);
        if (endptr == argv[1] || *endptr != '\0') {
            /* Input string was not a valid numerical representation */
            reading.status = SENSOR_ERR_INVALID_DATA;
            is_numeric = false;
        }
    }

    /* Optional second argument: Hardware status flag */
    if (argc >= 3) {
        if (strcasecmp(argv[2], "DISCONNECTED") == 0) {
            reading.status = SENSOR_ERR_DISCONNECTED;
        } else if (strcasecmp(argv[2], "COMM_TIMEOUT") == 0) {
            reading.status = SENSOR_ERR_COMM_TIMEOUT;
        } else if (strcasecmp(argv[2], "OUT_OF_BOUNDS") == 0) {
            reading.status = SENSOR_ERR_OUT_OF_BOUNDS;
        } else if (strcasecmp(argv[2], "INVALID") == 0) {
            reading.status = SENSOR_ERR_INVALID_DATA;
        } else if (strcasecmp(argv[2], "OK") == 0) {
            if (is_numeric) {
                reading.status = SENSOR_OK;
            }
        }
    }

    /* Initialize controller context */
    ControllerContext_t ctx;
    sensor_controller_init(&ctx);

    /* Process sensor reading through the deterministic state machine */
    sensor_controller_process(&ctx, &reading);

    SystemState_t state = sensor_controller_get_state(&ctx);
    FaultCode_t fault = sensor_controller_get_fault(&ctx);

    /* Print structured telemetry for human review and automated parsers */
    if (is_numeric && !isnan(reading.temperature_c) && !isinf(reading.temperature_c)) {
        printf("STATE: %s | TEMP: %.2f C | FAULT: %s\n",
               system_state_to_string(state),
               reading.temperature_c,
               fault_code_to_string(fault));
    } else {
        printf("STATE: %s | TEMP: INVALID | FAULT: %s\n",
               system_state_to_string(state),
               fault_code_to_string(fault));
    }

    /* Exit code contract:
     * 0: Nominal / Warning operational states
     * 2: Safety Fault state
     */
    if (state == STATE_FAULT) {
        return 2;
    }

    return 0;
}
