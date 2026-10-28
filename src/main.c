#include "dashboard.h"
#include "simulator.h"
#include "storage.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_SIZE 128
#define MESSAGE_SIZE 192
#define STATE_FILE "dashboard_state.txt"

static bool read_line(const char *prompt, char *buffer, size_t buffer_size)
{
    size_t length;
    int character;

    if (prompt != NULL) {
        fputs(prompt, stdout);
    }
    if (fgets(buffer, (int)buffer_size, stdin) == NULL) {
        return false;
    }
    length = strlen(buffer);
    if (length > 0U && buffer[length - 1U] == '\n') {
        buffer[length - 1U] = '\0';
    } else {
        do {
            character = getchar();
        } while (character != '\n' && character != EOF);
    }
    return true;
}

static char *skip_spaces(char *text)
{
    while (*text != '\0' && isspace((unsigned char)*text)) {
        ++text;
    }
    return text;
}

static bool parse_long_value(char *text, long *value)
{
    char *end;
    char *start = skip_spaces(text);
    long parsed;

    errno = 0;
    parsed = strtol(start, &end, 10);
    if (errno != 0 || end == start) {
        return false;
    }
    end = skip_spaces(end);
    if (*end != '\0') {
        return false;
    }
    *value = parsed;
    return true;
}

static bool parse_double_value(char *text, double *value)
{
    char *end;
    char *start = skip_spaces(text);
    double parsed;

    errno = 0;
    parsed = strtod(start, &end);
    if (errno != 0 || end == start || !isfinite(parsed)) {
        return false;
    }
    end = skip_spaces(end);
    if (*end != '\0') {
        return false;
    }
    *value = parsed;
    return true;
}

static bool read_int_range(const char *prompt, int minimum, int maximum, int *value)
{
    char input[INPUT_SIZE];
    long parsed;

    for (;;) {
        if (!read_line(prompt, input, sizeof input)) {
            return false;
        }
        if (parse_long_value(input, &parsed) && parsed >= minimum && parsed <= maximum) {
            *value = (int)parsed;
            return true;
        }
        printf("Please enter a whole number from %d to %d.\n", minimum, maximum);
    }
}

static bool read_double_range(const char *prompt, double minimum, double maximum, double *value)
{
    char input[INPUT_SIZE];
    double parsed;

    for (;;) {
        if (!read_line(prompt, input, sizeof input)) {
            return false;
        }
        if (parse_double_value(input, &parsed) && parsed >= minimum && parsed <= maximum) {
            *value = parsed;
            return true;
        }
        printf("Please enter a number from %.1f to %.1f.\n", minimum, maximum);
    }
}

static bool read_gear(Gear *gear)
{
    char input[INPUT_SIZE];
    char *value;

    for (;;) {
        if (!read_line("Select gear (P, R, N, D, 1-6): ", input, sizeof input)) {
            return false;
        }
        value = skip_spaces(input);
        if (value[0] != '\0' && value[1] == '\0') {
            switch (toupper((unsigned char)value[0])) {
            case 'P': *gear = GEAR_PARK; return true;
            case 'R': *gear = GEAR_REVERSE; return true;
            case 'N': *gear = GEAR_NEUTRAL; return true;
            case 'D': *gear = GEAR_DRIVE; return true;
            case '1': *gear = GEAR_1; return true;
            case '2': *gear = GEAR_2; return true;
            case '3': *gear = GEAR_3; return true;
            case '4': *gear = GEAR_4; return true;
            case '5': *gear = GEAR_5; return true;
            case '6': *gear = GEAR_6; return true;
            default: break;
            }
        }
        puts("Enter exactly one valid gear: P, R, N, D, or 1 through 6.");
    }
}

static void print_menu(void)
{
    puts("\n  MENU");
    puts("   1. Start / stop engine");
    puts("   2. Accelerate");
    puts("   3. Brake");
    puts("   4. Change gear");
    puts("   5. Refuel");
    puts("   6. Reset trip meter");
    puts("   7. Toggle headlights");
    puts("   8. Manage indicators / hazards");
    puts("   9. Fasten / unfasten seatbelt");
    puts("  10. Toggle check-engine warning");
    puts("  11. Configure speed limit");
    puts("  12. Automatic drive simulation");
    puts("  13. Advance simulation manually");
    puts("  14. Save dashboard state");
    puts("  15. Load dashboard state");
    puts("   0. Exit");
}

