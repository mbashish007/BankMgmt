#define _GNU_SOURCE  // Enable GNU-specific features
#include "loan.h"
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../Structs/adminDTO.h"
#include "../../Structs/employeeDTO.h"
#include "../../Structs/customerDTO.h"
#include "../../Structs/txnDTO.h"
#include "../../Structs/loanDTO.h"
#include "../../utils/file/fileUtils.h"
#include "../../login/login.h"
#include <limits.h> //for PATH_MAX


#define ADM_DIR  "./data/users/admin/"
#define CUST_DIR  "./data/users/customer/"
#define EMP_DIR  "./data/users/employee/"
#define MGR_DIR  "./data/users/manager/"
#define TXN_PATH  "./data/transactions/transactions"
#define LOAN_PATH  "./data/loans/loans"

#define MAX_FILES 1024

int createLoan(LoanDTO *loan) {
    char path[256];
    struct flock lock;
    struct stat st; //for file size 
    ssize_t file_size;
    long loanCount;

    memset(&lock, 0, sizeof(lock));
    memset(&st, 0, sizeof(st));

    snprintf(path, sizeof(path), "%s", LOAN_PATH);
 

    int fd = open(path, O_WRONLY);
    if (fd == -1) 
    {
        if (errno == ENOENT) {  // File does not exist
           
            
            // Open file in create mode with read and write permissions
            fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
            if (fd == -1 && errno != EEXIST) {  // If open with O_CREAT also fails
                perror("Error creating file");
                return -1;
            }
        } 
        else {
            perror("Unable to open Loan  file");
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

    // Calculate the number of structs in the file
    loanCount = file_size / sizeof(LoanDTO);
    loan->loanId = loanCount;
    
    lseek(fd, 0, SEEK_END);

    if (write(fd, loan, sizeof(LoanDTO)) == -1) {
        perror("Failed to add loan");
        unlock(fd, &lock);
        close(fd);
        return -1;
    }
    
    // Unlock and close file
    unlock(fd, &lock);
    close(fd);
    return 0;
}

/**
 * 0 success
 * -1 error
 * -2 invalid argument for id
 */
int get_loan_data(long loanId, LoanDTO *loan) {
    char path[256] ;
    struct flock lock;

    memset(&lock, 0, sizeof(lock));

    snprintf(path, sizeof(path), "%s", LOAN_PATH);
 
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return -1; 
    }
    if (loanId < 0) {
        close(fd);
        return -2; // invalid id;
    }
    
    // Lock the file for reading
    if (arlock_w(fd, &lock, SEEK_SET, loanId*sizeof(LoanDTO), sizeof(LoanDTO)) == -1) {
        perror("Failed to lock file");
        close(fd);
        return -1;
    }

    lseek(fd, loanId*sizeof(LoanDTO), SEEK_SET);
    
    ssize_t bytesRead;
    if ((bytesRead = read(fd, loan, sizeof(LoanDTO))) < 0) {
            perror("Unable to read loan: ");
            unlock(fd,&lock);
            close(fd);
            return -1; 
    }
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
int read_loans_data(LoanDTO *loans) {
    char path[256] ;
    struct flock lock;

    struct stat st; //for file size 
    ssize_t file_size;
    memset(&lock, 0, sizeof(lock));
    memset(&st, 0, sizeof(st));


    snprintf(path, sizeof(path), "%s", LOAN_PATH);
    int fd = open(path, O_RDONLY);
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
    fstat(fd, &st);
    file_size = st.st_size;
    
    // Read LoanDTOs from the file
    ssize_t bytesRead;
    int count = 0;
    // long bytesReadTillNow = 0;
    lseek(fd, 0, SEEK_SET);
    while ( (bytesRead = read(fd, &loans[count++], sizeof(LoanDTO))) > 0 ) {
        
        if (count >= MAX_FILES ) {
            // If we've reached the max limit of loans
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

int read_loans_for_cust(LoanDTO *loans, long custId) {
    char path[256] ;
    struct flock lock;

    ssize_t file_size;
    memset(&lock, 0, sizeof(lock));


    snprintf(path, sizeof(path), "%s", LOAN_PATH);
    int fd = open(path, O_RDONLY);
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
    
    // Read LoanDTOs from the file
    ssize_t bytesRead;
    int count = 0;
    long bytesReadTillNow = 0;
    LoanDTO temp;
    memset(&temp, 0, sizeof(LoanDTO));
    // lseek(fd, -sizeof(LoanDTO), SEEK_END);

    while ((bytesRead = read(fd, &temp, sizeof(LoanDTO))) > 0) {
        
           if (temp.customerId == custId) {
            // Store the transaction in the loans array
            loans[count] = temp;
            count++;
        }
        if (count >= MAX_FILES ) {
            // If we've reached the max limit of loans
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

int read_loans_for_emp(LoanDTO *loans, long empId) {
    char path[256] ;
    struct flock lock;

    ssize_t file_size;
    memset(&lock, 0, sizeof(lock));


    snprintf(path, sizeof(path), "%s", LOAN_PATH);
    int fd = open(path, O_RDONLY);
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
    
    // Read LoanDTOs from the file
    ssize_t bytesRead;
    int count = 0;
    long bytesReadTillNow = 0;
    LoanDTO temp;
    memset(&temp, 0, sizeof(LoanDTO));
    // lseek(fd, -sizeof(LoanDTO), SEEK_END);

    while ((bytesRead = read(fd, &temp, sizeof(LoanDTO))) > 0) {
        
           if (temp.empId == empId) {
            // Store the transaction in the loans array
            loans[count] = temp;
            count++;
        }
        if (count >= MAX_FILES ) {
            // If we've reached the max limit of loans
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