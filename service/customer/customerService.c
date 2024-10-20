#define _GNU_SOURCE  // Enable GNU-specific features
#include "customerService.h"
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
#include "../../Structs/txnDTO.h"
#include "../../utils/file/fileUtils.h"
#include "../../utils/transaction/transaction.h"
#include "../../login/login.h"
#include <limits.h> //for PATH_MAX


#define ADM_DIR  "./data/users/admin/"
#define CUST_DIR  "./data/users/customer/"
#define EMP_DIR  "./data/users/employee/"
#define MGR_DIR  "./data/users/manager/"

#define STATUS_ACTIVE "active"
#define STATUS_INACTIVE "inactive"

//cpy customer
void copy_customer(CustomerDTO* dest, const CustomerDTO* src) {
    memcpy(dest, src, sizeof(CustomerDTO));
}

int add_new_customer(CustomerDTO *cust) {
    char path[256];
    struct flock lock;
    struct stat st; //for file size 
    ssize_t file_size;
    long custCount;

    memset(&lock, 0, sizeof(lock));
    memset(&st, 0, sizeof(st));

    snprintf(path, sizeof(path), "%s%s", CUST_DIR, "customer");
 

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
    custCount = file_size / sizeof(CustomerDTO);
    cust->userId = CUSTID_START + custCount;

    lseek(fd, 0, SEEK_END);
    if (write(fd, cust, sizeof(CustomerDTO)) == -1) {
        perror("Failed to add cust.");
        unlock(fd, &lock);
        close(fd);
        return -1;
    }
    //convert long to string
    char custId_char[MAX_USERNAME_LEN];
    snprintf(custId_char, MAX_USERNAME_LEN, "%ld", cust->userId);

    if(add_user(custId_char, custId_char)==-1){
        ftruncate(fd, file_size);// undo write
        return -1;
    }
    // Unlock and close file
    unlock(fd, &lock);
    close(fd);
    return 0;
}

// Function to modify customer details
/**
 * returns
 * -2 invalid ID
 * -1 error
 * 0 success
 */
