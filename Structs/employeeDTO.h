#ifndef EMP_DTO_H  
#define EMP_DTO_H

#define MAX_USERNAME_LEN 50
#define MAX_STATUS_LEN 9 // For storing status
#define MAX_Name_LEN 128


typedef struct {
    char username[MAX_USERNAME_LEN];    
    char name[MAX_Name_LEN] ;
    char status[MAX_STATUS_LEN];
    char grade;
} EmployeeDTO;

#endif 

