# Vehicle Dashboard Information Simulator

A terminal-only, ISO C11 vehicle dashboard simulator. It uses standard input/output, optional ANSI colour, and no external libraries or GUI frameworks.

## Build and run

From this project directory:

```sh
make build
make run
```

The build uses:

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic
```

Run the automated tests with:

```sh
make test
```

Remove generated executables and temporary test state with:

```sh
make clean
```

## Controls

The main menu lets you start and stop the engine, select `P`, `R`, `N`, `D`, or gears `1`-`6`, accelerate, brake, refuel, reset the trip meter, control lighting/signals, manage the seatbelt, configure a speed limit, and trigger or clear a simulated check-engine light.

Automatic drive simulation advances the model by a chosen number of seconds. The manual advance option is useful for observing idle temperature, battery drain, and other changes without driving.

The program validates every menu and numeric input. It also prevents unsafe state changes such as accelerating with the engine off, starting outside Park/Neutral, stopping the engine while moving, selecting reverse while travelling forward, and switching from reverse into a forward gear while moving.

## Dashboard values and warnings

- Speed, RPM, fuel, engine temperature, battery voltage, odometer, and trip distance
- Gear, engine, headlights, indicators, hazards, and seatbelt states
- Seatbelt, check-engine, low-fuel, overheating, low-battery, and speed-limit warnings
- Fuel consumption, distance, RPM, temperature, and battery changes calculated each simulation step

## Save/load

Choose **Save dashboard state** to write `dashboard_state.txt` in the project folder. **Load dashboard state** restores that local text file after checking its contents.

## Project layout

    include/    Public data model and module interfaces
    src/        Menu, dashboard rendering, simulation rules, and persistence
    tests/      Automated simulator tests
    Makefile    Build, run, clean, and test targets
