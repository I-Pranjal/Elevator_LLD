# Elevator Low-Level Design

This example models a small elevator system with:

- `Request` objects for up, down, and destination requests.
- An `Elevator` state machine that moves one floor per `step()` call.
- An `ElevatorController` that assigns requests to suitable elevators.
- Floor-range validation, duplicate-request protection, current-floor servicing, and direction reversal.

## Build and run

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror elevator.cpp -o elevator
./elevator
```

The executable runs assertions for the core behavior and prints `Elevator self-checks passed.` when they succeed.
