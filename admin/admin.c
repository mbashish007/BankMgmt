#define _GNU_SOURCE  // Enable GNU-specific features
#include "admin.h"
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include "../Structs/adminDTO.h"
#include "../Structs/employeeDTO.h"
#include "../Structs/customerDTO.h"
#include "../utils/file/fileUtils.h"
#include "../login/login.h"
#include <limits.h> //for PATH_MAX


#define ADM_DIR  "./data/users/admin/"
#define CUST_DIR  "./data/users/customer/"
#define EMP_DIR  "./data/users/employee/"
#define MGR_DIR  "./data/users/manager/"

#define STATUS_ACTIVE "active"
#define STATUS_INACTIVE "inactive"

int getAdmin(char* username, AdminDTO* admin) {
    char path[256];
    struct flock lock;
    snprintf(path, sizeof(path), "%s%s", ADM_DIR,username);

    int fd = open(path, O_RDONLY);
    if(fd<0) {
        perror("admin not exist\n");
        return -1;
    }

    // AdminDTO admin;
    mrlock_w(fd, &lock);
    if(read(fd, admin, sizeof(AdminDTO)) != sizeof(AdminDTO)) {
        perror("ERROR reading admin info\n");
        unlock(fd, &lock);
        close(fd);
        return -1;
    }
    unlock(fd,&lock);
    close(fd);
    return 0;
}

// Function to add a new Admin user
/**
 * return 0 success
 * return -1 error
 * return -2 user with username already exists
 */
int add_new_admin(AdminDTO *admin) {
    char path[256];
    struct flock lock;
    snprintf(path, sizeof(path), "%s%s", ADM_DIR,admin->username);
    
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (fd == -1) {
        perror("Failed to create admin file");
        return -2;
    }

    // Lock the file for writing
    if (mwlock_w(fd, &lock) == -1) {
        perror("Failed to lock file");
        close(fd);
        return -1;
    }

    if (write(fd, admin, sizeof(AdminDTO)) == -1) {
        perror("Failed to write admin data");
        unlock(fd, &lock);
        close(fd);
        return -1;
    }

    if(add_user(admin->username, admin->username)==-1){
        unlock(fd, &lock);
        close(fd);
        if(unlink(path)==0) {
            perror("Unlink Failed");
        }
        return -1;
    }
    printf("succefully added admin\n");
    // Unlock and close file
    unlock(fd, &lock);
    close(fd);
    return 0;
}

// Function to add a new Bank Employee
/**
 * return 0 success
 * return -1 error
 * return -2 user with username already exists
 */
int add_new_employee(EmployeeDTO *employee) {
    char path[256];
    char mgrpath[256];
    struct flock lock;
    struct stat st; //for file size 
    ssize_t file_size;
    long empCount;

    memset(&lock, 0, sizeof(lock));
    memset(&st, 0, sizeof(st));

    snprintf(path, sizeof(path), "%s%s", EMP_DIR, "employee");
    // snprintf(mgrpath, sizeof(path), "%s%s", MGR_DIR, employee->username);
    // int fd1 = open(path, O_RDONLY );
    // if (fd1 > 0) {
    //     perror("employee with username already exists");
    //     close(fd1);
    //     return -2;
    // }

    int fd = open(path, O_WRONLY);
    if (fd == -1) 
    {
        if (errno == ENOENT) {  // File does not exist
           
            
            // Open file in create mode with read and write permissions
            fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
            if (fd == -1) {  // If open with O_CREAT also fails
                perror("Error creating file");
                return -1;
            }
        } 
        else {
            perror("Unable to open employee file");
        return -1;
        }
    }

    // Lock the file for writing
    if (awlock_w(fd, &lock, SEEK_END, 0, 0) == -1) {
        perror("Failed to lock file");
        close(fd);
        return -1;
    }
    fstat(fd,&st);
    
    file_size = st.st_size;
    printf("filesidze = %ld", file_size);

    // Calculate the number of structs in the file
    empCount = file_size / sizeof(EmployeeDTO);
    printf("empcount = %ld", empCount);
    employee->empId = EMPID_START + empCount;
    printf("employeeId = %ld",employee->empId);
    lseek(fd, 0, SEEK_END);
    if (write(fd, employee, sizeof(EmployeeDTO)) == -1) {
        perror("Failed to add employee.");
        unlock(fd, &lock);
        close(fd);
        return -1;
    }
    char empid_char[MAX_USERNAME_LEN];
    snprintf(empid_char, MAX_USERNAME_LEN, "%ld", employee->empId);
    if(add_user(empid_char, empid_char)==-1){
        // unlock(fd, &lock);
        // close(fd);
       ftruncate(fd, file_size);// delete last addition

        // if(unlink(path)==0) {
        //     perror("Unlink Failed");
        // }
        return -1;
    }
    // Unlock and close file
    unlock(fd, &lock);
    close(fd);
    return 0;
}


