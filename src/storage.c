#include "storage.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void set_message(char *message, size_t message_size, const char *text)
{
    if (message != NULL && message_size > 0U) {
        (void)snprintf(message, message_size, "%s", text);
    }
}

static bool parse_double(const char *text, double *value)
{
    char *end;
    double parsed;

    errno = 0;
    parsed = strtod(text, &end);
    if (errno != 0 || end == text || *end != '\0' || !isfinite(parsed)) {
        return false;
    }
    *value = parsed;
    return true;
}

static bool parse_int(const char *text, int *value)
{
    char *end;
    long parsed;

    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        parsed < -2147483647L || parsed > 2147483647L) {
        return false;
    }
    *value = (int)parsed;
    return true;
}

static void trim_newline(char *text)
{
    size_t length = strlen(text);

    while (length > 0U && (text[length - 1U] == '\n' || text[length - 1U] == '\r')) {
        text[--length] = '\0';
    }
}

bool storage_save(const Dashboard *dashboard, const char *path,
                  char *message, size_t message_size)
{
    FILE *file;
    int write_failed;

    if (dashboard == NULL || path == NULL) {
        set_message(message, message_size, "Cannot save: invalid dashboard or path.");
        return false;
    }
    file = fopen(path, "w");
    if (file == NULL) {
        set_message(message, message_size, "Could not open the state file for writing.");
        return false;
    }

    write_failed = fprintf(file,
                           "speed=%.10g\nrpm=%.10g\nfuel=%.10g\ntemp=%.10g\nbattery=%.10g\n"
                           "odometer=%.10g\ntrip=%.10g\nspeed_limit=%d\ngear=%d\nengine=%d\n"
                           "reverse=%d\nheadlights=%d\nleft_indicator=%d\nright_indicator=%d\n"
                           "hazards=%d\nseatbelt=%d\ncheck_engine=%d\n",
                           dashboard->speed_kmh, dashboard->rpm, dashboard->fuel_percent,
                           dashboard->engine_temp_c, dashboard->battery_voltage,
                           dashboard->odometer_km, dashboard->trip_km, dashboard->speed_limit_kmh,
                           (int)dashboard->gear, dashboard->engine_on ? 1 : 0,
                           dashboard->moving_in_reverse ? 1 : 0, dashboard->headlights_on ? 1 : 0,
                           dashboard->left_indicator_on ? 1 : 0,
                           dashboard->right_indicator_on ? 1 : 0,
                           dashboard->hazards_on ? 1 : 0, dashboard->seatbelt_fastened ? 1 : 0,
                           dashboard->check_engine_on ? 1 : 0) < 0;
    if (fclose(file) != 0 || write_failed != 0) {
        set_message(message, message_size, "Failed while writing the state file.");
        return false;
    }
    set_message(message, message_size, "Dashboard state saved to dashboard_state.txt.");
    return true;
}

