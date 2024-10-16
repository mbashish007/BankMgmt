#ifndef CUST_DTO_H  
#define CUST_DTO_H

#define MAX_USERNAME_LEN 50
#define MAX_STATUS_LEN 9 // For storing status
#define MAX_Name_LEN 128
#define MAX_PHONE_LEN 11



typedef struct {
    char username[MAX_USERNAME_LEN];    
    char name[MAX_Name_LEN] ;
    char phone[MAX_PHONE_LEN];
    char status[MAX_STATUS_LEN];
    int balance;
} CustomerDTO;

#endif 

