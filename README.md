# 🧑‍🎓 Student Record System (C)

A simple console-based **Student Record Management System** written in C.  
This project helps beginners understand **structures**, **file handling**, and basic menu-driven programs.

## 📌 Features
- Add new student (Roll No, Name, Marks)
- Display all student records
- Search student by roll number
- Delete a student record
- Rejects invalid numeric input, overlong names, and duplicate roll numbers
- Stores records in the binary file `students.dat` in the program's working directory
- Reports unreadable records and file-operation failures; failed deletions preserve the original file

## ▶️ Build and Run

Compile with GCC or another C11-compatible compiler:

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic Record.c -o student-record-system
```

Run the generated executable:

```sh
./student-record-system
```

On Windows, run `.\student-record-system.exe` from PowerShell or Command Prompt.

## 🛠️ Concepts Used
- Structures (`struct`)
- File Handling (`fopen`, `fread`, `fwrite`)
- Input validation
- Functions
- Loops & conditionals

