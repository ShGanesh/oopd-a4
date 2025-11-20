# ERP Student Management System (C++ OOPD Assignment)

This project implements a small ERP-style system for managing student records across different universities, focusing on:

- **Data abstraction** and **data hiding**
- **Templates** and **polymorphism**
- **Parallel sorting** (multi-threading)
- **Iterator-based views** without copying heavy data
- **Fast query structures** for grade-based recruitment queries


## 1. Single generic student class with different field types

**Assignment requirement**
> Write a single class that can be used to accommodate all the above cases irrespective of the type of fields for the above cases.  
> (Different roll number types: `string` / `unsigned int`; different course code types.)

**Implementation**
- File: `student.h`
  - `Student<RollT, CourseCodeT>` is the single generic student class.
  - Template parameters:
    - `RollT` – type of roll number (e.g., `std::string`, `unsigned int`)
    - `CourseCodeT` – type of course code (e.g., `std::string`, `int`)
  - Internal data members (`name`, `roll`, `branch`, `startingYear`, `currentCourses`, `pastCourses`) are all **private** (**data hiding**).

- File: `student.h` (`IStudent` interface)  
  - `class IStudent` is a **pure virtual base class**:
    - `virtual std::string getNameStr() const = 0;`
    - `virtual std::string getRollStr() const = 0;`
    - `virtual std::string getBranchStr() const = 0;`
    - `virtual unsigned int getStartingYear() const = 0;`
    - `virtual void forEachPastCourse(const std::function<void(const std::string&, int)>&) const = 0;`
  - This interface is the **abstraction boundary**: other modules work only with `IStudent` / `IStudentPtr`, not with the template internals.

- File: `erp_types.h`  
  - Concrete instantiations of the generic class:
    ```cpp
    using IIITStudent = Student<std::string, std::string>;
    using IITStudent  = Student<unsigned int, int>;
    using IStudentPtr = std::unique_ptr<IStudent>;
    ```
  - This covers:
    - Roll numbers as `std::string` (e.g., `MT22834`, `PhD22404`).
    - Roll numbers as `unsigned int` (e.g., `2025432`).
    - Course codes as `std::string` (e.g., `OOPD`, `801`, `ML`).
    - Course codes as `int` (e.g., `801`, `615`).

**Abstraction & hiding**
- All code outside `student.h` talks via `IStudent` and `IStudentPtr`.
- The actual field types (`RollT`, `CourseCodeT`) are hidden behind:
  - `getNameStr()`, `getRollStr()`, `getBranchStr()`, `getStartingYear()`
  - `forEachPastCourse(...)`
  - `hasGradeAtLeast(...)`


## 2. IIIT OOPD students, IIT course numbers, and mixed types

**Assignment requirement**
> Write a program that keeps track of students of the OOPD course of IIIT-Delhi, by creating objects of the above class/classes.  
> IIIT-Delhi students can take IIT-Delhi courses. IIT-Delhi course numbers are integers.  
> Ensure that your program can handle both types of course numbers.

**Implementation**
- File: `csv_loader.h`
  - `parseStudentRecord(...)` looks at the first CSV column (`Institute`):
    - `"IIIT"` → constructs an `IIITStudent`:
      ```cpp
      auto stu = std::make_unique<IIITStudent>(
          name,       // std::string
          rollStr,    // std::string
          branch,
          startingYear
      );
      ```
      - Current and past courses are parsed as `std::string` codes (e.g., `OOPD`, `801`).
    - `"IIT"` → constructs an `IITStudent`:
      ```cpp
      unsigned int rollNum = std::stoul(rollStr);
      auto stu = std::make_unique<IITStudent>(
          name,       // std::string
          rollNum,    // unsigned int
          branch,
          startingYear
      );
      ```
      - Current and past courses are parsed as `int` (e.g., `801`, `615`).
  - `loadStudentsFromCSV(filename)` loads all students (IIIT + IIT) into a single `std::vector<IStudentPtr>`, preserving insertion order.

- File: `student.h`
  - `Student::forEachPastCourse` and `Student::hasGradeAtLeast` convert any `CourseCodeT` to `std::string` using `toStringGeneric(...)`.  
  - This allows using the same query mechanism for:
    - string codes like `"OOPD"`, `"ML"`;
    - integer codes like `801`, but compared via their string representation.

- File: `main.cpp`
  - Menu options 5 and 6 ask for a **course code string** (e.g., `OOPD`, `801`) and a grade threshold.
  - These calls work uniformly for both IIIT and IIT students, because course codes are stringified at the abstraction boundary.

**Result**
- The program can:
  - track IIIT students enrolled in OOPD,
  - simultaneously handle IIT-style integer course codes,
  - mix both kinds of students in a single run.


## 3. CSV loading (~3000 students) and parallel sorting

**Assignment requirement**
> Read 3000 records from CSV; allow parallel sorting using at least two threads; no race conditions; log the time taken by each thread.

**Implementation**
- File: `csv_loader.h`
  - `loadStudentsFromCSV(const std::string& filename)`:
    - Opens the CSV.
    - Skips the header line.
    - For each subsequent line:
      - Splits into columns.
      - Calls `parseStudentRecord(...)` to create `IIITStudent` / `IITStudent`.
      - Pushes `std::unique_ptr<IStudent>` into `std::vector<IStudentPtr> students`.
    - Preserves **insertion order** from the file.
  - The code supports any number of students (3000+).

