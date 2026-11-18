#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

enum class Direction {
    UP,
    DOWN,
    IDLE
};

enum class RequestType {
    PICKUP_UP,
    PICKUP_DOWN,
    DESTINATION
};

class Request {
public:
    Request(int floor, RequestType type) : floor_(floor), type_(type) {}

    int getFloor() const { return floor_; }
    RequestType getType() const { return type_; }

    bool operator==(const Request& other) const {
        return floor_ == other.floor_ && type_ == other.type_;
    }

private:
    int floor_;
    RequestType type_;
};

class Elevator {
public:
    explicit Elevator(int startFloor = 0, int minFloor = 0, int maxFloor = 9)
        : floor_(startFloor), minFloor_(minFloor), maxFloor_(maxFloor),
          direction_(Direction::IDLE) {
        if (minFloor > maxFloor || startFloor < minFloor || startFloor > maxFloor) {
            throw std::invalid_argument("Invalid elevator floor range");
        }
    }

    bool addRequest(const Request& request) {
        validateFloor(request.getFloor());
        if (containsRequest(request)) {
            return false;
        }
        requests_.push_back(request);
        return true;
    }

    void step() {
        if (requests_.empty()) {
            direction_ = Direction::IDLE;
            return;
        }

        if (hasRequestAtCurrentFloor()) {
            removeRequestsAtCurrentFloor();
            direction_ = chooseDirection();
            return;
        }

        if (direction_ == Direction::IDLE || !hasRequestAhead(direction_)) {
            direction_ = chooseDirection();
        }

        if (direction_ == Direction::UP && floor_ < maxFloor_) {
            ++floor_;
        } else if (direction_ == Direction::DOWN && floor_ > minFloor_) {
            --floor_;
        } else {
            direction_ = chooseDirection();
        }
    }

    int getFloor() const { return floor_; }
    Direction getDirection() const { return direction_; }
    std::vector<Request> getRequests() const { return requests_; }

    void stop() {
        direction_ = Direction::IDLE;
    }

private:
    void validateFloor(int floor) const {
        if (floor < minFloor_ || floor > maxFloor_) {
            throw std::invalid_argument("Floor is outside the elevator range");
        }
    }

    bool containsRequest(const Request& request) const {
        for (const auto& existingRequest : requests_) {
            if (existingRequest == request) {
                return true;
            }
        }
        return false;
    }

    bool hasRequestAtCurrentFloor() const {
        return std::any_of(requests_.begin(), requests_.end(),
            [this](const Request& request) { return request.getFloor() == floor_; });
    }

    void removeRequestsAtCurrentFloor() {
        requests_.erase(
            std::remove_if(requests_.begin(), requests_.end(),
                [this](const Request& request) { return request.getFloor() == floor_; }),
            requests_.end());
    }

    Request* findNearestRequest() {
        Request* nearest = nullptr;
        int minDistance = std::numeric_limits<int>::max();

        for (auto& request : requests_) {
            int distance = std::abs(request.getFloor() - floor_);
            if (distance < minDistance) {
                minDistance = distance;
                nearest = &request;
            }
        }

        return nearest;
    }

    Direction chooseDirection() {
        Request* nearest = findNearestRequest();
        if (nearest == nullptr || nearest->getFloor() == floor_) {
            return Direction::IDLE;
        }
        return nearest->getFloor() > floor_ ? Direction::UP : Direction::DOWN;
    }

    bool hasRequestAhead(Direction dir) const {
        for (const auto& request : requests_) {
            if (dir == Direction::UP && request.getFloor() > floor_) {
                return true;
            }
            if (dir == Direction::DOWN && request.getFloor() < floor_) {
                return true;
            }
        }
        return false;
    }

    int floor_;
    int minFloor_;
    int maxFloor_;
    Direction direction_;
    std::vector<Request> requests_;
};

