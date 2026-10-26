/**
 * @file test_sensor_controller.c
 * @brief Native C Unit and Boundary Test Runner for Sensor Fault Controller.
 * 
 * Verifies all boundary conditions, invalid inputs, and state transitions
 * without external dependencies.
 */

#include "sensor_controller.h"
#include <stdio.h>
#include <math.h>

static int g_tests_run = 0;
static int g_tests_passed = 0;

#define ASSERT_TEST(cond, name) do { \
    g_tests_run++; \
    if (cond) { \
        g_tests_passed++; \
        printf("  [PASS] %s\n", (name)); \
    } else { \
        printf("  [FAIL] %s (Line %d)\n", (name), __LINE__); \
    } \
} while(0)

/* ========================================================================== */
/* Test Cases                                                                 */
/* ========================================================================== */

static void test_normal_operating_temperatures(void)
{
    printf("\n--- Test Suite 1: Normal Operating Range (< 60°C) ---\n");
    ControllerContext_t ctx;
    sensor_controller_init(&ctx);

    SensorReading_t r1 = { .temperature_c = 25.0f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r1);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_NORMAL &&
                sensor_controller_get_fault(&ctx) == FAULT_NONE,
                "TC-01: Room Temperature (25°C) -> STATE_NORMAL");

    SensorReading_t r2 = { .temperature_c = 0.0f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r2);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_NORMAL &&
                sensor_controller_get_fault(&ctx) == FAULT_NONE,
                "TC-02: Freezing Point (0°C) -> STATE_NORMAL");

    SensorReading_t r3 = { .temperature_c = -20.0f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r3);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_NORMAL &&
                sensor_controller_get_fault(&ctx) == FAULT_NONE,
                "TC-03: Sub-zero Operating (-20°C) -> STATE_NORMAL");

    SensorReading_t r4 = { .temperature_c = 59.0f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r4);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_NORMAL &&
                sensor_controller_get_fault(&ctx) == FAULT_NONE,
                "TC-04: Integer Boundary High (59°C) -> STATE_NORMAL");

    SensorReading_t r5 = { .temperature_c = 59.9f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r5);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_NORMAL &&
                sensor_controller_get_fault(&ctx) == FAULT_NONE,
                "TC-05: Strict Boundary Float (59.9°C) -> STATE_NORMAL");
}

static void test_warning_boundary_and_range(void)
{
    printf("\n--- Test Suite 2: Warning Range (60°C through 79°C) ---\n");
    ControllerContext_t ctx;
    sensor_controller_init(&ctx);

    SensorReading_t r1 = { .temperature_c = 60.0f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r1);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_WARNING &&
                sensor_controller_get_fault(&ctx) == FAULT_NONE,
                "TC-06: Warning Lower Boundary (60°C) -> STATE_WARNING");

    SensorReading_t r2 = { .temperature_c = 70.0f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r2);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_WARNING &&
                sensor_controller_get_fault(&ctx) == FAULT_NONE,
                "TC-07: Warning Mid-range (70°C) -> STATE_WARNING");

    SensorReading_t r3 = { .temperature_c = 79.0f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r3);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_WARNING &&
                sensor_controller_get_fault(&ctx) == FAULT_NONE,
                "TC-08: Warning High Integer Boundary (79°C) -> STATE_WARNING");

    SensorReading_t r4 = { .temperature_c = 79.9f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r4);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_WARNING &&
                sensor_controller_get_fault(&ctx) == FAULT_NONE,
                "TC-09: Warning High Float Boundary (79.9°C) -> STATE_WARNING");
}

static void test_fault_thermal_boundaries(void)
{
    printf("\n--- Test Suite 3: Fault Over-Temperature (>= 80°C) ---\n");
    ControllerContext_t ctx;
    sensor_controller_init(&ctx);

    SensorReading_t r1 = { .temperature_c = 80.0f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r1);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_FAULT &&
                sensor_controller_get_fault(&ctx) == FAULT_OVER_TEMP,
                "TC-10: Fault Exact Boundary (80°C) -> STATE_FAULT (FAULT_OVER_TEMP)");

    SensorReading_t r2 = { .temperature_c = 120.0f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r2);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_FAULT &&
                sensor_controller_get_fault(&ctx) == FAULT_OVER_TEMP,
                "TC-11: Extreme Thermal Runaway (120°C) -> STATE_FAULT (FAULT_OVER_TEMP)");
}