- File: `sorting.h`
  - `SortViews` holds **only indices**, not copies of students:
    ```cpp
    struct SortViews {
        std::vector<std::size_t> byName;
        std::vector<std::size_t> byRoll;
        std::list<std::size_t>   byNameList;
    };
    ```
  - `buildAndSortViews(const std::vector<IStudentPtr>& students)`:
    - Creates `byName` and `byRoll` as `[0, 1, ..., n-1]`.
    - Starts **two threads**:
      - Thread 1 sorts `byName` by `students[a]->getNameStr()`.
      - Thread 2 sorts `byRoll` by `students[a]->getRollStr()`.
    - Both threads:
      - treat `students` as **read-only**,
      - work on different index vectors (`byName` vs `byRoll`),
      - hence there are **no race conditions**.
    - After joining, it builds `byNameList` (a `std::list`) from `byName` to provide a different iterator type.
  - `logDuration(...)` logs time per thread:
    - Example output:
      ```
      [TIMER] Sort by name took 0 ms
      [TIMER] Sort by roll took 0 ms
      ```

---

## 4. Views without copying data, using different iterator types

**Assignment requirement**
> Without copying the whole data, show the records in insertion order and in sorted sequences. Use different types of iterators for each case.

**Implementation**
- No copying of student objects:
  - The underlying container of students is always:
    ```cpp
    std::vector<IStudentPtr> students;
    ```
  - Sorting is done on **index containers** (`std::vector<std::size_t>`, `std::list<std::size_t>`).

- File: `print_utils.h`
  - Insertion order view:
    ```cpp
    void printStudentsInsertionOrder(const std::vector<IStudentPtr>& students);
    ```
    - Iterates directly over `std::vector<IStudentPtr>`.
  - Sorted/indexed views:
    ```cpp
    template<typename IndexIter>
    void printStudentsByIndex(const std::vector<IStudentPtr>& students,
                              IndexIter begin, IndexIter end);
    ```
    - Works with any iterator type as long as it yields indices.

- File: `main.cpp`
  - Menu option 1:
    - Calls `printStudentsInsertionOrder(students)` → direct vector iteration.
  - Menu option 2:
    - Calls `printStudentsByIndex(students,
        views.byName.begin(), views.byName.end())` → `std::vector` iterators.
  - Menu option 3:
    - Calls `printStudentsByIndex(students,
        views.byRoll.begin(), views.byRoll.end())` → another `std::vector` iterator view.
  - Menu option 4:
    - Calls `printStudentsByIndex(students,
        views.byNameList.begin(), views.byNameList.end())` → `std::list` iterators.

**Result**

- Insertion order: direct `std::vector<IStudentPtr>` iteration.
- Sorted orders: “indexed view” using:
  - `std::vector<std::size_t>::iterator` (random-access iterators), and
  - `std::list<std::size_t>::iterator` (bidirectional iterators).
- At no point do we copy the actual student objects to sort them.


## 5. Fast queries for “grade ≥ 9 in a course”

**Assignment requirement**
> Design a data structure that allows fast retrieval of students with grade ≥ 9 in a specific course, without scanning all students linearly.

**Implementation**
- File: `course_index.h`
  - `CourseIndex`:
    ```cpp
    struct CourseIndex {
        std::array<std::vector<IStudent*>, 11> grades; // index 0..10
    };
    ```
    - For each course, maintain 11 buckets of students, one per grade.
  - `CourseIndexDB`:
    - `build(const std::vector<IStudentPtr>& students)`:
      - Iterates through `students` once.
      - For each student:
        - Calls `s->forEachPastCourse(...)` from `IStudent`.
        - For each `(courseStr, grade)` pair, inserts the student pointer into:
          ```cpp
          index_[courseStr].grades[grade].push_back(s);
          ```
    - `queryAtLeast(const std::string& course, int threshold)`:
      - Looks up the `CourseIndex` for `course`.
      - Clamps `threshold` to `[0, 10]`.
      - Iterates only over buckets `grades[threshold]` to `grades[10]` and concatenates results.
      - No full scan over all students is performed at query time.

- File: `main.cpp`
  - Menu option 5:
    - Asks for course (e.g., `OOPD`) and uses `threshold = 9`.
  - Menu option 6:
    - Asks for course and a custom grade threshold.
  - In both cases, calls `courseIndex.queryAtLeast(course, threshold)` and prints the resulting students.

**Result**

- A company can query:
  - “Students with grade ≥ 9 in `OOPD`”
  - “Students with grade ≥ 8 in `ML`”
  - etc.
- The system answers using a pre-built index, **not** a linear scan over all student records.

---

## Build and Run

### Build

```bash
make
```

This will compile main.cpp and all headers into an executable (`erp`).

### Run
```bash
./erp
```

You will be prompted for a CSV filename:
```text
Enter CSV filename (e.g. students_sample.csv): ./students_mixed.csv
```

Then, you can choose from the menu:

1. Show students (insertion order)
2. Show students sorted by name
3. Show students sorted by roll
4. Show students sorted by name (list iterator view)
5. Query: students with grade ≥ 9 in a course
6. Query: students with grade ≥ custom threshold in a course
0. Exit


## File Overview

* `main.cpp`
Entry point; loads CSV, builds sorted views and course index; implements the CLI menu.

* `student.h`
  * `IStudent` abstract interface.
  * `Student<RollT, CourseCodeT>` template class implementing polymorphic "students".

* `erp_types.h`: 
Type aliases for `IIITStudent`, `IITStudent`, and `IStudentPtr`.

* `csv_loader.h`: 
CSV parsing and loading into `std::vector<IStudentPtr>`.

* `sorting.h`: 
Parallel construction and sorting of index views (`SortViews`).

* `course_index.h`: 
Grade-based per-course index (`CourseIndexDB`) for fast queries.

* `print_utils.h`: 
Functions to print students in different views using different iterator types.

* `Makefile`: 
Simple build script for g++ with C++17 and -pthread.

* `README.md`: 
This documentation.