class ElevatorController {
public:
    ElevatorController(int elevatorCount = 3, int minFloor = 0, int maxFloor = 9)
        : minFloor_(minFloor), maxFloor_(maxFloor) {
        if (elevatorCount <= 0 || minFloor > maxFloor) {
            throw std::invalid_argument("Invalid controller configuration");
        }
        for (int i = 0; i < elevatorCount; ++i) {
            elevators_.emplace_back(minFloor_, minFloor_, maxFloor_);
        }
    }

    bool requestElevator(int floor, RequestType type) {
        if (floor < minFloor_ || floor > maxFloor_) {
            throw std::invalid_argument("Not a valid floor");
        }

        Request request(floor, type);
        Elevator* best = selectBestElevator(request);
        if (best == nullptr) {
            return false;
        }

        return best->addRequest(request);
    }

    void step() {
        for (auto& elevator : elevators_) {
            elevator.step();
        }
    }

    const std::vector<Elevator>& getElevators() const {
        return elevators_;
    }

private:
    Elevator* findMovingToward(const Request& request) {
        if (request.getType() == RequestType::DESTINATION) {
            return nullptr;
        }

        Direction direction = (request.getType() == RequestType::PICKUP_UP)
            ? Direction::UP
            : Direction::DOWN;

        Elevator* nearest = nullptr;
        int minDistance = std::numeric_limits<int>::max();

        for (auto& elevator : elevators_) {
            Direction elevatorDirection = elevator.getDirection();
            if (elevatorDirection == Direction::IDLE) {
                continue;
            }

            if (direction == Direction::UP && elevator.getFloor() > request.getFloor()) {
                continue;
            }
            if (direction == Direction::DOWN && elevator.getFloor() < request.getFloor()) {
                continue;
            }

            int distance = std::abs(elevator.getFloor() - request.getFloor());
            if (distance < minDistance) {
                minDistance = distance;
                nearest = &elevator;
            }
        }

        return nearest;
    }

    Elevator* findNearestIdle(int floor) {
        Elevator* nearest = nullptr;
        int minDistance = std::numeric_limits<int>::max();

        for (auto& elevator : elevators_) {
            if (elevator.getDirection() != Direction::IDLE) {
                continue;
            }

            int distance = std::abs(elevator.getFloor() - floor);
            if (distance < minDistance) {
                minDistance = distance;
                nearest = &elevator;
            }
        }

        return nearest;
    }

    Elevator* findNearest(int floor) {
        Elevator* nearest = nullptr;
        int minDistance = std::numeric_limits<int>::max();

        for (auto& elevator : elevators_) {
            int distance = std::abs(elevator.getFloor() - floor);
            if (distance < minDistance) {
                minDistance = distance;
                nearest = &elevator;
            }
        }

        return nearest;
    }

    Elevator* selectBestElevator(const Request& request) {
        Elevator* best = findMovingToward(request);
        if (best != nullptr) {
            return best;
        }

        best = findNearestIdle(request.getFloor());
        if (best != nullptr) {
            return best;
        }

        return findNearest(request.getFloor());
    }

    int minFloor_;
    int maxFloor_;
    std::vector<Elevator> elevators_;
};

int main() {
    Elevator elevator;
    assert(elevator.addRequest(Request(3, RequestType::PICKUP_UP)));
    assert(!elevator.addRequest(Request(3, RequestType::PICKUP_UP)));
    for (int step = 0; step < 3; ++step) {
        elevator.step();
    }
    assert(elevator.getFloor() == 3);
    elevator.step();
    assert(elevator.getDirection() == Direction::IDLE);
    assert(elevator.getRequests().empty());

    ElevatorController controller;
    assert(controller.requestElevator(8, RequestType::PICKUP_DOWN));

    for (int i = 0; i < 9; ++i) {
        controller.step();
    }
    assert(controller.getElevators()[0].getRequests().empty());

    std::cout << "Elevator self-checks passed.\n";

    return 0;
}
