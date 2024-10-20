#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <time.h>
#include <stdio.h> 
#include "../utils/file/fileUtils.h" // For perror
#include "../login/login.h"
#include "../Structs/sessionIdentifier.h"

#define SESSION_DIR "./data/sessions/"
#define BUFFER_SIZE 256

/**
 * success fd
 * -2 session already exists
 * error -1
 */
// Function to create a session file for the user
int create__new_session(const char *username, int clientSocket, pid_t pid) {
    struct flock lock;
   
    char session_file[BUFFER_SIZE];
    char session_data[CTIME_SIZE];

    
    // Build session file path
    snprintf(session_file, sizeof(session_file), "%s%s", SESSION_DIR, username);

    // Open the session file with exclusive creation (O_EXCL)
    int fd = open(session_file, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (fd < 0) {
        if (errno == EEXIST) {
          close(fd);
          return -2;
        } 

            perror("Error creating session file");
        
        return -1;
    }

    //lock session file
    mwlock_w(fd, &lock);
    
    time_t now = time(NULL);
    //add session identifiers
    Sess_Identifier sess;
    sess.client_socket = clientSocket;
    sess.pid = pid;
    sess.loginTime = now;
   
 
    if (write(fd, &sess, sizeof(Sess_Identifier)) < 0) {
        perror("Error writing session data");
        unlock(fd, &lock);
        close(fd);
        return -1;
    }
    unlock(fd, &lock);
    return fd;  // Return file descriptor of the session
}

// Function to remove the session file 
/**
 * 0 success
 * 1 error
 */
int logout(const char *username) {
    char session_file[BUFFER_SIZE];
    struct flock lock;
    // Build session file path
    snprintf(session_file, sizeof(session_file), "%s%s", SESSION_DIR, username);

    // Open the session file
    int fd = open(session_file, O_RDONLY);
    if (fd < 0) {
        perror("Error opening session file");
        return -1;
    }
    //read lock
    mrlock_w(fd, &lock);

    // Read the session struct from the file
    Sess_Identifier session;
    ssize_t read_bytes = read(fd, &session, sizeof(Sess_Identifier));
    if (read_bytes != sizeof(Sess_Identifier)) {
        perror("Error reading session data");
        unlock(fd, &lock);
        close(fd);
        return -1;
    }

    // Display the session details
    // printf("Logging out user %s:\n", username);
    // printf("Client Socket: %d\n", session.client_socket);
    // printf("Process ID: %d\n", session.pid);
    // printf("Login Time: %ld\n", session.loginTime);

    //unlock and close session file
    unlock(fd, &lock);
    close(fd);
    // Remove the session file
    if (unlink(session_file) == 0) {
        return 0;
    } else {
        perror("Error removing session file");
        return -1;
    }
}





