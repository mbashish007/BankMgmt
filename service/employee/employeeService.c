#define _GNU_SOURCE  // Enable GNU-specific features
#include "employeeService.h"
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "../../Structs/adminDTO.h"
#include "../../Structs/employeeDTO.h"
#include "../../Structs/customerDTO.h"
#include "../../utils/file/fileUtils.h"
#include "../../login/login.h"
#include <limits.h> //for PATH_MAX


#define ADM_DIR  "./data/users/admin/"
#define CUST_DIR  "./data/users/customer/"
#define EMP_DIR  "./data/users/employee/"
#define MGR_DIR  "./data/users/manager/"



// Function to list files in a directory and fill the provided array
int list_directory_files(const char* dir_path, char (*filenames)[MAX_FILENAME_LEN]) {
    DIR* dir;
    struct dirent* entry;
    int file_count = 0;
    printf("%s", dir_path);

    char cwd[256];
if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("Current working directory: %s\n", cwd);
} else {
    perror("getcwd() error");
}
    // Open the directory
    dir = opendir(dir_path);
    if (dir == NULL) {
        perror("Error opening directory");
        return -1;
    }

    // Read directory entries
    while ((entry = readdir(dir)) != NULL) {
        // Skip special directories "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Check if we have space left in the array
        if (file_count < MAX_FILES) {
            strncpy(filenames[file_count], entry->d_name, MAX_FILENAME_LEN - 1);
            filenames[file_count][MAX_FILENAME_LEN - 1] = '\0'; // Ensure null-termination
            file_count++;
        } else {
            // More files than we can handle in the array
            closedir(dir);
            return 1;
        }
    }

    // Close the directory
    closedir(dir);

    // All files have been listed, and they fit in the array
    return 0;
}

/**
 * 0 success
 * -1 error
 * -2 invalid argument for id
 */
int get_employee_data(long empId, EmployeeDTO *employee) {
    char file_path[256] = "./data/users/employee/employee";
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    
    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return -1; 
    }
    if ((empId - EMPID_START)< 0) return -2; // invalid id;
    off_t offset = empId - EMPID_START;
    
    // Lock the file for writing
    if (arlock_w(fd, &lock, SEEK_SET, offset*sizeof(EmployeeDTO), sizeof(EmployeeDTO)) == -1) {
        perror("Failed to lock file");
        close(fd);
        return -1;
    }

    // Read EmployeeDTOs from the file
    ssize_t bytesRead;
    if ((bytesRead = read(fd, employee, sizeof(EmployeeDTO))) < 0) {
            perror("Unable to read employee: ");
            // If we've reached the max limit of employees
             unlock(fd,&lock);

            // Close the file
            close(fd);
            return -1; 
    }
    sleep(5);
    unlock(fd,&lock);

    // Close the file
    close(fd);
    return 0; 
}

/**
 * 0 more than 1024
 * >0 no of elements +1
 * -1 error
 */
int read_employees_data(EmployeeDTO *employees) {
    char file_path[256] = "./data/users/employee/employee";
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    
    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return -1; 
    }

    // Acquire a read lock
    if (mrlock_w(fd, &lock) == -1) {
        perror("Error acquiring read lock");
        close(fd);
        return -1; 
    }

    // Read EmployeeDTOs from the file
    ssize_t bytesRead;
    int count = 0;
    while ((bytesRead = read(fd, &employees[count], sizeof(EmployeeDTO))) > 0) {
        count++;
        if (count >= MAX_FILES) {
            // If we've reached the max limit of employees
             unlock(fd,&lock);

            // Close the file
            close(fd);
            return 0; 

        }
    }

    if (bytesRead < 0) {
        perror("Error reading file");
        close(fd);
        return -1; // Error during read
    }

    unlock(fd,&lock);

    // Close the file
    close(fd);
    return count+1; 
}