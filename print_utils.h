#ifndef PRINT_UTILS_H
#define PRINT_UTILS_H

#include <iostream>
#include <vector>
#include <cstddef>
#include "erp_types.h"

// Basic printer for a single student
inline void printStudent(const IStudent& s, std::ostream& os = std::cout) {
    os << "Name: "          << s.getNameStr()
       << ", Roll: "        << s.getRollStr()
       << ", Branch: "      << s.getBranchStr()
       << ", StartingYear: " << s.getStartingYear()
       << '\n';
}

// Print list in insertion order (directly over students vector)
inline void printStudentsInsertionOrder(
    const std::vector<IStudentPtr>& students,
    std::ostream& os = std::cout
) {
    os << "=== Students (insertion order) ===\n";
    for (const auto& ptr : students) {
        if (ptr) printStudent(*ptr, os);
    }
    os << "==================================\n";
}

// Print list according to an index container (vector<size_t>, list<size_t>, etc.)
template<typename IndexIter>
inline void printStudentsByIndex(
    const std::vector<IStudentPtr>& students,
    IndexIter begin,
    IndexIter end,
    std::ostream& os = std::cout
) {
    os << "=== Students (indexed view) ===\n";
    for (auto it = begin; it != end; ++it) {
        std::size_t idx = static_cast<std::size_t>(*it);
        if (idx < students.size() && students[idx]) {
            printStudent(*students[idx], os);
        }
    }
    os << "================================\n";
}

#endif // PRINT_UTILS_H
