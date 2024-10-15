
#ifndef SESSION_IDEN_H  
#define SESSION_IDEN_H
#include<time.h>

#define CTIME_SIZE 26

typedef struct {
    int client_socket;         // Client's socket descriptor
    int pid;    
    time_t loginTime;

} Sess_Identifier;

#endif 
