CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS := -lm
BUILD_DIR := build
APP := $(BUILD_DIR)/vehicle_dashboard
TEST_APP := $(BUILD_DIR)/test_simulator
APP_SOURCES := src/main.c src/dashboard.c src/simulator.c src/storage.c
TEST_SOURCES := tests/test_simulator.c src/dashboard.c src/simulator.c src/storage.c

.PHONY: build run clean test

build: $(APP)

$(APP): $(APP_SOURCES) include/dashboard.h include/simulator.h include/storage.h
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(APP_SOURCES) -o $@ $(LDFLAGS)

run: build
	./$(APP)

$(TEST_APP): $(TEST_SOURCES) include/dashboard.h include/simulator.h include/storage.h
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_SOURCES) -o $@ $(LDFLAGS)

test: $(TEST_APP)
	./$(TEST_APP)

clean:
	rm -rf $(BUILD_DIR)