static void test_invalid_sensor_inputs(void)
{
    printf("\n--- Test Suite 4: Invalid Sensor Inputs & Fault Handling ---\n");
    ControllerContext_t ctx;
    sensor_controller_init(&ctx);

    /* Disconnected */
    SensorReading_t r_disc = { .temperature_c = 25.0f, .status = SENSOR_ERR_DISCONNECTED };
    sensor_controller_process(&ctx, &r_disc);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_FAULT &&
                sensor_controller_get_fault(&ctx) == FAULT_SENSOR_DISCONNECTED,
                "TC-12: Hardware Disconnection -> STATE_FAULT (FAULT_SENSOR_DISCONNECTED)");

    /* Comm error */
    SensorReading_t r_comm = { .temperature_c = 25.0f, .status = SENSOR_ERR_COMM_TIMEOUT };
    sensor_controller_process(&ctx, &r_comm);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_FAULT &&
                sensor_controller_get_fault(&ctx) == FAULT_SENSOR_COMM_ERROR,
                "TC-13: Bus Timeout / ADC Comm Error -> STATE_FAULT (FAULT_SENSOR_COMM_ERROR)");

    /* Physical under-range (< -40°C) */
    SensorReading_t r_under = { .temperature_c = -45.0f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r_under);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_FAULT &&
                sensor_controller_get_fault(&ctx) == FAULT_SENSOR_UNDER_RANGE,
                "TC-14: Sensor Under Physical Limit (-45°C) -> STATE_FAULT (FAULT_SENSOR_UNDER_RANGE)");

    /* Physical over-range (> 150°C) */
    SensorReading_t r_over = { .temperature_c = 160.0f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r_over);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_FAULT &&
                sensor_controller_get_fault(&ctx) == FAULT_SENSOR_OVER_RANGE,
                "TC-15: Sensor Over Physical Limit (160°C) -> STATE_FAULT (FAULT_SENSOR_OVER_RANGE)");

    /* NaN Float */
    SensorReading_t r_nan = { .temperature_c = (float)NAN, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r_nan);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_FAULT &&
                sensor_controller_get_fault(&ctx) == FAULT_SENSOR_INVALID_DATA,
                "TC-16: Arithmetic NaN Telemetry -> STATE_FAULT (FAULT_SENSOR_INVALID_DATA)");

    /* NULL Context & Reading Defensive Checks */
    sensor_controller_process(NULL, &r_disc);
    ASSERT_TEST(1, "TC-17: NULL Context Defense -> Handled safely without crash");

    sensor_controller_process(&ctx, NULL);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_FAULT &&
                sensor_controller_get_fault(&ctx) == FAULT_SENSOR_INVALID_DATA,
                "TC-18: NULL Reading Defense -> STATE_FAULT (FAULT_SENSOR_INVALID_DATA)");
}

static void test_fault_clearing_and_hysteresis(void)
{
    printf("\n--- Test Suite 5: State Recovery & Fault Reset ---\n");
    ControllerContext_t ctx;
    sensor_controller_init(&ctx);

    /* Trigger thermal fault (85°C) */
    SensorReading_t r_fault = { .temperature_c = 85.0f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r_fault);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_FAULT,
                "TC-19: Initial Over-Temp Trigger -> STATE_FAULT");

    /* Attempt clear while still too hot (78°C) */
    SensorReading_t r_hot = { .temperature_c = 78.0f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r_hot);
    bool cleared_hot = sensor_controller_clear_fault(&ctx);
    ASSERT_TEST(!cleared_hot && sensor_controller_get_state(&ctx) == STATE_FAULT,
                "TC-20: Clear Rejected When T >= 75°C (Hysteresis Check) -> Stays STATE_FAULT");

    /* Clear when cooled down (50°C) */
    SensorReading_t r_cool = { .temperature_c = 50.0f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r_cool);
    bool cleared_cool = sensor_controller_clear_fault(&ctx);
    ASSERT_TEST(cleared_cool && sensor_controller_get_state(&ctx) == STATE_NORMAL,
                "TC-21: Clear Accepted When T < 75°C -> Transitions to STATE_NORMAL");
}