int modify_customer_details(long custId, CustomerDTO *newCust) {
    char path[256];
    struct flock lock;
    snprintf(path, sizeof(path), "%s%s", CUST_DIR, "customer");
    memset(&lock, 0, sizeof(lock));
    
    int fd = open(path, O_RDWR);
    if (fd == -1) {
        perror("Failed to open customer file");
        return -1;
    }
    off_t offset = custId - CUSTID_START;
    if(offset<0) {
        return -2 ;//Invalid userId
    }
    // Lock the file for writing
   // Lock the file for writing
    if (awlock_w(fd, &lock, SEEK_SET, offset*sizeof(CustomerDTO), sizeof(CustomerDTO)) == -1) {
        perror("Failed to lock file");
        close(fd);
        return -1;
    }

    CustomerDTO oldCust;
    lseek(fd, offset*sizeof(CustomerDTO), SEEK_SET);
    int rbytes = read(fd, &oldCust, sizeof(CustomerDTO));

    if (rbytes != sizeof(CustomerDTO)) {
        perror("Error reading Customer data");
        unlock(fd, &lock);
        close(fd);
        return -1;
    }
    strcpy(newCust->status, oldCust.status); //copying existing status
    newCust->balance = oldCust.balance;
    lseek(fd,-sizeof(CustomerDTO),SEEK_CUR);//start of file

    if (write(fd, newCust, sizeof(CustomerDTO)) == -1) {
        perror("Failed to update customer data");
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
int get_customer_data(long custId, CustomerDTO *customer) {
    char file_path[256] = "./data/users/customer/customer";
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    
    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return -1; 
    }
    if ((custId - CUSTID_START)< 0) return -2; // invalid id;
    off_t offset = custId - CUSTID_START;
    
    // Lock the file for writing
    if (arlock_w(fd, &lock, SEEK_SET, offset*sizeof(CustomerDTO), sizeof(CustomerDTO)) == -1) {
        perror("Failed to lock file");
        close(fd);
        return -1;
    }
    lseek(fd, offset*sizeof(CustomerDTO), SEEK_SET);
    // Read CustomerDTOs from the file
    ssize_t bytesRead;
    if ((bytesRead = read(fd, customer, sizeof(CustomerDTO))) < 0) {
            perror("Unable to read customer: ");
            // If we've reached the max limit of customers
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
int read_customers_data(CustomerDTO *customers) {
    char file_path[256] = "./data/users/customer/customer";
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

    // Read CustomerDTOs from the file
    ssize_t bytesRead;
    int count = 0;
    while ((bytesRead = read(fd, &customers[count], sizeof(CustomerDTO))) > 0) {
        count++;
        if (count >= MAX_FILES) {
            // If we've reached the max limit of customers
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
    return count+1; 
}

/**
 * 0 success 
 * -1 error
 * 
 */
int depositMoney(long custId, CustomerDTO* customer, int damount) {
    char path[256];
    struct flock lock;
    snprintf(path, sizeof(path), "%s%s", CUST_DIR, "customer");
    memset(&lock, 0, sizeof(lock));
    
    int fd = open(path, O_RDWR);
    if (fd == -1) {
        perror("Failed to open customer file");
        return -1;
    }
    off_t offset = custId - CUSTID_START;
    if(offset<0) {
        return -2 ;//Invalid userId
    }
    // Lock the file for writing  
    if(awlock_w(fd, &lock, SEEK_SET, offset*sizeof(CustomerDTO), sizeof(CustomerDTO)) < 0) {
        //locking error
        close(fd);
        return -1;
    }

    lseek(fd, offset*sizeof(CustomerDTO), SEEK_SET);
    int rbytes = read(fd, customer, sizeof(CustomerDTO));

    if (rbytes != sizeof(CustomerDTO)) {
        perror("Error reading Customer data");
        unlock(fd, &lock);
        close(fd);
        return -1;
    }
    
    CustomerDTO custCopy ;
    copy_customer(&custCopy, customer);
    customer->balance += damount;
    

    
    lseek(fd,-sizeof(CustomerDTO),SEEK_CUR);//start of write

    if (write(fd, customer, sizeof(CustomerDTO)) == -1) {
        perror("Failed to update customer data");
        unlock(fd, &lock);
        close(fd);
        return -1;
    }
    TxnDTO txn; //body for txn
    memset(&txn, 0, sizeof(TxnDTO));
    txn.amount = damount;
    txn.fromCust = -1;
    txn.toCust = custId;
    txn.timestamp = time(NULL);

    if(writeTXN(&txn) < 0) {
        perror("Error creating txn: ");
        
         lseek(fd,-sizeof(CustomerDTO),SEEK_CUR);//start of write

        write(fd, &custCopy, sizeof(CustomerDTO));
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
 * -2 Invalid cust id
 */
int processLoanMoney(long custId, long loanId, CustomerDTO* customer, int damount) {
    char path[256];
    struct flock lock;
    snprintf(path, sizeof(path), "%s%s", CUST_DIR, "customer");
    memset(&lock, 0, sizeof(lock));
    
    int fd = open(path, O_RDWR);
    if (fd == -1) {
        perror("Failed to open customer file");
        return -1;
    }
    off_t offset = custId - CUSTID_START;
    if(offset<0) {
        return -2 ;//Invalid userId
    }
    // Lock the file for writing  
    if(awlock_w(fd, &lock, SEEK_SET, offset*sizeof(CustomerDTO), sizeof(CustomerDTO)) < 0) {
        //locking error
        close(fd);
        return -1;
    }

    lseek(fd, offset*sizeof(CustomerDTO), SEEK_SET);
    int rbytes = read(fd, customer, sizeof(CustomerDTO));

    if (rbytes != sizeof(CustomerDTO)) {
        perror("Error reading Customer data");
        unlock(fd, &lock);
        close(fd);
        return -1;
    }
    
    CustomerDTO custCopy ;
    copy_customer(&custCopy, customer);
    customer->balance += damount;
    

    
    lseek(fd,-sizeof(CustomerDTO),SEEK_CUR);//start of write

    if (write(fd, customer, sizeof(CustomerDTO)) == -1) {
        perror("Failed to update customer data");
        unlock(fd, &lock);
        close(fd);
        return -1;
    }
    TxnDTO txn; //body for txn
    memset(&txn, 0, sizeof(TxnDTO));
    txn.amount = damount;
    txn.fromCust = loanId;
    txn.toCust = custId;
    txn.timestamp = time(NULL);

    if(writeTXN(&txn) < 0) {
        perror("Error creating txn: ");
        
         lseek(fd,-sizeof(CustomerDTO),SEEK_CUR);//start of write

        write(fd, &custCopy, sizeof(CustomerDTO));
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
 * -3 insufficient balance
 */
int withdrawMoney(long custId, CustomerDTO* customer, int damount) {
    char path[256];
    struct flock lock;
    snprintf(path, sizeof(path), "%s%s", CUST_DIR, "customer");
    memset(&lock, 0, sizeof(lock));
    
    int fd = open(path, O_RDWR);
    if (fd == -1) {
        perror("Failed to open customer file");
        return -1;
    }

    off_t offset = custId - CUSTID_START;
    if(offset<0) {
        return -2 ;//Invalid userId
    }
    // Lock the file for writing  
    
       if(awlock_w(fd, &lock, SEEK_SET, offset*sizeof(CustomerDTO), sizeof(CustomerDTO)) < 0) {
        //locking error
        close(fd);
        return -1;
    }

    lseek(fd, offset*sizeof(CustomerDTO), SEEK_SET);
    int rbytes = read(fd, customer, sizeof(CustomerDTO));

    if (rbytes != sizeof(CustomerDTO)) {
        perror("Error reading Customer data");
        unlock(fd, &lock);
        close(fd);
        return -1;
    }
    
    CustomerDTO custCopy ;
    copy_customer(&custCopy, customer);

    if(customer->balance < damount) {
        unlock(fd, &lock);
        close(fd);
        return -3 ;// insufficient balance check 
    }
    customer->balance -= damount;
    

    
    lseek(fd,-sizeof(CustomerDTO),SEEK_CUR);//start of write

    if (write(fd, customer, sizeof(CustomerDTO)) == -1) {
        perror("Failed to update customer data");
        unlock(fd, &lock);
        close(fd);
        return -1;
    }
    TxnDTO txn; //body for txn
    memset(&txn, 0, sizeof(TxnDTO));
    txn.amount = damount;
    txn.fromCust = custId;
    txn.toCust = -1;
    txn.timestamp = time(NULL);

    if(writeTXN(&txn) < 0) {
        perror("Error creating txn: ");
        
         lseek(fd,-sizeof(CustomerDTO),SEEK_CUR);//start of write

        write(fd, &custCopy, sizeof(CustomerDTO));
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
 * -3 insufficient balance
 * -4 Inactive recepient
 */
int transferMoney(long fromCustId,long toCustId,  int damount) {
    char path[256];
    struct flock frmlock; //Initializing both locks
    struct flock tolock;


    snprintf(path, sizeof(path), "%s%s", CUST_DIR, "customer");
    memset(&frmlock, 0, sizeof(frmlock));
    memset(&tolock, 0, sizeof(tolock));

    
    int fd = open(path, O_RDWR);// open customer file

    if (fd == -1) {
        perror("Failed to open customer file");
        close(fd);
        return -1;
    }
    
    //calculating offsets
    off_t frmoffset = fromCustId - CUSTID_START;
    if(frmoffset<0) {
        close(fd);
        return -2 ;//Invalid userId
        
    }
    
    off_t tooffset = toCustId - CUSTID_START;
    if(tooffset<0) {
        close(fd);
        return -2 ;//Invalid userId
    }
    // Lock the file for writing 
    //always acquire the smallest customer id lock first 
    if(fromCustId<= toCustId) {
            if(awlock_w(fd, &frmlock, SEEK_SET, frmoffset*sizeof(CustomerDTO), sizeof(CustomerDTO)) < 0) {
                //locking error
                close(fd);
                return -1;
            }
            if(awlock_w(fd, &tolock, SEEK_SET, tooffset*sizeof(CustomerDTO), sizeof(CustomerDTO)) < 0) {
                //locking error
                close(fd);
                return -1;
            }
              
    }else {
            if(awlock_w(fd, &tolock, SEEK_SET, tooffset*sizeof(CustomerDTO), sizeof(CustomerDTO)) < 0) {
                //locking error
                close(fd);
                return -1;
            }
            if(awlock_w(fd, &frmlock, SEEK_SET, frmoffset*sizeof(CustomerDTO), sizeof(CustomerDTO)) < 0) {
                //locking error
                close(fd);
                return -1;
            }
    }

    //reading from customer
    CustomerDTO frmCustomer;
    lseek(fd, frmoffset*sizeof(CustomerDTO), SEEK_SET);
    int rbytes = read(fd, &frmCustomer, sizeof(CustomerDTO));

    if (rbytes != sizeof(CustomerDTO)) {
        perror("Error reading Customer data");
        unlock(fd, &frmlock);
        unlock(fd, &tolock);
        close(fd);
        return -1;
    }
    
    CustomerDTO frmCustCopy ;
    copy_customer(&frmCustCopy, &frmCustomer);

    if(frmCustomer.balance < damount) return -3 ;// insufficient balance check 
    
    //reading to customer
    CustomerDTO toCustomer;
    lseek(fd, tooffset*sizeof(CustomerDTO), SEEK_SET);
    rbytes = read(fd, &toCustomer, sizeof(CustomerDTO));

    if (rbytes != sizeof(CustomerDTO)) {
        perror("Error reading Customer data");
        unlock(fd, &frmlock);
        unlock(fd, &tolock);
        close(fd);
        return -1;
    }
    
    CustomerDTO toCustCopy ;
    copy_customer(&toCustCopy, &toCustomer);
    
    if(strcmp(toCustomer.status, STATUS_INACTIVE ) == 0) {
        unlock(fd, &frmlock);
        unlock(fd, &tolock);
        close(fd);
        return -4;

    }
    //subtract from first add to second
    frmCustomer.balance -= damount;
    toCustomer.balance += damount;

    
    lseek(fd, frmoffset*sizeof(CustomerDTO), SEEK_SET);

    if (write(fd, &frmCustomer, sizeof(CustomerDTO)) == -1) {
       perror("Write error to cust: ");
       unlock(fd, &frmlock);
        unlock(fd, &tolock);
        close(fd);
        return -1;
    }

    lseek(fd, tooffset*sizeof(CustomerDTO), SEEK_SET);
    if (write(fd, &toCustomer, sizeof(CustomerDTO)) == -1) {
       perror("Write error to cust: ");
       unlock(fd, &frmlock);
        unlock(fd, &tolock);
        close(fd);
        return -1;
    }

    TxnDTO txn; //body for txn
    memset(&txn, 0, sizeof(TxnDTO));
    txn.amount = damount;
    txn.fromCust = frmCustomer.userId;
    txn.toCust = toCustomer.userId;
    txn.timestamp = time(NULL);

    if(writeTXN(&txn) < 0) {
        perror("Error creating txn: ");
        

        //rewrite old from customer data
        lseek(fd, frmoffset*sizeof(CustomerDTO), SEEK_SET);
        write(fd, &frmCustCopy, sizeof(CustomerDTO));
        lseek(fd, tooffset*sizeof(CustomerDTO), SEEK_SET);
        write(fd, &toCustCopy, sizeof(CustomerDTO));
            unlock(fd, &frmlock);
            unlock(fd, &tolock);
            close(fd);
            return -1;

    }


    // Unlock and close file
    unlock(fd, &frmlock);
    unlock(fd, &tolock);
    close(fd);
    return 0;
}

// Function to modify employee details
int activate_deactivate_customer(long custId) {
    char path[256];
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    // memset(&st, 0, sizeof(st));

    snprintf(path, sizeof(path), "%s%s", CUST_DIR, "customer");

    int fd = open(path, O_RDWR);
    if (fd == -1) {
        perror("Failed to open customer file");
        return -1;
    }
    off_t offset = custId - CUSTID_START;
    
    // Lock the file for writing
    if (awlock_w(fd, &lock, SEEK_SET, offset*sizeof(CustomerDTO), sizeof(CustomerDTO)) == -1) {
        perror("Failed to lock file");
        close(fd);
        return -1;
    }
    
    CustomerDTO cust;
    lseek(fd, offset*sizeof(CustomerDTO),SEEK_SET);
    int rbytes = read(fd, &cust, sizeof(CustomerDTO));

    if (rbytes != sizeof(CustomerDTO)) {
        perror("Error reading customer data data");
        unlock(fd, &lock);
        close(fd);
        return -1;
    }
    if (strcmp(cust.status, CUST_STATUS_ACTIVE) == 0) {
        // If status is "active", set to "inactive"
        strncpy(cust.status, CUST_STATUS_INACTIVE, sizeof(CUST_STATUS_INACTIVE) - 1);
        cust.status[sizeof(STATUS_INACTIVE) - 1] = '\0';
    } else if (strcmp(cust.status, CUST_STATUS_INACTIVE) == 0) {
        // If status is "inactive", set to "active"
        strncpy(cust.status, CUST_STATUS_ACTIVE, sizeof(CUST_STATUS_ACTIVE) - 1);
        cust.status[sizeof(CUST_STATUS_ACTIVE) - 1] = '\0';
    }
    lseek(fd,-sizeof(CustomerDTO),SEEK_CUR);//start of file
    if (write(fd, &cust, sizeof(CustomerDTO)) == -1) {
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
