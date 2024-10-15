#ifndef FILE_UTIL_H  
#define FILE_UTIL_H

#include <fcntl.h>   // For fcntl(), struct flock
#include <unistd.h>  
/**
 * Mandatory File Write Lock
 */
int mwlock_w(int fd, struct flock *lock); 

/**
 * Mandatory File Read Lock 
 */

int mrlock_w(int fd, struct flock *lock);

/**
 * File Unlock
 */
int unlock(int fd, struct flock *lock) ;

#endif 