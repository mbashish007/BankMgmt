#ifndef ADMIN_DTO_H  
#define ADMIN_DTO_H

#define MAX_USERNAME_LEN 50
#define MAX_STATUS_LEN 9 // For storing status
#define MAX_ROLE_LEN 9
#define MAX_NAME_LEN 128


typedef struct {
    char username[MAX_USERNAME_LEN];    
    char name[MAX_NAME_LEN] ;
    char status[MAX_STATUS_LEN];
} AdminDTO;

#endif 