bool storage_load(Dashboard *dashboard, const char *path,
                  char *message, size_t message_size)
{
    Dashboard loaded;
    FILE *file;
    char line[128];
    int values_read = 0;
    int read_failed;

    if (dashboard == NULL || path == NULL) {
        set_message(message, message_size, "Cannot load: invalid dashboard or path.");
        return false;
    }
    file = fopen(path, "r");
    if (file == NULL) {
        set_message(message, message_size, "Could not open dashboard_state.txt. Save a state first.");
        return false;
    }

    dashboard_init(&loaded);
    while (fgets(line, sizeof line, file) != NULL) {
        char *equals;
        char *key;
        char *value;
        double decimal;
        int integer;

        trim_newline(line);
        equals = strchr(line, '=');
        if (equals == NULL || equals == line || equals[1] == '\0') {
            continue;
        }
        *equals = '\0';
        key = line;
        value = equals + 1;

        if (strcmp(key, "speed") == 0 && parse_double(value, &decimal)) {
            loaded.speed_kmh = decimal; ++values_read;
        } else if (strcmp(key, "rpm") == 0 && parse_double(value, &decimal)) {
            loaded.rpm = decimal; ++values_read;
        } else if (strcmp(key, "fuel") == 0 && parse_double(value, &decimal)) {
            loaded.fuel_percent = decimal; ++values_read;
        } else if (strcmp(key, "temp") == 0 && parse_double(value, &decimal)) {
            loaded.engine_temp_c = decimal; ++values_read;
        } else if (strcmp(key, "battery") == 0 && parse_double(value, &decimal)) {
            loaded.battery_voltage = decimal; ++values_read;
        } else if (strcmp(key, "odometer") == 0 && parse_double(value, &decimal)) {
            loaded.odometer_km = decimal; ++values_read;
        } else if (strcmp(key, "trip") == 0 && parse_double(value, &decimal)) {
            loaded.trip_km = decimal; ++values_read;
        } else if (strcmp(key, "speed_limit") == 0 && parse_int(value, &integer)) {
            loaded.speed_limit_kmh = integer; ++values_read;
        } else if (strcmp(key, "gear") == 0 && parse_int(value, &integer) &&
                   integer >= (int)GEAR_PARK && integer <= (int)GEAR_6) {
            loaded.gear = (Gear)integer; ++values_read;
        } else if (strcmp(key, "engine") == 0 && parse_int(value, &integer) &&
                   (integer == 0 || integer == 1)) {
            loaded.engine_on = integer != 0; ++values_read;
        } else if (strcmp(key, "reverse") == 0 && parse_int(value, &integer) &&
                   (integer == 0 || integer == 1)) {
            loaded.moving_in_reverse = integer != 0; ++values_read;
        } else if (strcmp(key, "headlights") == 0 && parse_int(value, &integer) &&
                   (integer == 0 || integer == 1)) {
            loaded.headlights_on = integer != 0; ++values_read;
        } else if (strcmp(key, "left_indicator") == 0 && parse_int(value, &integer) &&
                   (integer == 0 || integer == 1)) {
            loaded.left_indicator_on = integer != 0; ++values_read;
        } else if (strcmp(key, "right_indicator") == 0 && parse_int(value, &integer) &&
                   (integer == 0 || integer == 1)) {
            loaded.right_indicator_on = integer != 0; ++values_read;
        } else if (strcmp(key, "hazards") == 0 && parse_int(value, &integer) &&
                   (integer == 0 || integer == 1)) {
            loaded.hazards_on = integer != 0; ++values_read;
        } else if (strcmp(key, "seatbelt") == 0 && parse_int(value, &integer) &&
                   (integer == 0 || integer == 1)) {
            loaded.seatbelt_fastened = integer != 0; ++values_read;
        } else if (strcmp(key, "check_engine") == 0 && parse_int(value, &integer) &&
                   (integer == 0 || integer == 1)) {
            loaded.check_engine_on = integer != 0; ++values_read;
        }
    }
    read_failed = ferror(file);
    if (fclose(file) != 0 || read_failed != 0) {
        set_message(message, message_size, "Failed while reading the state file.");
        return false;
    }
    if (values_read < 10 || loaded.speed_kmh < 0.0 || loaded.speed_kmh > 240.0 ||
        loaded.rpm < 0.0 || loaded.rpm > 6500.0 ||
        loaded.fuel_percent < 0.0 || loaded.fuel_percent > 100.0 ||
        loaded.engine_temp_c < -40.0 || loaded.engine_temp_c > 130.0 ||
        loaded.battery_voltage < 8.0 || loaded.battery_voltage > 16.0 ||
        loaded.odometer_km < 0.0 || loaded.trip_km < 0.0 ||
        loaded.speed_limit_kmh < 10 || loaded.speed_limit_kmh > 300) {
        set_message(message, message_size, "State file is incomplete or contains invalid values.");
        return false;
    }
    if (!loaded.engine_on) {
        loaded.rpm = 0.0;
    }
    if (loaded.speed_kmh <= 0.05) {
        loaded.speed_kmh = 0.0;
        loaded.moving_in_reverse = false;
    }
    if (loaded.moving_in_reverse && loaded.gear != GEAR_REVERSE) {
        set_message(message, message_size, "State file contains an inconsistent motion direction.");
        return false;
    }
    *dashboard = loaded;
    set_message(message, message_size, "Dashboard state loaded from dashboard_state.txt.");
    return true;
}
