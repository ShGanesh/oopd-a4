#ifndef CSV_LOADER_H
#define CSV_LOADER_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <algorithm>

#include "erp_types.h"

// Simple string split
inline std::vector<std::string> splitString(const std::string& s, char delim) {
    std::vector<std::string> parts;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        parts.push_back(item);
    }
    return parts;
}

// Trim whitespace from both ends (for safety)
inline std::string trim(const std::string& s) {
    const char* ws = " \t\r\n";
    std::size_t start = s.find_first_not_of(ws);
    if (start == std::string::npos) return "";
    std::size_t end = s.find_last_not_of(ws);
    return s.substr(start, end - start + 1);
}

// Parse one CSV record (already split into columns) into a concrete Student
// Expected columns:
//   0: Institute        ("IIIT" / "IIT")
//   1: Name
//   2: RollNumber
//   3: Branch
//   4: StartingYear
//   5: CurrentCourses        (semicolon-separated)
//   6: PastCoursesGrades     (semicolon-separated "course:grade")
inline IStudentPtr parseStudentRecord(const std::vector<std::string>& cols) {
    if (cols.size() < 7) {
        throw std::runtime_error("parseStudentRecord: not enough columns");
    }

    std::string institute   = trim(cols[0]);
    std::string name        = trim(cols[1]);
    std::string rollStr     = trim(cols[2]);
    std::string branch      = trim(cols[3]);
    std::string yearStr     = trim(cols[4]);
    std::string currentStr  = trim(cols[5]);
    std::string pastStr     = trim(cols[6]);

    unsigned int startingYear = 0;
    try {
        startingYear = static_cast<unsigned int>(std::stoul(yearStr));
    } catch (...) {
        throw std::runtime_error("Invalid starting year: " + yearStr);
    }

    // IIIT branch: roll = unsigned int, course codes = std::string
    if (institute == "IIIT") {
        auto stu = std::make_unique<IIITStudent>(name, rollStr, branch, startingYear);
        // unsigned int rollNum = 0;
        // try {
        //     rollNum = static_cast<unsigned int>(std::stoul(rollStr));
        // } catch (...) {
        //     throw std::runtime_error("Invalid IIIT roll number: " + rollStr);
        // }
        // 
        // auto stu = std::make_unique<IIITStudent>(name, rollNum, branch, startingYear);

        // CurrentCourses: semicolon-separated strings
        if (!currentStr.empty()) {
            auto tokens = splitString(currentStr, ';');
            for (auto& t : tokens) {
                std::string courseCode = trim(t);
                if (!courseCode.empty()) {
                    stu->addCurrentCourse(courseCode); // std::string
                }
            }
        }

        // PastCoursesGrades: "course:grade;course:grade;..."
        if (!pastStr.empty()) {
            auto pairs = splitString(pastStr, ';');
            for (auto& token : pairs) {
                std::string entry = trim(token);
                if (entry.empty()) continue;

                auto cg = splitString(entry, ':');
                if (cg.size() != 2) continue; // skip malformed

                std::string courseCode = trim(cg[0]);
                std::string gradeStr   = trim(cg[1]);

                if (courseCode.empty()) continue;

                int grade = 0;
                try {
                    grade = std::stoi(gradeStr);
                } catch (...) {
                    continue; // skip bad grade
                }
                stu->addPastCourse(courseCode, grade);
            }
        }

        return stu;
    }

    // IIT branch: roll = std::string, course codes = int
    if (institute == "IIT") {
        unsigned int rollNum = 0;
        try {
            // NOTE: Raises error if Roll number is not a pure unsigned int". 
            rollNum = static_cast<unsigned int>(std::stoul(rollStr));
        } catch (...) {
            throw std::runtime_error("Invalid IIT roll number: " + rollStr);
        }
        auto stu = std::make_unique<IITStudent>(name, rollNum, branch, startingYear);
        // CurrentCourses: semicolon-separated ints
        if (!currentStr.empty()) {
            auto tokens = splitString(currentStr, ';');
            for (auto& t : tokens) {
                std::string courseStr = trim(t);
                if (courseStr.empty()) continue;
                try {
                    int courseCode = std::stoi(courseStr);
                    stu->addCurrentCourse(courseCode);
                } catch (...) {
                    // skip malformed course codes
                }
            }
        }

        // PastCoursesGrades: "course:grade;course:grade;..."
        if (!pastStr.empty()) {
            auto pairs = splitString(pastStr, ';');
            for (auto& token : pairs) {
                std::string entry = trim(token);
                if (entry.empty()) continue;

                auto cg = splitString(entry, ':');
                if (cg.size() != 2) continue;

                std::string courseStr = trim(cg[0]);
                std::string gradeStr  = trim(cg[1]);

                if (courseStr.empty()) continue;

                int courseCode = 0;
                int grade      = 0;
                try {
                    courseCode = std::stoi(courseStr);
                    grade      = std::stoi(gradeStr);
                } catch (...) {
                    continue; // skip malformed pair
                }

                stu->addPastCourse(courseCode, grade);
            }
        }

        return stu;
    }

    // If institute is unknown, you can either throw or skip.
    throw std::runtime_error("Unknown institute: " + institute);
}

// Load students from a CSV file into a single container of polymorphic pointers.
// Preserves the insertion order from the file.
inline std::vector<IStudentPtr> loadStudentsFromCSV(const std::string& filename) {
    std::vector<IStudentPtr> students;

    std::ifstream fin(filename);
    if (!fin.is_open()) {
        throw std::runtime_error("Could not open CSV file: " + filename);
    }

    std::string line;

    // Skip header line
    if (!std::getline(fin, line)) {
        return students; // empty file
    }

    // Read records
    while (std::getline(fin, line)) {
        if (line.empty()) continue;

        auto cols = splitString(line, ',');

        try {
            IStudentPtr ptr = parseStudentRecord(cols);
            students.push_back(std::move(ptr));
        } catch (const std::exception& e) {
            // Try and catch exceptons anywhere and everywhere xD
            continue;
        }
    }

    return students;
}

#endif // CSV_LOADER_H
