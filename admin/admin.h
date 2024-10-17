#ifndef ADMIN_H
#define ADMIN_H

#include "../Structs/adminDTO.h"
#include "../Structs/employeeDTO.h"
#include "../Structs/customerDTO.h"

int getAdmin(char* username, AdminDTO* admin) ;

/**
 * return 0 success
 * return -1 error
 * return -2 user with username already exists
 */
int add_new_admin(AdminDTO *admin) ;

// Function to add a new Bank Employee
/**
 * return 0 success
 * return -1 error
 * return -2 user with username already exists
 */
int add_new_employee(EmployeeDTO *employee);

/**
 * returns
 * -1 error
 * 0 success
 */
int modify_customer_details(const char *username, CustomerDTO *newDetails) ;

// Function to modify employee details
int modify_employee_details(long , EmployeeDTO *newDetails) ;

// Function to manage user roles
int manage_user_roles(const char *username, const char *newRole) ;

int activate_deactivate_employee(long );

#endif