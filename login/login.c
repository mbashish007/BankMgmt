#include "login.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/file.h> // for fcntl()
#include <openssl/evp.h>
#include "../Structs/userCred.h"
#include "../utils/file/fileUtils.h"

#define USER_FILE "logins"

// Function to convert hash to a hexadecimal string
char* hash_to_hex(const unsigned char* hashRaw) {
    char* hexStr = malloc(MAX_PASSWORD_LEN); // Allocate space for hash
    for (int i = 0; i < 32; i++) {
        sprintf(hexStr + (i * 2), "%02x", hashRaw[i]);
    }
    hexStr[64] = '\0';
    return hexStr;
}

// Function to hash a password using SHA-256
void hash_password(const char* password, char* hashRaw) {
    EVP_MD_CTX* mdctx;
    const EVP_MD* md;
    unsigned int hashLen;

    // Create and initialize the context
    mdctx = EVP_MD_CTX_new();
    md = EVP_sha256(); // Use SHA-256 hashing algorithm

    // Initialize, update, and finalize the hash
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, password, strlen(password));
    EVP_DigestFinal_ex(mdctx, hashRaw, &hashLen);

    // Clean up
    EVP_MD_CTX_free(mdctx);
}

// Function to add a new user to the system
int add_user(const char* username, const char* password) {
    struct flock lock;
    // Hash the password using SHA-256
    char passwordHash[MAX_PASSWORD_LEN];
    hash_password(password, passwordHash);
    char* hexPassword = hash_to_hex(passwordHash);

    // Create a new user struct
    UserCred newUser;
    strncpy(newUser.username, username, MAX_USERNAME_LEN);
    strncpy(newUser.passwordHash, hexPassword, MAX_PASSWORD_LEN);

    // Open the user file for writing (append mode)
    int fd = open(USER_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Failed to open user file");
        return -1;
    }


    printf("Acquiring Write Lock\n");

    // Lock the file exclusively for writing
    if (mwlock_w(fd, &lock) == -1) {
        perror("Failed to lock user file");
        close(fd);
        return -1;
    }

    // Write the new user to the file
    ssize_t bytesWritten = write(fd, &newUser, sizeof(UserCred));
    if (bytesWritten == -1) {
        perror("Failed to write user to file");
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock); // Unlock the file
        close(fd);
        return -1;
    }

    // Unlock the file and close it
    unlock(fd, &lock);
    close(fd);

    printf("User %s added successfully.\n", username);
    return 0;
}

// Function to authenticate a user by username and password
bool authenticate_user(const char* username, const char* password) {
    struct flock lock;
    // Open the user file for reading
    int fd = open(USER_FILE, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open user file");
        return false;
    }

   

    printf("Acquiring Read Lock\n");
    // Lock the file for shared reading
    if (mrlock_w(fd, &lock) == -1) {
        perror("Failed to lock user file");
        close(fd);
        return false;
    }
    // Hash the input password
    char passwordHash[MAX_PASSWORD_LEN];
    hash_password(password, passwordHash);
    char* recvPassword = hash_to_hex(passwordHash);
    UserCred user;
    ssize_t bytesRead;

    // Loop through the file and check each user
    while ((bytesRead = read(fd, &user, sizeof(UserCred))) > 0) {
        if (strncmp(user.username, username, MAX_USERNAME_LEN) == 0) {
            // Username matches, now check the password
            if (strncmp(user.passwordHash, recvPassword, MAX_PASSWORD_LEN) == 0) {
                unlock(fd, &lock);  // Unlock the file
                close(fd);
                printf("Login successful! Welcome, %s.\n", username);
                return true;
            } else {
                printf("Incorrect password for user %s.\n", username);
                unlock(fd, &lock);
                return false;
            }
        }
    }

    // If we reach here, the user was not found
    unlock(fd, &lock); // Unlock the file
    close(fd);
    printf("User %s not found.\n", username);
    return false;
}

int update_user_password(const char* username, const char* newPassword) {
    UserCred user;
    int found = 0;
    off_t offset = 0; // Offset to track the position in the file

    // Open user file
    int fd = open(USER_FILE, O_RDWR);
    if (fd == -1) {
        perror("Failed to open user file");
        return -1;
    }
    struct flock lock;
    
        // Apply a write lock
    if (mwlock_w(fd, &lock) == -1) {
        perror("Failed to lock user file");
        close(fd);
        return -1;
    }


 
    // Read user data and update password if found
    while (read(fd, &user, sizeof(UserCred)) > 0) {
        if (strcmp(user.username, username) == 0) {
            // User found, hash the new password
            unsigned char hashRaw[32]; // SHA-256 produces 32 bytes
            hash_password(newPassword, hashRaw);
            char* newHash = hash_to_hex(hashRaw);
            strncpy(user.passwordHash, newHash, MAX_PASSWORD_LEN);
            free(newHash);
            found = 1;

            // Move ptr to correct pos
            lseek(fd, offset, SEEK_SET);
            int wbytes = write(fd, &user, sizeof(UserCred)); // Update the password
            if(wbytes<0) {
                unlock(fd, &lock);
                close(fd);
                return -1;

            }
            break; // Exit loop after updating
        }
        offset += sizeof(UserCred); // Move to the next user record
    }

    
    unlock(fd, &lock);
    close(fd);

    return found ? 0 : -1; // Return 0 if updated, -1 if user not found
}
