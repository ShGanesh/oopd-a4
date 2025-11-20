#ifndef SORTING_H
#define SORTING_H

#include <vector>
#include <list>
#include <numeric>
#include <thread>
#include <chrono>
#include <iostream>
#include <string>

#include "erp_types.h"

// Simple time logger
inline void logDuration(const std::string& label,
                        const std::chrono::high_resolution_clock::time_point& start,
                        const std::chrono::high_resolution_clock::time_point& end)
{
    using namespace std::chrono;
    auto dur = duration_cast<milliseconds>(end - start).count();
    std::cout << "[TIMER] " << label << " took " << dur << " ms\n";
}

// Holds sorted views (indices), does not copy student objects
struct SortViews {
    // These are the different iterators.
    std::vector<std::size_t> byName;
    std::vector<std::size_t> byRoll;
    std::list<std::size_t> byNameList; // same as byName, but using list iterators
};

// Build index vectors and sort them in parallel.
// No race conditions: students is read-only, and each thread owns its index vector.

// Implementation Strategy:
// 1. NO COPYING: I do not sort the `students` vector. Instead,I create lightweight vectors of INDICES (`byName`, `byRoll`).
// 2. THREAD SAFETY: The main `students` vector is treated as READ-ONLY during sorting. Each thread modifies its own private index vector.
//    Since they don't write to the same memory location, there is no Race Condition.

inline SortViews buildAndSortViews(const std::vector<IStudentPtr>& students) {
    SortViews views;

    const std::size_t n = students.size();
    views.byName.resize(n);
    views.byRoll.resize(n);

    // Fill both index vectors with 0..n-1.
    std::iota(views.byName.begin(), views.byName.end(), 0);
    std::iota(views.byRoll.begin(),  views.byRoll.end(),  0);
    
    // Sorting tasks.
    auto sortByName = [&]() {
        auto start = std::chrono::high_resolution_clock::now();
        std::sort(views.byName.begin(), views.byName.end(),
                  [&](std::size_t a, std::size_t b) {
                      if (!students[a] || !students[b]) return a < b;
                      return students[a]->getNameStr() < students[b]->getNameStr();
                  });
        auto end = std::chrono::high_resolution_clock::now();
        logDuration("Sort by name", start, end);
    };

    auto sortByRoll = [&]() {
        auto start = std::chrono::high_resolution_clock::now();
        std::sort(views.byRoll.begin(), views.byRoll.end(),
                  [&](std::size_t a, std::size_t b) {
                      if (!students[a] || !students[b]) return a < b;
                      return students[a]->getRollStr() < students[b]->getRollStr();
                  });
        auto end = std::chrono::high_resolution_clock::now();
        logDuration("Sort by roll", start, end);
    };

    // Two threads in parallel
    std::thread t1(sortByName);
    std::thread t2(sortByRoll);
    t1.join();
    t2.join();

    // Build a list-based view from the name-sorted indices (different iterator type, Q4).
    views.byNameList.assign(views.byName.begin(), views.byName.end());

    return views;
}

#endif // SORTING_H
