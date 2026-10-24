#include "task.hpp"

Task::Task(
    std::function<double(std::array<size_t, 5>)> func,
    std::array<size_t, 5> args,
    std::promise<double> promise)
    : func(std::move(func)), args(args), promise(std::move(promise)) {}

Task::Task(): func(nullptr), args({}), promise() {}

Task& Task::operator=(Task&& other)  noexcept {
        if (this != &other) {
            func = std::move(other.func);
            args = other.args;
            promise = std::move(other.promise);
        }
        return *this;
}

Task::Task(Task&& other) noexcept
        : func(std::move(other.func)),
          args(other.args),
          promise(std::move(other.promise)) {}
