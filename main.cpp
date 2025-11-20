// main.cpp
#include <iostream>
#include <string>
#include <limits>

#include "csv_loader.h"
#include "print_utils.h"
#include "sorting.h"
#include "course_index.h"

// Helper to safely get a line from std::cin after numeric input
inline void clearInputLine() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main() {
    std::string filename;
    std::cout << "Enter CSV filename (e.g. students_sample.csv): ";
    std::getline(std::cin, filename);

    if (filename.empty()) {
        std::cout << "No filename given.\n";
        return 0;
    }

    // 1. Load students from CSV
    std::vector<IStudentPtr> students;
    try {
        students = loadStudentsFromCSV(filename);
    } catch (const std::exception& e) {
        std::cerr << "Error loading CSV: " << e.what() << "\n";
        return 1;
    }

    if (students.empty()) {
        std::cout << "No students loaded.\n";
        return 0;
    }

    // 2. Build sorted views (parallel sorting)
    SortViews views = buildAndSortViews(students);

    // 3. Build course index
    CourseIndexDB courseIndex;
    courseIndex.build(students);

    // 4. Interactive menu
    while (true) {
        std::cout << "\n===== ERP MENU =====\n"
                  << "1. Show students (insertion order)\n"
                  << "2. Show students sorted by name\n"
                  << "3. Show students sorted by roll\n"
                  << "4. Show students sorted by name (list iterator view)\n"
                  << "5. Query: students with grade >= 9 in a course\n"
                  << "6. Query: students with grade >= custom threshold in a course\n"
                  << "0. Exit\n"
                  << "Enter choice: ";

        int choice = -1;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            clearInputLine();
            std::cout << "Invalid input. Try again.\n";
            continue;
        }
        clearInputLine();

        if (choice == 0) {
            std::cout << "Exiting ERP.\n";
            break;
        }

        switch (choice) {
        case 1: {
            printStudentsInsertionOrder(students);
            break;
        }
        case 2: {
            printStudentsByIndex(students,
                                 views.byName.begin(),
                                 views.byName.end());
            break;
        }
        case 3: {
            printStudentsByIndex(students,
                                 views.byRoll.begin(),
                                 views.byRoll.end());
            break;
        }
        case 4: {
            // Demonstrate using a different iterator type (list)
            printStudentsByIndex(students,
                                 views.byNameList.begin(),
                                 views.byNameList.end());
            break;
        }
        case 5: {
            std::string course;
            std::cout << "Enter course code (as in CSV, e.g. 801, OOPD): ";
            std::getline(std::cin, course);
            course = trim(course); // trim is from csv_loader.h

            int threshold = 9;
            auto result = courseIndex.queryAtLeast(course, threshold);

            std::cout << "Students with grade >= " << threshold
                      << " in course '" << course << "':\n";
            if (result.empty()) {
                std::cout << "(none)\n";
            } else {
                for (IStudent* s : result) {
                    printStudent(*s);
                }
            }
            break;
        }
        case 6: {
            std::string course;
            std::cout << "Enter course code (as in CSV, e.g. 801, OOPD): ";
            std::getline(std::cin, course);
            course = trim(course);

            std::cout << "Enter minimum grade (0–10): ";
            int threshold = 0;
            if (!(std::cin >> threshold)) {
                std::cin.clear();
                clearInputLine();
                std::cout << "Invalid grade.\n";
                break;
            }
            clearInputLine();

            auto result = courseIndex.queryAtLeast(course, threshold);
            std::cout << "Students with grade >= " << threshold
                      << " in course '" << course << "':\n";
            if (result.empty()) {
                std::cout << "(none)\n";
            } else {
                for (IStudent* s : result) {
                    printStudent(*s);
                }
            }
            break;
        }
        default:
            std::cout << "Unknown choice. Try again.\n";
            break;
        }
    }

    return 0;
}
