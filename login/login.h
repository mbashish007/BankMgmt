#ifndef LOGIN_H
#define LOGIN_H

#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 65

#include <stdbool.h>

// Add a new user to the system
int add_user(const char* username, const char* password);

// Authenticate a user
bool authenticate_user(const char* username, const char* password);

// Hash a password using SHA-256
void hash_password(const char* password, char* hash);

// Update User's Password
int update_user_password(const char* username, const char* newPassword) ;
#endif
