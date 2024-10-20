#include "adminController.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../admin/admin.h"  // Include action.h where you define all functions
#include "../../login/login.h"
#include "../../sessions/sessions.h"
#include "../../utils/file/fileUtils.h"
#include "../../service/employee/employeeService.h"
#include "../../service/customer/customerService.h"

#define ADM_DIR  "./data/users/admin/"
#define CUST_DIR  "./data/users/customer/"
#define EMP_DIR  "./data/users/employee/"
#define MGR_DIR  "./data/users/manager/"
#define MAX_FILES 1024
#define BUFFER_SIZE 512

#define ROLE_EMPLOYEE "employee"
#define ROLE_MANAGER  "manager"

void writeAMsg(int sd, const char* msg) {
    write(sd, msg, strlen(msg));
}

void getInput(int sd, char *input, size_t size) {
    ssize_t bytesRead = read(sd, input, size);
        input[bytesRead - 1] = '\0';  
}
void display_menu(int sd) {
    writeAMsg(sd,"\n--- Admin Panel ---\n");
    writeAMsg(sd, "1. Add New Admin User\n");
    writeAMsg(sd, "2. Add New Bank Employee\n");
    writeAMsg(sd, "3. Modify Customer Details\n");
    writeAMsg(sd, "4. Modify Employee Details\n");
    writeAMsg(sd, "5. Manage User Roles\n");
    writeAMsg(sd, "6. Change Password\n");
    writeAMsg(sd, "7. Logout\n");
    writeAMsg(sd, "8. View Employees\n");
    writeAMsg(sd, "Select an option (1-7): ");
}

int is_valid_role(const char* role) {
    if (strcmp(role, ROLE_EMPLOYEE) == 0 || strcmp(role, ROLE_MANAGER) == 0) {
        return 1;  // Role is valid
    }
    return 0;  // Role is invalid
}

int is_valid_status(const char* role) {
    if (strcmp(role, EMP_STATUS_ACTIVE) == 0 || strcmp(role, EMP_STATUS_INACTIVE) == 0) {
        return 1;  // Role is valid
    }
    return 0;  // Role is invalid
}


AdminDTO currAdmin;

