#ifndef SESSION_H  
#define SESSION_H

#include <fcntl.h>   // For fcntl(), struct flock
#include <unistd.h>  

/**
 * success fd
 * -2 session already exists
 * error -1
 */
int create__new_session(const char *username, int clientSocket, pid_t pid);

/**
 * 0 success
 * 1 error
 */
int logout(const char *username);


#endif 