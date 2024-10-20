#ifndef CUST_DTO_H  
#define CUST_DTO_H

#define MAX_USERNAME_LEN 50
#define MAX_STATUS_LEN 9 // For storing status
#define MAX_NAME_LEN 128
#define MAX_PHONE_LEN 11

#define CUSTID_START 100000
#define CUST_STATUS_INACTIVE "inactive"
#define CUST_STATUS_ACTIVE "active"

typedef struct {
    long userId;    
    char name[MAX_NAME_LEN] ;
    char phone[MAX_PHONE_LEN];
    char status[MAX_STATUS_LEN];
    int balance;
} CustomerDTO;

#endif 

