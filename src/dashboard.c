#include "dashboard.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ANSI_RESET "\033[0m"
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_CYAN "\033[36m"
#define ANSI_BOLD "\033[1m"

static bool ansi_supported(void)
{
    const char *term = getenv("TERM");

    return term != NULL && term[0] != '\0' && strcmp(term, "dumb") != 0;
}

static const char *colour(const char *code)
{
    return ansi_supported() ? code : "";
}

static const char *reset_colour(void)
{
    return ansi_supported() ? ANSI_RESET : "";
}

static void print_status(const char *label, bool on, const char *on_colour)
{
    printf("  %-18s %s%-3s%s\n", label, on ? colour(on_colour) : "",
           on ? "ON" : "OFF", on ? reset_colour() : "");
}

void dashboard_init(Dashboard *dashboard)
{
    if (dashboard == NULL) {
        return;
    }

    dashboard->speed_kmh = 0.0;
    dashboard->rpm = 0.0;
    dashboard->fuel_percent = 72.0;
    dashboard->engine_temp_c = 24.0;
    dashboard->battery_voltage = 12.6;
    dashboard->odometer_km = 12345.0;
    dashboard->trip_km = 0.0;
    dashboard->speed_limit_kmh = 80;
    dashboard->gear = GEAR_PARK;
    dashboard->engine_on = false;
    dashboard->moving_in_reverse = false;
    dashboard->headlights_on = false;
    dashboard->left_indicator_on = false;
    dashboard->right_indicator_on = false;
    dashboard->hazards_on = false;
    dashboard->seatbelt_fastened = false;
    dashboard->check_engine_on = false;
}

const char *dashboard_gear_name(Gear gear)
{
    static const char *const names[] = {
        "P", "R", "N", "D", "1", "2", "3", "4", "5", "6"
    };

    if (gear < GEAR_PARK || gear > GEAR_6) {
        return "?";
    }
    return names[gear];
}

bool dashboard_is_low_fuel(const Dashboard *dashboard)
{
    return dashboard != NULL && dashboard->fuel_percent <= 15.0;
}

bool dashboard_is_overheating(const Dashboard *dashboard)
{
    return dashboard != NULL && dashboard->engine_temp_c >= 110.0;
}

bool dashboard_is_speeding(const Dashboard *dashboard)
{
    return dashboard != NULL && dashboard->speed_limit_kmh > 0 &&
           dashboard->speed_kmh > (double)dashboard->speed_limit_kmh;
}

bool dashboard_has_battery_warning(const Dashboard *dashboard)
{
    return dashboard != NULL && dashboard->battery_voltage < 11.8;
}

void dashboard_clear_screen(void)
{
    if (ansi_supported()) {
        printf("\033[2J\033[H");
    } else {
        int line;
        for (line = 0; line < 30; ++line) {
            putchar('\n');
        }
    }
}

void dashboard_display(const Dashboard *dashboard, const char *message)
{
    const char *status;
    const char *direction;

    if (dashboard == NULL) {
        return;
    }

    dashboard_clear_screen();
    status = dashboard->engine_on ? "RUNNING" : "OFF";
    direction = dashboard->moving_in_reverse && dashboard->speed_kmh > 0.05
                    ? " (REVERSE)" : "";

    printf("%s%s+==========================================================+%s\n",
           colour(ANSI_BOLD), colour(ANSI_CYAN), reset_colour());
    printf("%s%s|              VEHICLE DASHBOARD SIMULATOR              |%s\n",
           colour(ANSI_BOLD), colour(ANSI_CYAN), reset_colour());
    printf("%s%s+==========================================================+%s\n",
           colour(ANSI_BOLD), colour(ANSI_CYAN), reset_colour());
    printf("  Engine: %-8s    Gear: %-3s    Seatbelt: %s%-9s%s\n",
           status, dashboard_gear_name(dashboard->gear),
           dashboard->seatbelt_fastened ? colour(ANSI_GREEN) : colour(ANSI_RED),
           dashboard->seatbelt_fastened ? "FASTENED" : "UNFASTENED",
           reset_colour());
    printf("\n");
    printf("  Speed             %7.1f km/h%s\n", dashboard->speed_kmh, direction);
    printf("  Engine speed      %7.0f RPM\n", dashboard->rpm);
    printf("  Fuel level        %7.1f %%\n", dashboard->fuel_percent);
    printf("  Engine temp.      %7.1f C\n", dashboard->engine_temp_c);
    printf("  Battery           %7.2f V\n", dashboard->battery_voltage);
    printf("  Odometer          %7.1f km\n", dashboard->odometer_km);
    printf("  Trip              %7.1f km\n", dashboard->trip_km);
    printf("  Speed limit       %7d km/h\n", dashboard->speed_limit_kmh);
    printf("\n  ELECTRICAL / SIGNALS\n");
    print_status("Headlights", dashboard->headlights_on, ANSI_GREEN);
    print_status("Left indicator", dashboard->left_indicator_on || dashboard->hazards_on,
                 ANSI_YELLOW);
    print_status("Right indicator", dashboard->right_indicator_on || dashboard->hazards_on,
                 ANSI_YELLOW);
    print_status("Hazard lights", dashboard->hazards_on, ANSI_YELLOW);

    printf("\n  %sWARNINGS%s\n", colour(ANSI_BOLD), reset_colour());
    {
        bool warning = false;
        if (dashboard->engine_on && !dashboard->seatbelt_fastened) {
            printf("  %s! SEATBELT NOT FASTENED%s\n", colour(ANSI_RED), reset_colour());
            warning = true;
        }
        if (dashboard->check_engine_on) {
            printf("  %s! CHECK ENGINE%s\n", colour(ANSI_YELLOW), reset_colour());
            warning = true;
        }
        if (dashboard_is_low_fuel(dashboard)) {
            printf("  %s! LOW FUEL%s\n", colour(ANSI_YELLOW), reset_colour());
            warning = true;
        }
        if (dashboard_is_overheating(dashboard)) {
            printf("  %s! ENGINE OVERHEATING%s\n", colour(ANSI_RED), reset_colour());
            warning = true;
        }
        if (dashboard_is_speeding(dashboard)) {
            printf("  %s! SPEED LIMIT EXCEEDED%s\n", colour(ANSI_YELLOW), reset_colour());
            warning = true;
        }
        if (dashboard_has_battery_warning(dashboard)) {
            printf("  %s! LOW BATTERY%s\n", colour(ANSI_RED), reset_colour());
            warning = true;
        }
        if (!warning) {
            printf("  %sAll systems normal%s\n", colour(ANSI_GREEN), reset_colour());
        }
    }

    printf("\n  %s\n", message != NULL && message[0] != '\0' ? message :
           "Choose an action from the menu.");
}
