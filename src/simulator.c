#include "simulator.h"

#include <stdio.h>

#define FULL_TANK_LITRES 50.0
#define MAX_SPEED_KMH 240.0
#define IDLE_RPM 800.0
#define AMBIENT_TEMP_C 24.0

static void set_message(char *message, size_t message_size, const char *text)
{
    if (message != NULL && message_size > 0U) {
        (void)snprintf(message, message_size, "%s", text);
    }
}

static double clamp(double value, double minimum, double maximum)
{
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

static bool is_driving_gear(Gear gear)
{
    return gear == GEAR_REVERSE || gear == GEAR_DRIVE ||
           (gear >= GEAR_1 && gear <= GEAR_6);
}

static double gear_max_speed(Gear gear)
{
    static const double limits[] = {30.0, 55.0, 85.0, 120.0, 165.0, 210.0};

    if (gear >= GEAR_1 && gear <= GEAR_6) {
        return limits[(int)gear - (int)GEAR_1];
    }
    if (gear == GEAR_REVERSE) {
        return 25.0;
    }
    return MAX_SPEED_KMH;
}

static double target_rpm(const Dashboard *dashboard)
{
    double ratio;

    if (!dashboard->engine_on) {
        return 0.0;
    }
    if (dashboard->gear == GEAR_PARK || dashboard->gear == GEAR_NEUTRAL) {
        return IDLE_RPM;
    }
    if (dashboard->gear == GEAR_REVERSE) {
        return 950.0 + dashboard->speed_kmh * 70.0;
    }
    if (dashboard->gear == GEAR_DRIVE) {
        return 950.0 + dashboard->speed_kmh * 35.0;
    }
    ratio = 115.0 - ((double)((int)dashboard->gear - (int)GEAR_1) * 15.0);
    return 900.0 + dashboard->speed_kmh * ratio;
}

bool simulator_start_engine(Dashboard *dashboard, char *message, size_t message_size)
{
    if (dashboard == NULL) {
        set_message(message, message_size, "Internal error: dashboard is unavailable.");
        return false;
    }
    if (dashboard->engine_on) {
        set_message(message, message_size, "Engine is already running.");
        return false;
    }
    if (dashboard->fuel_percent <= 0.0) {
        set_message(message, message_size, "Cannot start: refuel the vehicle first.");
        return false;
    }
    if (dashboard->battery_voltage < 11.2) {
        set_message(message, message_size, "Cannot start: battery voltage is too low.");
        return false;
    }
    if (dashboard->gear != GEAR_PARK && dashboard->gear != GEAR_NEUTRAL) {
        set_message(message, message_size, "Select Park or Neutral before starting the engine.");
        return false;
    }

    dashboard->engine_on = true;
    dashboard->rpm = IDLE_RPM;
    if (dashboard->engine_temp_c < 35.0) {
        dashboard->engine_temp_c = 35.0;
    }
    dashboard->battery_voltage = 14.2;
    set_message(message, message_size, "Engine started. Fasten your seatbelt before driving.");
    return true;
}

bool simulator_stop_engine(Dashboard *dashboard, char *message, size_t message_size)
{
    if (dashboard == NULL) {
        set_message(message, message_size, "Internal error: dashboard is unavailable.");
        return false;
    }
    if (!dashboard->engine_on) {
        set_message(message, message_size, "Engine is already off.");
        return false;
    }
    if (dashboard->speed_kmh > 0.05) {
        set_message(message, message_size, "Brake to a complete stop before turning the engine off.");
        return false;
    }

    dashboard->engine_on = false;
    dashboard->rpm = 0.0;
    dashboard->speed_kmh = 0.0;
    dashboard->moving_in_reverse = false;
    dashboard->battery_voltage = dashboard->headlights_on ? 12.3 : 12.6;
    set_message(message, message_size, "Engine stopped safely.");
    return true;
}

bool simulator_accelerate(Dashboard *dashboard, double amount_kmh,
                          char *message, size_t message_size)
{
    double maximum;

    if (dashboard == NULL || amount_kmh <= 0.0) {
        set_message(message, message_size, "Acceleration must be a positive value.");
        return false;
    }
    if (!dashboard->engine_on) {
        set_message(message, message_size, "Cannot accelerate while the engine is off.");
        return false;
    }
    if (!is_driving_gear(dashboard->gear)) {
        set_message(message, message_size, "Select Drive, Reverse, or a numbered gear before accelerating.");
        return false;
    }
    if (dashboard->fuel_percent <= 0.0) {
        set_message(message, message_size, "Cannot accelerate: fuel tank is empty.");
        return false;
    }

    maximum = gear_max_speed(dashboard->gear);
    dashboard->speed_kmh = clamp(dashboard->speed_kmh + amount_kmh, 0.0, maximum);
    dashboard->moving_in_reverse = dashboard->gear == GEAR_REVERSE;
    simulator_step(dashboard, 1.0);
    if (dashboard->speed_kmh >= maximum) {
        set_message(message, message_size, "Maximum safe speed for the selected gear reached.");
    } else {
        set_message(message, message_size, "Accelerated and advanced the simulation by one second.");
    }
    return true;
}

bool simulator_brake(Dashboard *dashboard, double amount_kmh,
                     char *message, size_t message_size)
{
    if (dashboard == NULL || amount_kmh <= 0.0) {
        set_message(message, message_size, "Brake amount must be a positive value.");
        return false;
    }
    if (dashboard->speed_kmh <= 0.05) {
        dashboard->speed_kmh = 0.0;
        dashboard->moving_in_reverse = false;
        set_message(message, message_size, "Vehicle is already stationary.");
        return false;
    }

    dashboard->speed_kmh = clamp(dashboard->speed_kmh - amount_kmh, 0.0, MAX_SPEED_KMH);
    if (dashboard->speed_kmh <= 0.05) {
        dashboard->speed_kmh = 0.0;
        dashboard->moving_in_reverse = false;
    }
    simulator_step(dashboard, 1.0);
    set_message(message, message_size, "Braked and advanced the simulation by one second.");
    return true;
}

bool simulator_change_gear(Dashboard *dashboard, Gear gear,
                           char *message, size_t message_size)
{
    if (dashboard == NULL || gear < GEAR_PARK || gear > GEAR_6) {
        set_message(message, message_size, "Invalid gear selection.");
        return false;
    }
    if (gear == dashboard->gear) {
        set_message(message, message_size, "That gear is already selected.");
        return false;
    }
    if (dashboard->speed_kmh > 0.05 &&
        (gear == GEAR_PARK || gear == GEAR_REVERSE ||
         (dashboard->moving_in_reverse && gear != GEAR_REVERSE))) {
        set_message(message, message_size,
                    "Come to a complete stop before selecting Park, Reverse, or a forward gear.");
        return false;
    }
    if (gear == GEAR_PARK && dashboard->speed_kmh > 0.05) {
        set_message(message, message_size, "Park can only be selected while stationary.");
        return false;
    }

    dashboard->gear = gear;
    if (gear != GEAR_REVERSE) {
        dashboard->moving_in_reverse = false;
    }
    simulator_step(dashboard, 0.25);
    set_message(message, message_size, "Gear changed successfully.");
    return true;
}

bool simulator_refuel(Dashboard *dashboard, double percent,
                      char *message, size_t message_size)
{
    if (dashboard == NULL || percent <= 0.0) {
        set_message(message, message_size, "Refuel amount must be positive.");
        return false;
    }
    if (dashboard->speed_kmh > 0.05) {
        set_message(message, message_size, "Stop the vehicle before refuelling.");
        return false;
    }

    dashboard->fuel_percent = clamp(dashboard->fuel_percent + percent, 0.0, 100.0);
    set_message(message, message_size, "Fuel level updated.");
    return true;
}

void simulator_reset_trip(Dashboard *dashboard)
{
    if (dashboard != NULL) {
        dashboard->trip_km = 0.0;
    }
}

void simulator_set_speed_limit(Dashboard *dashboard, int limit_kmh)
{
    if (dashboard != NULL && limit_kmh >= 10 && limit_kmh <= 300) {
        dashboard->speed_limit_kmh = limit_kmh;
    }
}

void simulator_step(Dashboard *dashboard, double seconds)
{
    double distance_km;
    double fuel_litres;
    double temperature_target;
    double electrical_load;

    if (dashboard == NULL || seconds <= 0.0) {
        return;
    }

    if (!dashboard->engine_on) {
        electrical_load = dashboard->headlights_on ? 0.004 : 0.0005;
        if (dashboard->left_indicator_on || dashboard->right_indicator_on || dashboard->hazards_on) {
            electrical_load += 0.001;
        }
        dashboard->battery_voltage = clamp(dashboard->battery_voltage - electrical_load * seconds,
                                           10.0, 12.6);
        dashboard->engine_temp_c = clamp(dashboard->engine_temp_c - 0.02 * seconds,
                                         AMBIENT_TEMP_C, 130.0);
        return;
    }

    distance_km = dashboard->speed_kmh * seconds / 3600.0;
    dashboard->odometer_km += distance_km;
    dashboard->trip_km += distance_km;

    fuel_litres = (0.60 * seconds / 3600.0) +
                  (distance_km * (6.5 + dashboard->speed_kmh * 0.02) / 100.0);
    dashboard->fuel_percent = clamp(dashboard->fuel_percent -
                                    (fuel_litres / FULL_TANK_LITRES * 100.0), 0.0, 100.0);
    if (dashboard->fuel_percent <= 0.0) {
        dashboard->speed_kmh = 0.0;
        dashboard->moving_in_reverse = false;
    }

    dashboard->rpm = clamp(target_rpm(dashboard), 0.0, 6500.0);
    temperature_target = 86.0 + (dashboard->rpm - IDLE_RPM) * 0.004 +
                         dashboard->speed_kmh * 0.025;
    dashboard->engine_temp_c += (temperature_target - dashboard->engine_temp_c) *
                                (0.035 * seconds);
    dashboard->engine_temp_c = clamp(dashboard->engine_temp_c, AMBIENT_TEMP_C, 130.0);

    electrical_load = dashboard->headlights_on ? 0.12 : 0.0;
    if (dashboard->left_indicator_on || dashboard->right_indicator_on || dashboard->hazards_on) {
        electrical_load += 0.04;
    }
    dashboard->battery_voltage = clamp(14.2 - electrical_load -
                                       (dashboard->engine_temp_c > 115.0 ? 0.4 : 0.0),
                                       11.5, 14.3);
}

void simulator_auto_drive(Dashboard *dashboard, unsigned int seconds)
{
    unsigned int step;

    if (dashboard == NULL) {
        return;
    }
    for (step = 0U; step < seconds; ++step) {
        if (dashboard->speed_kmh > 0.0 && dashboard->gear != GEAR_DRIVE &&
            !is_driving_gear(dashboard->gear)) {
            dashboard->speed_kmh = clamp(dashboard->speed_kmh - 0.8, 0.0, MAX_SPEED_KMH);
        }
        simulator_step(dashboard, 1.0);
    }
}
