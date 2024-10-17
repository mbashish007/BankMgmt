#include "adminController.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../admin/admin.h"  // Include action.h where you define all functions
#include "../../login/login.h"
#include "../../sessions/sessions.h"
#include "../../utils/fileUtils.h"
#include "../../service/employee/employeeService.h"

#define ADM_DIR  "./data/users/admin/"
#define CUST_DIR  "./data/users/customer/"
#define EMP_DIR  "./data/users/employee/"
#define MGR_DIR  "./data/users/manager/"
#define MAX_FILES 1024

#define ROLE_EMPLOYEE "employee"
#define ROLE_MANAGER  "manager"

void display_menu() {
    printf("\n--- Admin Panel ---\n");
    printf("1. Add New Admin User\n");
    printf("2. Add New Bank Employee\n");
    printf("3. Modify Customer Details\n");
    printf("4. Modify Employee Details\n");
    printf("5. Manage User Roles\n");
    printf("6. Change Password\n");
    printf("7. Logout\n");
    printf("8. View Employees\n");
    printf("Select an option (1-7): ");
}

int is_valid_role(const char* role) {
    if (strcmp(role, ROLE_EMPLOYEE) == 0 || strcmp(role, ROLE_MANAGER) == 0) {
        return 1;  // Role is valid
    }
    return 0;  // Role is invalid
}

AdminDTO currAdmin;

int admin_sess(const char *username) {
    memset(&currAdmin, 0, sizeof(AdminDTO));
    int choice;
    AdminDTO admin;
    EmployeeDTO employee;
    CustomerDTO customer;
    char newPassword[MAX_PASSWORD_LEN];
    char newRole[MAX_STATUS_LEN];

    getAdmin(username, &currAdmin);

    while (1) {
        display_menu();
        scanf("%d", &choice);  // Get the user's choice

        switch (choice) {
            case 1: {
                // Add new admin user
                memset(&admin, 0, sizeof(AdminDTO));
                printf("Enter new admin username: ");
                scanf("%s", admin.username);
                printf("Enter admin name: ");
                scanf("%s", admin.name);
                strcpy(admin.status, "active");  // Set default status to Active
                add_new_admin(&admin);
                break;
            }
            case 2: {
                // Add new bank employee
                memset(&employee, 0, sizeof(EmployeeDTO));
                // printf("Enter new employee username: ");
                // scanf("%s", employee.username);
                printf("Enter employee name: ");
                scanf("%s", employee.name);
                printf("Enter employee grade (A, B, C, etc.): ");
                scanf(" %c", &employee.grade);
                strcpy(employee.status, "active"); // Set default status to Active 
                strcpy(employee.role, "employee"); // Default role employee
                add_new_employee(&employee);
                break;
            }
            case 3: {
                // Modify customer details
                memset(&customer, 0, sizeof(CustomerDTO));
                char unamebuff[MAX_USERNAME_LEN];
                char namebuff[MAX_Name_LEN];
                char phonebuff[MAX_PHONE_LEN];
                
                printf("Enter customer username to modify: ");
                scanf("%s", unamebuff);
                strncpy(customer.username, unamebuff, MAX_USERNAME_LEN);
            
                printf("Enter new customer name: ");
                scanf("%s", namebuff);
                strncpy(customer.name, namebuff, MAX_Name_LEN);

                printf("Enter new customer phone number: ");
                scanf("%s", phonebuff);
                strncpy(customer.phone, phonebuff, MAX_PHONE_LEN);//null terminate manually when doing socket programming
             
                if(modify_customer_details(customer.username,&customer) < 0) {
                    printf("Customer Modification failed");
                }else {
                    printf("Customer Modified Successfully\n");
                }

                break;
            }
            case 4: {
                // Modify employee details
                memset(&employee, 0, sizeof(EmployeeDTO));
                printf("Enter employee username to modify: ");
                scanf("%ld", &employee.empId);

                if(employee.empId - EMPID_START < 0) {
                    printf("Invalid Argument for Emp ID\n");
                    break; // Invalid Argument
                }
                
                printf("Enter new employee name: ");
                scanf("%s", employee.name);
                printf("Enter new employee grade (A, B, C, etc.): ");
                scanf(" %c", &employee.grade);
                printf("Enter new employee role: ");
                scanf("%s", employee.role);

                  if (!is_valid_role(employee.role)) {
                    printf("Error: Invalid role entered. Please enter either 'employee' or 'manager'.\n");
                    break;
                }

                if(modify_employee_details(employee.empId, &employee) < 0)  {
                    printf("EMPloyee modification failed\n");
                }
                break;
            }
            case 5: {
                // Manage user roles
                printf("Enter username: ");
                scanf("%s", admin.username);
                printf("Enter new role (Active/Inactive): ");
                scanf("%s", newRole);
                manage_user_roles(admin.username, newRole);
                break;
            }
            case 6: {
                printf("Enter new Password: ");
                bzero(newPassword, sizeof(newPassword));
                scanf("%s",newPassword);
                // Change password
                update_user_password(currAdmin.username, newPassword);
                break;
            }
            case 7: {
                // Logout
                printf("Logging out...\n");
                if(logout(currAdmin.username)<0){
                    printf("Error Logging Out\n");
                    break;
                }
                return 0;
            }
            case 8: {
                // Logout
                printf("Listing all employees\n");
                int count = 1024;
                EmployeeDTO employees[count];
            memset(employees, 0, sizeof(EmployeeDTO) * MAX_FILES);
            int op = read_employees_data(employees);
            if (op == -1) {
                printf( "Failed to read employee data.\n");
            }

            for (int i = 0; i < op-1; i++) {
        printf("Employee %d: Username: %ld, Name: %s, Grade: %c, Role: %s, Status: %s\n",
               i + 1,
               employees[i].empId,
               employees[i].name,
               employees[i].grade,
               employees[i].role,
               employees[i].status);
    }

            // Check the operation code
            if (op == 0) {
                printf("There are more than 1024 employees\n");
            } 
            break;
            }

        case 9: {
                // Logout
                printf("enter employee Id to see details: ");
                long empid;
                scanf("%ld", &empid);
                EmployeeDTO emp;
                memset(&emp, 0, sizeof(EmployeeDTO));
                int op = get_employee_data(empid, &emp);
                if(op == -2) {
                    printf("Invalid Id");
                    break;
                }
                if(op == -1) {
                    printf("error could not fetch details\n");
                    break;
                }
                printf("Employee details: Username: %ld, Name: %s, Grade: %c, Role: %s, Status: %s\n",
                        emp.empId,
                        emp.name,
                        emp.grade,
                        emp.role,
                        emp.status
                        );

                break;
                }
        
        case 10: {
                // Logout
                printf("enter employee Id to see details: ");
                long empid;
                scanf("%ld", &empid);
                int op = activate_deactivate_employee(empid);
                if(op == -2) {
                    printf("Invalid Id");
                    break;
                }
                if(op == -1) {
                    printf("error could not fetch details\n");
                    break;
                }
                printf("Employee Status flipped\n");
                break;
            // Check the operation code
            
        }
    }

    return 0;
}