static void test_fault_priority_and_hardware_flags(void)
{
    printf("\n--- Test Suite 6: Fault Priority & Hardware Flags ---\n");
    ControllerContext_t ctx;
    sensor_controller_init(&ctx);

    /* Disconnected status takes priority even if temperature is above 80°C */
    SensorReading_t r_disc_hot = { .temperature_c = 95.0f, .status = SENSOR_ERR_DISCONNECTED };
    sensor_controller_process(&ctx, &r_disc_hot);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_FAULT &&
                sensor_controller_get_fault(&ctx) == FAULT_SENSOR_DISCONNECTED,
                "TC-22: Fault Priority: SENSOR_ERR_DISCONNECTED takes precedence over Over-Temp");

    /* Comm timeout takes priority even if temperature is above 80°C */
    SensorReading_t r_comm_hot = { .temperature_c = 85.0f, .status = SENSOR_ERR_COMM_TIMEOUT };
    sensor_controller_process(&ctx, &r_comm_hot);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_FAULT &&
                sensor_controller_get_fault(&ctx) == FAULT_SENSOR_COMM_ERROR,
                "TC-23: Fault Priority: SENSOR_ERR_COMM_TIMEOUT takes precedence over Over-Temp");

    /* Out of bounds flag with low temp */
    SensorReading_t r_oob_low = { .temperature_c = -42.0f, .status = SENSOR_ERR_OUT_OF_BOUNDS };
    sensor_controller_process(&ctx, &r_oob_low);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_FAULT &&
                sensor_controller_get_fault(&ctx) == FAULT_SENSOR_UNDER_RANGE,
                "TC-24: Hardware SENSOR_ERR_OUT_OF_BOUNDS Low -> FAULT_SENSOR_UNDER_RANGE");

    /* Out of bounds flag with high temp */
    SensorReading_t r_oob_high = { .temperature_c = 155.0f, .status = SENSOR_ERR_OUT_OF_BOUNDS };
    sensor_controller_process(&ctx, &r_oob_high);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_FAULT &&
                sensor_controller_get_fault(&ctx) == FAULT_SENSOR_OVER_RANGE,
                "TC-25: Hardware SENSOR_ERR_OUT_OF_BOUNDS High -> FAULT_SENSOR_OVER_RANGE");
}

static void test_dynamic_state_transitions(void)
{
    printf("\n--- Test Suite 7: Continuous Dynamic State Transitions ---\n");
    ControllerContext_t ctx;
    sensor_controller_init(&ctx);

    /* Step 1: Nominal start (25°C) -> NORMAL */
    SensorReading_t r1 = { .temperature_c = 25.0f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r1);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_NORMAL,
                "TC-26: Dynamic Seq 1: Initial (25°C) -> STATE_NORMAL");

    /* Step 2: Rises to 65°C -> WARNING */
    SensorReading_t r2 = { .temperature_c = 65.0f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r2);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_WARNING,
                "TC-27: Dynamic Seq 2: Rise to Warning (65°C) -> STATE_WARNING");

    /* Step 3: Cools down to 50°C -> Back to NORMAL */
    SensorReading_t r3 = { .temperature_c = 50.0f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r3);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_NORMAL,
                "TC-28: Dynamic Seq 3: Cooled (50°C) -> Recovers to STATE_NORMAL");

    /* Step 4: Spikes to 85°C -> FAULT */
    SensorReading_t r4 = { .temperature_c = 85.0f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r4);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_FAULT,
                "TC-29: Dynamic Seq 4: Thermal Spike (85°C) -> STATE_FAULT");

    /* Step 5: Drops to 76°C (above 75°C recovery) -> Remains FAULT (Latching) */
    SensorReading_t r5 = { .temperature_c = 76.0f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r5);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_FAULT,
                "TC-30: Dynamic Seq 5: Partial Cool (76°C) -> Latched in STATE_FAULT");

    /* Step 6: Drops to 45°C -> Clears to NORMAL */
    SensorReading_t r6 = { .temperature_c = 45.0f, .status = SENSOR_OK };
    sensor_controller_process(&ctx, &r6);
    ASSERT_TEST(sensor_controller_get_state(&ctx) == STATE_NORMAL,
                "TC-31: Dynamic Seq 6: Safe Cool (45°C) -> Auto-recovers to STATE_NORMAL");
}

int main(void)
{
    printf("=================================================================\n");
    printf("  Embedded Sensor Monitoring & Fault Controller - C Unit Tests   \n");
    printf("=================================================================\n");

    test_normal_operating_temperatures();
    test_warning_boundary_and_range();
    test_fault_thermal_boundaries();
    test_invalid_sensor_inputs();
    test_fault_clearing_and_hysteresis();
    test_fault_priority_and_hardware_flags();
    test_dynamic_state_transitions();

    printf("\n=================================================================\n");
    printf("  Test Summary: %d / %d Passed\n", g_tests_passed, g_tests_run);
    printf("=================================================================\n");

    return (g_tests_passed == g_tests_run) ? 0 : 1;
}
