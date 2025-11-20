#ifndef STUDENT_H
#define STUDENT_H

#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <array>

/* 
Data Abstraction and Hiding

Implementation: 
1. IStudent acts as a Pure Virtual, abstract interface. It hidesthe underlying complexity 
of the template types from the rest of the system.
2. Concrete data (vectors, names) is private (Data Hiding).
*/

class IStudent {
public:
    virtual ~IStudent() = default;

    // Basic info
    virtual std::string getNameStr() const = 0;
    virtual std::string getRollStr() const = 0;
    virtual std::string getBranchStr() const = 0;
    virtual unsigned int getStartingYear() const = 0;

    // Iterate over all past courses (courseCodeStr, grade).
    // Apply a function to each past course taken by this student.
    // (courseCodeAsString, grade)
    virtual void forEachPastCourse(
        const std::function<void(const std::string&, int)>& f
    ) const = 0;

    // Helper for Q5 (Fast Query) logic: check if this student has >= threshold in given course.
    // All "course" are compared in string form.
    virtual bool hasGradeAtLeast(const std::string& course,
                                 int threshold) const = 0;
};


// Helper: toString for arbitrary types
template<typename T>
inline std::string toStringGeneric(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// overload for std::string to avoid extra copy if needed
inline std::string toStringGeneric(const std::string& value) {
    return value;
}

inline std::string toStringGeneric(const char* value) {
    return std::string(value);
}


/*
Template class: Student<RollT, CourseCodeT>

This is the SINGLE generic class requested in Q1, and can handle different data types for 
roll numbers and course codes.
Examples:
  using IIITStudent = Student<std::string, std::string>;
  using IITStudent  = Student<unsigned int, int>;

It also inherits from IStudent so that we can treat all students uniformly through the interface.
All interactions with the rest of the system occur through the IStudent interface.
*/
template<typename RollT, typename CourseCodeT>
class Student : public IStudent {
public:
    using roll_type        = RollT;
    using course_code_type = CourseCodeT;

private:
    // This data is Private: cannot be accessed by others. 
    // This is an implementaion of OOPS Principles (Data Hiding)
    std::string name;
    RollT roll;
    std::string branch;
    unsigned int startingYear;

    std::vector<CourseCodeT> currentCourses;

    struct PastCourse {
        CourseCodeT code;
        int grade; // 0 to 10
    };
    std::vector<PastCourse> pastCourses;

public:
    // Constructors

    Student(const std::string& name,
            const RollT& roll,
            const std::string& branch,
            unsigned int startingYear)
        : name(name),
          roll(roll),
          branch(branch),
          startingYear(startingYear)
    {}

    // Getters

    const std::string& getName() const {
        return name;
    }

    const RollT& getRoll() const {
        return roll;
    }

    const std::string& getBranch() const {
        return branch;
    }

    unsigned int getStartingYearConcrete() const {
        return startingYear;
    }

    const std::vector<CourseCodeT>& getCurrentCourses() const {
        return currentCourses;
    }

    const std::vector<PastCourse>& getPastCourses() const {
        return pastCourses;
    }

    // Mutators

    void addCurrentCourse(const CourseCodeT& course) {
        currentCourses.push_back(course);
    }

    void addPastCourse(const CourseCodeT& course, int grade) {
        pastCourses.push_back(PastCourse{course, grade});
    }

    // IStudent interface implementations:
    std::string getNameStr() const override {
        return name;
    }

    std::string getRollStr() const override {
        return toStringGeneric(roll);
    }

    std::string getBranchStr() const override {
        return branch;
    }

    unsigned int getStartingYear() const override {
        return startingYear;
    }

    void forEachPastCourse(
        const std::function<void(const std::string&, int)>& f
    ) const override {
        for (const auto& pc : pastCourses) {
            std::string courseStr = toStringGeneric(pc.code);
            f(courseStr, pc.grade);
        }
    }

    bool hasGradeAtLeast(const std::string& course,
                         int threshold) const override
    {
        if (threshold < 0) threshold = 0;
        for (const auto& pc : pastCourses) {
            std::string courseStr = toStringGeneric(pc.code);
            if (courseStr == course && pc.grade >= threshold) {
                return true;
            }
        }
        return false;
    }
};

#endif // STUDENT_H