static bool manage_indicators(Dashboard *dashboard, char *message, size_t message_size)
{
    int choice;

    puts("  1. Toggle left indicator");
    puts("  2. Toggle right indicator");
    puts("  3. Toggle hazard lights");
    puts("  4. Turn all indicators off");
    if (!read_int_range("Indicator action: ", 1, 4, &choice)) {
        return false;
    }
    switch (choice) {
    case 1:
        dashboard->hazards_on = false;
        dashboard->left_indicator_on = !dashboard->left_indicator_on;
        (void)snprintf(message, message_size, "Left indicator %s.",
                       dashboard->left_indicator_on ? "enabled" : "disabled");
        break;
    case 2:
        dashboard->hazards_on = false;
        dashboard->right_indicator_on = !dashboard->right_indicator_on;
        (void)snprintf(message, message_size, "Right indicator %s.",
                       dashboard->right_indicator_on ? "enabled" : "disabled");
        break;
    case 3:
        dashboard->hazards_on = !dashboard->hazards_on;
        (void)snprintf(message, message_size, "Hazard lights %s.",
                       dashboard->hazards_on ? "enabled" : "disabled");
        break;
    default:
        dashboard->left_indicator_on = false;
        dashboard->right_indicator_on = false;
        dashboard->hazards_on = false;
        (void)snprintf(message, message_size, "All indicators switched off.");
        break;
    }
    return true;
}

int main(void)
{
    Dashboard dashboard;
    char message[MESSAGE_SIZE] = "Welcome. The vehicle is parked and engine is off.";
    bool running = true;

    dashboard_init(&dashboard);
    while (running) {
        int choice;
        double amount;

        dashboard_display(&dashboard, message);
        print_menu();
        if (!read_int_range("\nChoose an action: ", 0, 15, &choice)) {
            puts("\nInput stream closed. Goodbye.");
            break;
        }

        switch (choice) {
        case 0:
            running = false;
            break;
        case 1:
            if (dashboard.engine_on) {
                (void)simulator_stop_engine(&dashboard, message, sizeof message);
            } else {
                (void)simulator_start_engine(&dashboard, message, sizeof message);
            }
            break;
        case 2:
            if (read_double_range("Accelerate by km/h (1-100): ", 1.0, 100.0, &amount)) {
                (void)simulator_accelerate(&dashboard, amount, message, sizeof message);
            } else {
                running = false;
            }
            break;
        case 3:
            if (read_double_range("Brake by km/h (1-100): ", 1.0, 100.0, &amount)) {
                (void)simulator_brake(&dashboard, amount, message, sizeof message);
            } else {
                running = false;
            }
            break;
        case 4: {
            Gear gear;
            if (read_gear(&gear)) {
                (void)simulator_change_gear(&dashboard, gear, message, sizeof message);
            } else {
                running = false;
            }
            break;
        }
        case 5:
            if (read_double_range("Add fuel percentage (1-100): ", 1.0, 100.0, &amount)) {
                (void)simulator_refuel(&dashboard, amount, message, sizeof message);
            } else {
                running = false;
            }
            break;
        case 6:
            simulator_reset_trip(&dashboard);
            (void)snprintf(message, sizeof message, "Trip meter reset to 0.0 km.");
            break;
        case 7:
            dashboard.headlights_on = !dashboard.headlights_on;
            (void)snprintf(message, sizeof message, "Headlights %s.",
                           dashboard.headlights_on ? "enabled" : "disabled");
            break;
        case 8:
            if (!manage_indicators(&dashboard, message, sizeof message)) {
                running = false;
            }
            break;
        case 9:
            dashboard.seatbelt_fastened = !dashboard.seatbelt_fastened;
            (void)snprintf(message, sizeof message, "Seatbelt %s.",
                           dashboard.seatbelt_fastened ? "fastened" : "unfastened");
            break;
        case 10:
            dashboard.check_engine_on = !dashboard.check_engine_on;
            (void)snprintf(message, sizeof message, "Check-engine warning %s.",
                           dashboard.check_engine_on ? "enabled" : "cleared");
            break;
        case 11: {
            int limit;
            if (read_int_range("Speed limit in km/h (10-300): ", 10, 300, &limit)) {
                simulator_set_speed_limit(&dashboard, limit);
                (void)snprintf(message, sizeof message, "Speed limit set to %d km/h.", limit);
            } else {
                running = false;
            }
            break;
        }
        case 12: {
            int seconds;
            if (read_int_range("Auto-drive duration in seconds (1-3600): ", 1, 3600, &seconds)) {
                simulator_auto_drive(&dashboard, (unsigned int)seconds);
                (void)snprintf(message, sizeof message,
                               "Automatic simulation advanced by %d second(s).", seconds);
            } else {
                running = false;
            }
            break;
        }
        case 13: {
            int seconds;
            if (read_int_range("Advance by seconds (1-3600): ", 1, 3600, &seconds)) {
                simulator_step(&dashboard, (double)seconds);
                (void)snprintf(message, sizeof message,
                               "Simulation advanced by %d second(s).", seconds);
            } else {
                running = false;
            }
            break;
        }
        case 14:
            (void)storage_save(&dashboard, STATE_FILE, message, sizeof message);
            break;
        case 15:
            (void)storage_load(&dashboard, STATE_FILE, message, sizeof message);
            break;
        default:
            break;
        }
    }

    puts("Thanks for using Vehicle Dashboard Information Simulator.");
    return 0;
}
