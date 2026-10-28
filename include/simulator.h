#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdbool.h>
#include <stddef.h>

#include "dashboard.h"

/* Every action reports a human-readable outcome through message. */
bool simulator_start_engine(Dashboard *dashboard, char *message, size_t message_size);
bool simulator_stop_engine(Dashboard *dashboard, char *message, size_t message_size);
bool simulator_accelerate(Dashboard *dashboard, double amount_kmh,
                          char *message, size_t message_size);
bool simulator_brake(Dashboard *dashboard, double amount_kmh,
                     char *message, size_t message_size);
bool simulator_change_gear(Dashboard *dashboard, Gear gear,
                           char *message, size_t message_size);
bool simulator_refuel(Dashboard *dashboard, double percent,
                      char *message, size_t message_size);
void simulator_reset_trip(Dashboard *dashboard);
void simulator_set_speed_limit(Dashboard *dashboard, int limit_kmh);
void simulator_step(Dashboard *dashboard, double seconds);
void simulator_auto_drive(Dashboard *dashboard, unsigned int seconds);

#endif
