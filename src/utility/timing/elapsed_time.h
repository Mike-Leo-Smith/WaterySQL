//
// Created by Mike Smith on 2018/11/28.
//

#ifndef WATERYSQL_ELAPSED_TIME_H
#define WATERYSQL_ELAPSED_TIME_H

#include <chrono>

namespace watery {

template<typename Func>
float elapsed_time_ms(Func &&f) {
    auto start = std::chrono::high_resolution_clock::now();
    f();
    auto stop = std::chrono::high_resolution_clock::now();
    using namespace std::chrono_literals;
    return (stop - start) / 1ns / 1e6f;
}

}

#endif  // WATERYSQL_ELAPSED_TIME_H
