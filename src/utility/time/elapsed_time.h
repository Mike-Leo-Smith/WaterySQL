//
// Created by Mike Smith on 2018/11/28.
//

#ifndef WATERYSQL_ELAPSED_TIME_H
#define WATERYSQL_ELAPSED_TIME_H

#include <type_traits>
#include <chrono>
#include <memory>
#include <functional>

namespace watery {

template<typename Func, typename ...Args>
std::pair<
    float,
    std::conditional_t<
        std::is_same_v<std::invoke_result_t<Func>, void>,
        int,
        std::invoke_result_t<Func>>>
timed_run(Func &&f, Args &&...args) {
    auto start = std::chrono::high_resolution_clock::now();
    using namespace std::chrono_literals;
    if constexpr (std::is_same_v<std::invoke_result_t<Func>, void>) {
        std::invoke(f, std::forward(args)...);
        auto stop = std::chrono::high_resolution_clock::now();
        return {(stop - start) / 1ns / 1e6f, 0};
    } else {
        auto &&result = std::invoke(f, std::forward(args)...);
        auto stop = std::chrono::high_resolution_clock::now();
        return {(stop - start) / 1ns / 1e6f, result};
    }
}

}

#endif  // WATERYSQL_ELAPSED_TIME_H
