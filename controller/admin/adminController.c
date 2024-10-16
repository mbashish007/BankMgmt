#include "adminController.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../admin/admin.h"  // Include action.h where you define all functions
#include "../../login/login.h"
#include "../../utils/fileUtils.h"

#define ADM_DIR  "./data/users/admin/"
#define CUST_DIR  "./data/users/customer/"
#define EMP_DIR  "./data/users/employee/"
#define MGR_DIR  "./data/users/manager/"

void display_menu() {
    printf("\n--- Admin Panel ---\n");
    printf("1. Add New Admin User\n");
    printf("2. Add New Bank Employee\n");
    printf("3. Modify Customer Details\n");
    printf("4. Modify Employee Details\n");
    printf("5. Manage User Roles\n");
    printf("6. Change Password\n");
    printf("7. Logout\n");
    printf("Select an option (1-7): ");
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
                printf("Enter new employee username: ");
                scanf("%s", employee.username);
                printf("Enter employee name: ");
                scanf("%s", employee.name);
                printf("Enter employee grade (A, B, C, etc.): ");
                scanf(" %c", &employee.grade);
                strcpy(employee.status, "active");  // Set default status to Active
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
                scanf("%s", employee.username);
                printf("Enter new employee name: ");
                scanf("%s", employee.name);
                printf("Enter new employee grade (A, B, C, etc.): ");
                scanf(" %c", &employee.grade);
                if(modify_employee_details(employee.username, &employee) < 0)  {
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
            default: {
                printf("Invalid option. Please try again.\n");
            }
        }
    }

    return 0;
}
