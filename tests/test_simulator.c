#include "dashboard.h"
#include "simulator.h"
#include "storage.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static void test_engine_and_motion_rules(void)
{
    Dashboard dashboard;
    char message[128];

    dashboard_init(&dashboard);
    assert(!simulator_accelerate(&dashboard, 10.0, message, sizeof message));
    assert(simulator_start_engine(&dashboard, message, sizeof message));
    assert(simulator_change_gear(&dashboard, GEAR_DRIVE, message, sizeof message));
    assert(simulator_accelerate(&dashboard, 30.0, message, sizeof message));
    assert(dashboard.speed_kmh > 0.0);
    assert(dashboard.rpm > 800.0);
    assert(!simulator_change_gear(&dashboard, GEAR_REVERSE, message, sizeof message));
    assert(simulator_brake(&dashboard, 100.0, message, sizeof message));
    assert(fabs(dashboard.speed_kmh) < 0.001);
    assert(simulator_change_gear(&dashboard, GEAR_REVERSE, message, sizeof message));
}

static void test_step_updates_vehicle_values(void)
{
    Dashboard dashboard;
    char message[128];
    double fuel_before;
    double odometer_before;

    dashboard_init(&dashboard);
    assert(simulator_start_engine(&dashboard, message, sizeof message));
    assert(simulator_change_gear(&dashboard, GEAR_DRIVE, message, sizeof message));
    assert(simulator_accelerate(&dashboard, 60.0, message, sizeof message));
    fuel_before = dashboard.fuel_percent;
    odometer_before = dashboard.odometer_km;
    simulator_step(&dashboard, 120.0);
    assert(dashboard.odometer_km > odometer_before);
    assert(dashboard.trip_km > 0.0);
    assert(dashboard.fuel_percent < fuel_before);
    assert(dashboard.battery_voltage >= 11.5 && dashboard.battery_voltage <= 14.3);
}

static void test_storage_round_trip(void)
{
    const char *path = "build/test_dashboard_state.txt";
    Dashboard saved;
    Dashboard loaded;
    char message[128];

    dashboard_init(&saved);
    saved.engine_on = true;
    saved.gear = GEAR_3;
    saved.speed_kmh = 42.5;
    saved.fuel_percent = 38.0;
    saved.headlights_on = true;
    saved.seatbelt_fastened = true;
    assert(storage_save(&saved, path, message, sizeof message));
    dashboard_init(&loaded);
    assert(storage_load(&loaded, path, message, sizeof message));
    assert(fabs(loaded.speed_kmh - 42.5) < 0.001);
    assert(loaded.gear == GEAR_3);
    assert(loaded.headlights_on);
    assert(loaded.seatbelt_fastened);
}

int main(void)
{
    test_engine_and_motion_rules();
    test_step_updates_vehicle_values();
    test_storage_round_trip();
    puts("All simulator tests passed.");
    return 0;
}
