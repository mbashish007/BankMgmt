#ifndef USER_CRED_H  
#define USER_CRED_H

#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 65  // For storing SHA-256 hash

typedef struct {
    char username[MAX_USERNAME_LEN];    
    char passwordHash[MAX_PASSWORD_LEN]; 
} UserCred;

#endif 

