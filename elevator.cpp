#include <algorithm>
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
    // Contructor
    explicit Elevator(int startFloor = 0) : floor_(startFloor), direction_(Direction::IDLE) {}

    bool addRequest(const Request& request) {
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

        if (direction_ == Direction::IDLE) {
            Request* nearestRequest = findNearestRequest();
            if (nearestRequest == nullptr) {
                direction_ = Direction::IDLE;
                return;
            }

            direction_ = nearestRequest->getFloor() > floor_ ? Direction::UP : Direction::DOWN;
        }

        RequestType hallType = (direction_ == Direction::UP)
            ? RequestType::PICKUP_UP
            : RequestType::PICKUP_DOWN;

        Request hallRequest(floor_, hallType);
        Request destinationRequest(floor_, RequestType::DESTINATION);

        if (containsRequest(hallRequest) || containsRequest(destinationRequest)) {
            removeRequest(hallRequest);
            removeRequest(destinationRequest);
            stop();
            return;
        }

        if (!hasRequestAhead(direction_)) {
            direction_ = (direction_ == Direction::UP) ? Direction::DOWN : Direction::UP;
        }

        if (direction_ == Direction::UP) {
            ++floor_;
        } else if (direction_ == Direction::DOWN) {
            --floor_;
        }
    }

    int getFloor() const { return floor_; }
    Direction getDirection() const { return direction_; }
    std::vector<Request> getRequests() const { return requests_; }

    void stop() {
        direction_ = Direction::IDLE;
    }

private:
    bool containsRequest(const Request& request) const {
        for (const auto& existingRequest : requests_) {
            if (existingRequest == request) {
                return true;
            }
        }
        return false;
    }

    void removeRequest(const Request& request) {
        requests_.erase(
            std::remove_if(requests_.begin(), requests_.end(),
                [&request](const Request& existingRequest) {
                    return existingRequest == request;
                }),
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
    Direction direction_;
    std::vector<Request> requests_;
};

class ElevatorController {
public:
    // Constructor
    ElevatorController() {
        elevators_.emplace_back(Elevator(0));
        elevators_.emplace_back(Elevator(0));
        elevators_.emplace_back(Elevator(0));
    }

    bool requestElevator(int floor, RequestType type) {
        if (floor < 0 || floor > 9) {
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

    std::vector<Elevator> elevators_;
};

int main() {
    ElevatorController controller;

    std::cout << "Requesting elevator at floor 5 going UP: "
              << controller.requestElevator(5, RequestType::PICKUP_UP) << std::endl;

    std::cout << "Requesting elevator at floor 8 going DOWN: "
              << controller.requestElevator(8, RequestType::PICKUP_DOWN) << std::endl;

    for (int i = 0; i < 10; ++i) {
        controller.step();
        for (const auto& elevator : controller.getElevators()) {
            std::cout << "Elevator floor: " << elevator.getFloor()
                      << ", direction: " << static_cast<int>(elevator.getDirection())
                      << std::endl;
        }
    }

    return 0;
}