// Function to modify employee details
int modify_employee_details(long empid, EmployeeDTO *newEmp) {
    char path[256];
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    // memset(&st, 0, sizeof(st));

    snprintf(path, sizeof(path), "%s%s", EMP_DIR, "employee");

    int fd = open(path, O_RDWR);
    if (fd == -1) {
        perror("Failed to open employee file");
        return -1;
    }
    off_t offset = empid - EMPID_START;
    
    // Lock the file for writing
    if (awlock_w(fd, &lock, SEEK_SET, offset*sizeof(EmployeeDTO), sizeof(EmployeeDTO)) == -1) {
        perror("Failed to lock file");
        close(fd);
        return -1;
    }
    
    EmployeeDTO oldEmp;
    lseek(fd, offset*sizeof(EmployeeDTO),SEEK_SET);
    int rbytes = read(fd, &oldEmp, sizeof(EmployeeDTO));

    if (rbytes != sizeof(EmployeeDTO)) {
        perror("Error reading Employee data");
        unlock(fd, &lock);
        close(fd);
        return -1;
    }
    strcpy(newEmp->status, oldEmp.status); //copying existing status
    lseek(fd,-sizeof(EmployeeDTO),SEEK_CUR);//start of file
    if (write(fd, newEmp, sizeof(EmployeeDTO)) == -1) {
        perror("Failed to update employee data");
        unlock(fd, &lock);
        close(fd);
        return -1;
    }
    sleep(10);
    // Unlock and close file
    unlock(fd, &lock);
    close(fd);
    return 0;
}

// Function to modify employee details
int activate_deactivate_employee(long empid) {
    char path[256];
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    // memset(&st, 0, sizeof(st));

    snprintf(path, sizeof(path), "%s%s", EMP_DIR, "employee");

    int fd = open(path, O_RDWR);
    if (fd == -1) {
        perror("Failed to open employee file");
        return -1;
    }
    off_t offset = empid - EMPID_START;
    
    // Lock the file for writing
    if (awlock_w(fd, &lock, SEEK_SET, offset*sizeof(EmployeeDTO), sizeof(EmployeeDTO)) == -1) {
        perror("Failed to lock file");
        close(fd);
        return -1;
    }
    
    EmployeeDTO oldEmp;
    lseek(fd, offset*sizeof(EmployeeDTO),SEEK_SET);
    int rbytes = read(fd, &oldEmp, sizeof(EmployeeDTO));

    if (rbytes != sizeof(EmployeeDTO)) {
        perror("Error reading Employee data");
        unlock(fd, &lock);
        close(fd);
        return -1;
    }
    if (strcmp(oldEmp.status, STATUS_ACTIVE) == 0) {
        // If status is "active", set to "inactive"
        strncpy(oldEmp.status, STATUS_INACTIVE, sizeof(STATUS_INACTIVE) - 1);
        oldEmp.status[sizeof(STATUS_INACTIVE) - 1] = '\0';
    } else if (strcmp(oldEmp.status, STATUS_INACTIVE) == 0) {
        // If status is "inactive", set to "active"
        strncpy(oldEmp.status, STATUS_ACTIVE, sizeof(STATUS_ACTIVE) - 1);
        oldEmp.status[sizeof(STATUS_ACTIVE) - 1] = '\0';
    }
    lseek(fd,-sizeof(EmployeeDTO),SEEK_CUR);//start of file
    if (write(fd, &oldEmp, sizeof(EmployeeDTO)) == -1) {
        perror("Failed to update employee data");
        unlock(fd, &lock);
        close(fd);
        return -1;
    }
    sleep(5);
    // Unlock and close file
    unlock(fd, &lock);
    close(fd);
    return 0;
}

const char* findempRole(const char *username) {
    char src_path[256];

    // Check if user is an employee
    snprintf(src_path, sizeof(src_path), "%s%s", EMP_DIR, username);
    int fd = open(src_path, O_RDONLY);
    if (fd != -1) {
        close(fd);  // Always close the file descriptor
        return "employee";
    }

    // Check if user is a manager
    bzero(src_path, 256);  // Clear buffer
    snprintf(src_path, sizeof(src_path), "%s%s", MGR_DIR, username);
    fd = open(src_path, O_RDONLY);
    if (fd != -1) {
        close(fd);  // Always close the file descriptor
        return "manager";
    }

    return "null";  // User not found in either role
}


// Function to manage user roles
int manage_user_roles(const char *username,  const char *newRole) {
    char src_path[256];
    char dest_path[256];
    const char *oldRole = findempRole(username);

    snprintf(src_path, sizeof(src_path), "./data/users/%s/%s", oldRole,username);
    snprintf(dest_path, sizeof(dest_path), "./data/users/%s/%s", newRole,username);
    printf("%s\n",src_path);
    printf("%s\n",dest_path);
    struct flock lock;

    int fd = open(src_path, O_RDWR);
    if (fd == -1) {
        perror("Failed to open file");
        return -1;
    }

    // Lock the file for writing
    if (mwlock_w(fd, &lock) == -1) {
        perror("Failed to lock file");
        close(fd);
        return -1;
    }

     char cwd[PATH_MAX];

    // Get the current working directory
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    } else {
        perror("getcwd() error");
    }

   if (renameat2(AT_FDCWD, src_path, AT_FDCWD, dest_path, RENAME_NOREPLACE) == -1) {
        if (errno == EEXIST) {
            fprintf(stderr, "Error: Destination file '%s' already exists.\n", dest_path);
        } else {
            perror("Error moving the file");
        }

        unlock(fd, &lock);  // Unlock before exiting
        close(fd);
        return -1;
    }
    // Unlock and close file
    unlock(fd, &lock);
    close(fd);
    return 0;
}