int admin_sess(int sd, const char *username) {
    const char* msg;
    char inputbuffer[BUFFER_SIZE];
    char wbuffer[BUFFER_SIZE];
    memset(&currAdmin, 0, sizeof(AdminDTO));
    int choice;
    AdminDTO admin;
    EmployeeDTO employee;
    CustomerDTO customer;
    char newPassword[MAX_PASSWORD_LEN];
    char newRole[MAX_STATUS_LEN];

    getAdmin(username, &currAdmin);

    ssize_t bytesRead;
    while (1) {
        display_menu(sd);
        getInput(sd, inputbuffer, BUFFER_SIZE); // Get the user's choice
        choice = atoi(inputbuffer);
        switch (choice) {
            case 1: {
                // Add new admin user
                memset(&admin, 0, sizeof(AdminDTO));
                writeAMsg(sd, "Enter new admin username: ");
                getInput(sd, admin.username, sizeof(admin.username));
                writeAMsg(sd, "Enter admin name: ");
                getInput(sd, admin.name, sizeof(admin.name));
                strcpy(admin.status, "active");  // Set default status to Active
                add_new_admin(&admin);
                break;
            }
            case 2: {
                // Add new bank employee
                memset(&employee, 0, sizeof(EmployeeDTO));
                
                writeAMsg(sd, "Enter employee name: ");
                getInput(sd, employee.name, sizeof(employee.name));
                writeAMsg(sd, "Enter employee grade (A, B, C, etc.): ");
                getInput(sd, inputbuffer, sizeof(inputbuffer));
                employee.grade = inputbuffer[0];
                strcpy(employee.status, "active"); // Set default status to Active 
                strcpy(employee.role, "employee"); // Default role employee
                if(add_new_employee(&employee)<0)  {
                    writeAMsg(sd, "Error: Could not add Employee\n");
                }else {
                    snprintf(wbuffer, BUFFER_SIZE, "Success Added new Employee: %ld", employee.empId);
                    write(sd, wbuffer, strlen(wbuffer)); 
                }
                break;
            }
            case 3: {
                // Modify customer details
                memset(&customer, 0, sizeof(CustomerDTO));
                char unamebuff[MAX_USERNAME_LEN];
                char namebuff[MAX_NAME_LEN];
                char phonebuff[MAX_PHONE_LEN];
                
                writeAMsg(sd, "Enter customer username to modify: ");
                 getInput(sd, inputbuffer, sizeof(inputbuffer));
                customer.userId = atol(inputbuffer);
                // strncpy(customer.username, unamebuff, MAX_USERNAME_LEN);
            
                writeAMsg(sd, "Enter new customer name: ");
                getInput(sd, customer.name, sizeof(customer.name));
                // strncpy(customer.name, namebuff, MAX_NAME_LEN);

                writeAMsg(sd, "Enter new customer phone number: ");
                getInput(sd, customer.phone, sizeof(customer.phone));
                // strncpy(customer.phone, phonebuff, MAX_PHONE_LEN);//null terminate manually when doing socket programming
             
                if(modify_customer_details(customer.userId,&customer) < 0) {
                    writeAMsg(sd, "Customer Modification failed");
                }else {
                    writeAMsg(sd, "Customer Modified Successfully\n");
                }

                break;
            }
            case 4: {
                // Modify employee details
                memset(&employee, 0, sizeof(EmployeeDTO));
                writeAMsg(sd, "Enter employee username to modify: ");
                    getInput(sd, inputbuffer, sizeof(inputbuffer));
                employee.empId = atol(inputbuffer);

                if(employee.empId - EMPID_START < 0) {
                    writeAMsg(sd, "Invalid Argument for Emp ID\n");
                    break; // Invalid Argument
                }
                
                writeAMsg(sd, "Enter new employee name: ");
                 getInput(sd, employee.name, MAX_NAME_LEN);
                writeAMsg(sd, "Enter new employee grade (A, B, C, etc.): ");
                getInput(sd, inputbuffer, sizeof(inputbuffer));
                employee.grade = inputbuffer[0];
                writeAMsg(sd, "Enter new employee role: ");
                getInput(sd, employee.role, MAX_ROLE_LEN);

                if (!is_valid_role(employee.role)) {
                                    writeAMsg(sd, "Error: Invalid role entered. Please enter either 'employee' or 'manager'.\n");
                                    break;
                }
                //  writeAMsg(sd, "Enter new employee status: ");
                // getInput(sd, employee.status, MAX_STATUS_LEN);

                //   if (!is_valid_status(employee.status)) {
                //     writeAMsg(sd, "Error: Invalid status entered. Please enter either 'active' or 'inactive'.\n");
                //     break;
                // }

                if(modify_employee_details(employee.empId, &employee) < 0)  {
                    writeAMsg(sd, "EMPloyee modification failed\n");
                }else 
                    writeAMsg(sd, "Employee modified successfully\n");
                break;
            }
            case 5: {
                // Manage user roles
              
                break;
            }
            case 6: {
                writeAMsg(sd, "Enter new Password: ");
                bzero(newPassword, sizeof(newPassword));
                getInput(sd, newPassword, sizeof(newPassword));
                // Change password
                update_user_password(username, newPassword);
                break;
            }
            case 7: {
                // Logout
                writeAMsg(sd, "Logging out...\n");
                if(logout(username)<0){
                    writeAMsg(sd, "Error Logging Out\n");
                    break;
                }
                return 0;
            }
            case 8: {
                // Logout
                writeAMsg(sd, "Listing all employees\n");
                int count = 1024;
                EmployeeDTO employees[count];
            memset(employees, 0, sizeof(EmployeeDTO) * MAX_FILES);
            int op = read_employees_data(employees);
            if (op == -1) {
                writeAMsg(sd,  "Failed to read employee data.\n");
            }

            for (int i = 0; i < op-1; i++) {
                snprintf(wbuffer, BUFFER_SIZE, "Employee %d: Username: %ld, Name: %s, Grade: %c, Role: %s, Status: %s\n",
               i + 1,
               employees[i].empId,
               employees[i].name,
               employees[i].grade,
               employees[i].role,
               employees[i].status);
               
               write(sd, wbuffer, strlen(wbuffer));
            }

            // Check the operation code
            if (op == 0) {
                writeAMsg(sd, "There are more than 1024 employees\n");
            } 
            break;
            }

        case 9: {
                // Logout
                writeAMsg(sd, "enter employee Id to see details: ");
                long empid;
                getInput(sd, inputbuffer, sizeof(inputbuffer));
                empid = atol(inputbuffer);
                EmployeeDTO emp;
                memset(&emp, 0, sizeof(EmployeeDTO));
                int op = get_employee_data(empid, &emp);
                if(op == -2) {
                    writeAMsg(sd, "Invalid Id");
                    break;
                }
                if(op == -1) {
                    writeAMsg(sd, "error could not fetch details\n");
                    break;
                }
                snprintf(wbuffer, BUFFER_SIZE, "Employee details: Username: %ld, Name: %s, Grade: %c, Role: %s, Status: %s\n",
                        emp.empId,
                        emp.name,
                        emp.grade,
                        emp.role,
                        emp.status
                        );
                
                write(sd, wbuffer, strlen(wbuffer));

                break;
                }
        
        case 10: {
                // Logout
                writeAMsg(sd, "Enter employee Id to Flip status details: ");
                long empid;
                getInput(sd, inputbuffer, sizeof(inputbuffer));
                empid = atol(inputbuffer);
                
                int op = activate_deactivate_employee(empid);
                if(op == -2) {
                    writeAMsg(sd, "Invalid Id");
                    break;
                }
                if(op == -1) {
                    writeAMsg(sd, "error could not fetch details\n");
                    break;
                }
                writeAMsg(sd, "Employee Status flipped\n");
                
            // Check the operation code
            
             }
    }

    
    }
    return 0;
}
