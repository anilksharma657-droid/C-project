#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <stdbool.h>

typedef enum {
    GEAR_PARK = 0,
    GEAR_REVERSE,
    GEAR_NEUTRAL,
    GEAR_DRIVE,
    GEAR_1,
    GEAR_2,
    GEAR_3,
    GEAR_4,
    GEAR_5,
    GEAR_6
} Gear;

typedef struct {
    double speed_kmh;
    double rpm;
    double fuel_percent;
    double engine_temp_c;
    double battery_voltage;
    double odometer_km;
    double trip_km;
    int speed_limit_kmh;
    Gear gear;
    bool engine_on;
    bool moving_in_reverse;
    bool headlights_on;
    bool left_indicator_on;
    bool right_indicator_on;
    bool hazards_on;
    bool seatbelt_fastened;
    bool check_engine_on;
} Dashboard;

void dashboard_init(Dashboard *dashboard);
const char *dashboard_gear_name(Gear gear);
bool dashboard_is_low_fuel(const Dashboard *dashboard);
bool dashboard_is_overheating(const Dashboard *dashboard);
bool dashboard_is_speeding(const Dashboard *dashboard);
bool dashboard_has_battery_warning(const Dashboard *dashboard);
void dashboard_clear_screen(void);
void dashboard_display(const Dashboard *dashboard, const char *message);

#endif
