#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#endif

struct Student {
    int roll;
    char name[50];
    float marks;
};

static int readLine(const char *prompt, char *buffer, size_t size);
static int readInt(const char *prompt, int *value);
static int readFloat(const char *prompt, float *value);
static int readName(char *name, size_t size);
static int readStudent(FILE *fp, struct Student *student);
static int studentExists(int roll);
static int replaceFile(const char *source, const char *destination);
static void addStudent(void);
static void displayStudents(void);
static void searchStudent(void);
static void deleteStudent(void);

int main() {
    int choice;

    while (1) {
        printf("\n=== Student Record System ===\n");
        printf("1. Add Student\n");
        printf("2. Display All Students\n");
        printf("3. Search Student by Roll Number\n");
        printf("4. Delete Student Record\n");
        printf("5. Exit\n");
        if (!readInt("Enter your choice: ", &choice)) {
            break;
        }

        switch (choice) {
            case 1: addStudent(); break;
            case 2: displayStudents(); break;
            case 3: searchStudent(); break;
            case 4: deleteStudent(); break;
            case 5: exit(0);
            default: printf("Invalid choice! Please try again.\n");
        }
    }
    return 0;
}

static int readLine(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (fgets(buffer, (int)size, stdin) == NULL) {
        return 0;
    }

    char *newline = strchr(buffer, '\n');
    if (newline != NULL) {
        *newline = '\0';
    } else if (!feof(stdin)) {
        int character;
        while ((character = getchar()) != '\n' && character != EOF) {
        }
        return -1;
    }
    return 1;
}

static int readInt(const char *prompt, int *value) {
    char input[128];
    char *end;

    while (1) {
        int result = readLine(prompt, input, sizeof(input));
        if (result == 0) {
            return 0;
        }
        if (result < 0) {
            printf("Input is too long. Please try again.\n");
            continue;
        }

        errno = 0;
        long parsed = strtol(input, &end, 10);
        while (isspace((unsigned char)*end)) {
            end++;
        }
        if (end == input || *end != '\0' || errno == ERANGE ||
            parsed < INT_MIN || parsed > INT_MAX) {
            printf("Please enter a valid whole number.\n");
            continue;
        }

        *value = (int)parsed;
        return 1;
    }
}

static int readFloat(const char *prompt, float *value) {
    char input[128];
    char *end;

    while (1) {
        int result = readLine(prompt, input, sizeof(input));
        if (result == 0) {
            return 0;
        }
        if (result < 0) {
            printf("Input is too long. Please try again.\n");
            continue;
        }

        errno = 0;
        float parsed = strtof(input, &end);
        while (isspace((unsigned char)*end)) {
            end++;
        }
        if (end == input || *end != '\0' || errno == ERANGE || !isfinite(parsed)) {
            printf("Please enter a valid number.\n");
            continue;
        }

        *value = parsed;
        return 1;
    }
}

static int readName(char *name, size_t size) {
    while (1) {
        int result = readLine("Enter name: ", name, size);
        if (result == 0) {
            return 0;
        }
        if (result < 0) {
            printf("Name is too long. Please enter at most %zu characters.\n", size - 1);
            continue;
        }
        return 1;
    }
}

static int readStudent(FILE *fp, struct Student *student) {
    size_t bytes = fread(student, 1, sizeof(*student), fp);
    if (bytes == 0) {
        return ferror(fp) ? -1 : 0;
    }
    if (bytes != sizeof(*student) ||
        memchr(student->name, '\0', sizeof(student->name)) == NULL) {
        return -1;
    }
    return 1;
}

static int studentExists(int roll) {
    FILE *fp = fopen("students.dat", "rb");
    if (fp == NULL) {
        return errno == ENOENT ? 0 : -1;
    }

    struct Student student;
    int result;
    while ((result = readStudent(fp, &student)) == 1) {
        if (student.roll == roll) {
            fclose(fp);
            return 1;
        }
    }

    if (fclose(fp) != 0 || result < 0) {
        return -1;
    }
    return 0;
}

static int replaceFile(const char *source, const char *destination) {
#ifdef _WIN32
    return MoveFileExA(source, destination,
                       MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) ? 0 : -1;
#else
    return rename(source, destination);
#endif
}

