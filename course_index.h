#ifndef COURSE_INDEX_H
#define COURSE_INDEX_H

#include <unordered_map>
#include <array>
#include <vector>
#include <string>
#include <algorithm>
#include "erp_types.h"

// For a given course, keep buckets by grade (0..10), in case we wanted floating point grades.
struct CourseIndex {
    std::array<std::vector<IStudent*>, 11> grades; // grades[g] = students with grade == g
};

// Holds indices for all courses
class CourseIndexDB {
public:
    // Build index from students list (a pre-process).
    // Uses the IStudent abstraction to iterate over past courses.
    void build(const std::vector<IStudentPtr>& students) {
        index_.clear();
        index_.reserve(students.size() * 2); // heuristic

        for (const auto& uptr : students) {
            if (!uptr) continue;
            IStudent* s = uptr.get();
            s->forEachPastCourse([&](const std::string& course, int grade) {
                if (grade < 0 || grade > 10) return;
                CourseIndex& ci = index_[course]; // O(1) access
                ci.grades[grade].push_back(s); // O(1) insertion
            });
        }
    }

    // Query: all students with grade >= threshold in given course.
    // Returns only pointers.
    std::vector<IStudent*> queryAtLeast(const std::string& course,
                                        int threshold) const
    {
        std::vector<IStudent*> result;
        auto it = index_.find(course);
        if (it == index_.end()) return result;

        int t = std::max(threshold, 0);
        t = std::min(t, 10);

        const CourseIndex& ci = it->second;
        for (int g = t; g <= 10; ++g) {
            const auto& bucket = ci.grades[g];
            result.insert(result.end(), bucket.begin(), bucket.end());
        }

        return result;
    }

private:
    std::unordered_map<std::string, CourseIndex> index_;
};

#endif // COURSE_INDEX_H
