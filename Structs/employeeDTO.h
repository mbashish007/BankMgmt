#ifndef EMP_DTO_H  
#define EMP_DTO_H

#define MAX_USERNAME_LEN 50
#define MAX_STATUS_LEN 9 // For storing status
#define MAX_NAME_LEN 128
#define MAX_ROLE_LEN 9
#define EMPID_START 300000

#define EMP_STATUS_INACTIVE "inactive"
#define EMP_STATUS_ACTIVE "active"

#define EMP_ROLE_EMP "employee"
#define EMP_ROLE_MGR "manager"

typedef struct {
    long empId;    
    char name[MAX_NAME_LEN] ;
    char status[MAX_STATUS_LEN];
    char grade;
    char role [MAX_ROLE_LEN];
} EmployeeDTO;

#endif 