// Function to add a new student
static void addStudent(void) {
    struct Student s;
    if (!readInt("Enter roll number: ", &s.roll) ||
        !readName(s.name, sizeof(s.name)) ||
        !readFloat("Enter marks: ", &s.marks)) {
        return;
    }

    int exists = studentExists(s.roll);
    if (exists < 0) {
        printf("Error reading student records.\n");
        return;
    }
    if (exists) {
        printf("A student with roll number %d already exists.\n", s.roll);
        return;
    }

    FILE *fp = fopen("students.dat", "ab");
    if (fp == NULL) {
        printf("Error opening student records for writing.\n");
        return;
    }

    int writeFailed = fwrite(&s, sizeof(s), 1, fp) != 1;
    int closeFailed = fclose(fp) != 0;
    if (writeFailed || closeFailed) {
        printf("Error saving the student record.\n");
        return;
    }
    printf("Student added successfully!\n");
}

// Function to display all students
static void displayStudents(void) {
    struct Student s;
    FILE *fp = fopen("students.dat", "rb");
    if (fp == NULL) {
        printf(errno == ENOENT ? "No records found.\n" : "Error opening student records.\n");
        return;
    }

    printf("\n--- Student Records ---\n");
    int result;
    while ((result = readStudent(fp, &s)) == 1) {
        printf("Roll: %d\nName: %s\nMarks: %.2f\n-------------------\n", s.roll, s.name, s.marks);
    }
    if (result < 0) {
        printf("Error: student records are unreadable or incomplete.\n");
    }
    if (fclose(fp) != 0) {
        printf("Error closing student records.\n");
    }
}

// Function to search for a student
static void searchStudent(void) {
    struct Student s;
    int roll, found = 0;
    FILE *fp = fopen("students.dat", "rb");
    if (fp == NULL) {
        printf(errno == ENOENT ? "No records found.\n" : "Error opening student records.\n");
        return;
    }

    if (!readInt("Enter roll number to search: ", &roll)) {
        fclose(fp);
        return;
    }

    int result;
    while ((result = readStudent(fp, &s)) == 1) {
        if (s.roll == roll) {
            printf("\nRecord Found:\n");
            printf("Roll: %d\nName: %s\nMarks: %.2f\n", s.roll, s.name, s.marks);
            found = 1;
            break;
        }
    }
    if (result < 0) {
        printf("Error: student records are unreadable or incomplete.\n");
    }
    if (!found)
        printf("No record found for roll number %d\n", roll);

    if (fclose(fp) != 0) {
        printf("Error closing student records.\n");
    }
}

// Function to delete a student record
static void deleteStudent(void) {
    struct Student s;
    int roll, found = 0;
    FILE *fp = fopen("students.dat", "rb");
    if (fp == NULL) {
        printf(errno == ENOENT ? "No records found.\n" : "Error opening student records.\n");
        return;
    }

    if (!readInt("Enter roll number to delete: ", &roll)) {
        fclose(fp);
        return;
    }

    FILE *temp = fopen("temp.dat", "wb");
    if (temp == NULL) {
        fclose(fp);
        printf("Error creating temporary records file.\n");
        return;
    }

    int result;
    int writeFailed = 0;
    while ((result = readStudent(fp, &s)) == 1) {
        if (s.roll == roll) {
            found = 1;
        } else if (fwrite(&s, sizeof(s), 1, temp) != 1) {
            writeFailed = 1;
            break;
        }
    }

    int sourceCloseFailed = fclose(fp) != 0;
    int tempCloseFailed = fclose(temp) != 0;
    if (sourceCloseFailed || tempCloseFailed || result < 0 || writeFailed) {
        remove("temp.dat");
        printf("Error processing student records; original records were not replaced.\n");
        return;
    }

    if (!found) {
        remove("temp.dat");
        printf("No record found for roll number %d\n", roll);
        return;
    }

    if (replaceFile("temp.dat", "students.dat") != 0) {
        remove("temp.dat");
        printf("Error replacing student records; the original records were preserved.\n");
        return;
    }

    printf("Record deleted successfully!\n");
}