#include "fileUtils.h"
#include <fcntl.h>   // For fcntl(), struct flock
#include <unistd.h>  
#include <stdio.h>

int mwlock_w(int fd, struct flock *lock) {
    
    lock->l_type=F_WRLCK;
    lock->l_whence= SEEK_SET;
    lock->l_start=0;
    lock->l_len = 0;
    lock->l_pid = getpid();
    if (fcntl(fd, F_SETLKW, lock) == -1) {
        perror("fcntl");
        return -1;  // Return error
    }

    return 0;  // Success
}

int awlock_w(int fd, struct flock *lock, short int l_whence, off_t start, off_t len) {
    
    lock->l_type=F_WRLCK;
    lock->l_whence= l_whence;
    lock->l_start=start;
    lock->l_len = len;
    lock->l_pid = getpid();
    if (fcntl(fd, F_SETLKW, lock) == -1) {
        perror("fcntl");
        return -1;  // Return error
    }

    return 0;  // Success
}

int mrlock_w(int fd, struct flock *lock) {
    
    lock->l_type=F_RDLCK;
    lock->l_whence= SEEK_SET;
    lock->l_start=0;
    lock->l_len = 0;
    lock->l_pid = getpid();
    if (fcntl(fd, F_SETLKW, lock) == -1) {
        perror("fcntl");
        return -1;  // Return error
    }

    return 0;  // Success
}

int arlock_w(int fd, struct flock *lock, short int l_whence, off_t start, off_t len) {
    
    lock->l_type=F_RDLCK;
    lock->l_whence= l_whence;
    lock->l_start=start;
    lock->l_len = len;
    lock->l_pid = getpid();
    if (fcntl(fd, F_SETLKW, lock) == -1) {
        perror("fcntl");
        return -1;  // Return error
    }

    return 0;  // Success
}

int unlock(int fd, struct flock *lock) {
    
    lock->l_type=F_UNLCK;
    if (fcntl(fd, F_SETLKW, lock) == -1) {
        perror("fcntl");
        return -1;  // Return error
    }

    return 0;  // Success
}