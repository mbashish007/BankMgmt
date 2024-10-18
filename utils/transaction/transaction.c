//3 functions

#define _GNU_SOURCE  // Enable GNU-specific features
#include "transaction.h"
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
#include "../../utils/file/fileUtils.h"
#include "../../login/login.h"
#include <limits.h> //for PATH_MAX


#define ADM_DIR  "./data/users/admin/"
#define CUST_DIR  "./data/users/customer/"
#define EMP_DIR  "./data/users/employee/"
#define MGR_DIR  "./data/users/manager/"
#define TXN_PATH  "./data/txns/transactions"

int writeTXN(TxnDTO *txn) {
    char path[256];
    struct flock lock;
    struct stat st; //for file size 
    ssize_t file_size;
    long txnCount;

    memset(&lock, 0, sizeof(lock));
    memset(&st, 0, sizeof(st));

    snprintf(path, sizeof(path), "%s", TXN_PATH);
 

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
            perror("Unable to open cust file");
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
    // printf("filesidze = %ld", file_size);

    // Calculate the number of structs in the file
    txnCount = file_size / sizeof(TxnDTO);
    txn->txnId = txnCount;
    // txn->timestamp = time(NULL);
    
    lseek(fd, 0, SEEK_END);

    if (write(fd, txn, sizeof(TxnDTO)) == -1) {
        perror("Failed to add txn");
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
int get_txn_data(long txnId, TxnDTO *txn) {
    char path[256] ;
    struct flock lock;

    memset(&lock, 0, sizeof(lock));

    snprintf(path, sizeof(path), "%s", TXN_PATH);
 
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return -1; 
    }
    if (txnId < 0) return -2; // invalid id;
    
    // Lock the file for reading
    if (arlock_w(fd, &lock, SEEK_SET, txnId*sizeof(TxnDTO), sizeof(TxnDTO)) == -1) {
        perror("Failed to lock file");
        close(fd);
        return -1;
    }

    lseek(fd, txnId*sizeof(TxnDTO), SEEK_SET);
    
    ssize_t bytesRead;
    if ((bytesRead = read(fd, txn, sizeof(TxnDTO))) < 0) {
            perror("Unable to read txn: ");
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
int read_txns_data(TxnDTO *txns) {
    char path[256] ;
    struct flock lock;

    struct stat st; //for file size 
    ssize_t file_size;
    memset(&lock, 0, sizeof(lock));
    memset(&st, 0, sizeof(st));


    snprintf(path, sizeof(path), "%s", TXN_PATH);
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
    
    // Read TxnDTOs from the file
    ssize_t bytesRead;
    int count = 0;
    long bytesReadTillNow = 0;
    lseek(fd, -sizeof(TxnDTO), SEEK_END);
    while ((bytesReadTillNow < file_size) && (bytesRead = read(fd, &txns[count], sizeof(TxnDTO))) > 0 ) {
        
        count++; 
        bytesReadTillNow += sizeof(TxnDTO);
        if (count >= MAX_FILES ) {
            // If we've reached the max limit of txns
             unlock(fd,&lock);

            // Close the file
            close(fd);
            return 0; 

        }
        lseek(fd, -2*sizeof(TxnDTO), SEEK_CUR);
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

int read_txns_for_cust(TxnDTO *txns, long custId) {
    char path[256] ;
    struct flock lock;

    struct stat st; //for file size 
    ssize_t file_size;
    memset(&lock, 0, sizeof(lock));
    memset(&st, 0, sizeof(st));


    snprintf(path, sizeof(path), "%s", TXN_PATH);
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

    // Read TxnDTOs from the file
    ssize_t bytesRead;
    int count = 0;
    long bytesReadTillNow = 0;
    TxnDTO temp;
    memset(&temp, 0, sizeof(TxnDTO));
    lseek(fd, -sizeof(TxnDTO), SEEK_END);

    while ((bytesReadTillNow < file_size) && (bytesRead = read(fd, &temp, sizeof(TxnDTO))) > 0) {
        
           if (temp.fromCust == custId || temp.toCust == custId) {
            // Store the transaction in the txns array
            txns[count] = temp;
            count++;
        }

        bytesReadTillNow += sizeof(TxnDTO);
        if (count >= MAX_FILES ) {
            // If we've reached the max limit of txns
             unlock(fd,&lock);

            // Close the file
            close(fd);
            return 0; 

        }
        lseek(fd, -2*sizeof(TxnDTO), SEEK_CUR);
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