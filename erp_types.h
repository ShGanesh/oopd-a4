#ifndef ERP_TYPES_H
#define ERP_TYPES_H

#include <memory>
#include "student.h"

// IIIT-Delhi:
//   - RollNumber: string  (MT25003, PhD25033, ...)
//   - CourseCode: std::string   ("OOPD", "801", "DSA", ...)
using IIITStudent = Student<std::string, std::string>;

// IIT-Delhi:
//   - RollNumber: std::string   ("2025432", ...)
//   - CourseCode: int           (615, 601, 701, 802, ...)
using IITStudent = Student<unsigned int, int>;

// Convenience alias for the polymorphic handle
using IStudentPtr = std::unique_ptr<IStudent>;

#endif // ERP_TYPES_H
