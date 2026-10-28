#ifndef STORAGE_H
#define STORAGE_H

#include <stdbool.h>
#include <stddef.h>

#include "dashboard.h"

bool storage_save(const Dashboard *dashboard, const char *path,
                  char *message, size_t message_size);
bool storage_load(Dashboard *dashboard, const char *path,
                  char *message, size_t message_size);

#endif
