#ifndef SESSION_H  
#define SESSION_H

#include <fcntl.h>   // For fcntl(), struct flock
#include <unistd.h>  

int create__new_session(const char *username, int clientSocket, pid_t pid);

int logout(const char *username);


#endif 