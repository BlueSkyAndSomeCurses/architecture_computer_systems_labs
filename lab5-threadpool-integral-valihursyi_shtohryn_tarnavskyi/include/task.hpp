#pragma once
#ifndef TASK_H
#define TASK_H

#include <functional>
#include <array>
#include <future>

struct Task {
    std::function<double(std::array<size_t, 5>)> func;
    std::array<size_t, 5> args;
    std::promise<double> promise;

    Task();

    Task(
        std::function<double(std::array<size_t, 5>)> func,
        std::array<size_t, 5> args,
        std::promise<double> promise);

    Task &operator=(Task &&other) noexcept ;

    Task(Task &&other) noexcept;

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
};

#endif
