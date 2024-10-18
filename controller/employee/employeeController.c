#include "employeeController.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../admin/admin.h" // Include action.h where you define all functions
#include "../../login/login.h"
#include "../../sessions/sessions.h"
#include "../../utils/file/fileUtils.h"
#include "../../service/employee/employeeService.h"
#include "../../service/customer/customerService.h"

#define ADM_DIR "./data/users/admin/"
#define CUST_DIR "./data/users/customer/"
#define EMP_DIR "./data/users/employee/"
#define MGR_DIR "./data/users/manager/"
#define MAX_FILES 1024

EmployeeDTO currEmployee;

// Stub function implementations
void addCustomer(int sd)
{
    int sd1 = STDOUT_FILENO;
    CustomerDTO customer;
    char buffer[BUFFER_SIZE];

    // Initialize the customer structure
    memset(&customer, 0, sizeof(CustomerDTO));

    // Ask for customer name
    const char *msg = "Enter customer name: ";
    write(STDOUT_FILENO, msg, strlen(msg)); // Send the message to the socket
    read(sd, customer.name, MAX_NAME_LEN);  // Read customer name from the socket

    // Ask for customer phone
    msg = "Enter customer phone: ";
    write(STDOUT_FILENO, msg, strlen(msg));  // Send the message to the socket
    read(sd, customer.phone, MAX_PHONE_LEN); // Read customer phone from the socket

    // Ask for customer status
    msg = "Enter customer status (active/inactive): ";
    write(STDOUT_FILENO, msg, strlen(msg));    // Send the message to the socket
    read(sd, customer.status, MAX_STATUS_LEN); // Read customer status from the socket

    customer.balance = 0; // Convert balance to an integer

    int op = add_new_customer(&customer);
    if (op == -1)
    {
        msg = "Customer added successfully!\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        return;
    }
    // Confirm customer addition
    msg = "Customer added successfully!\n";
    write(STDOUT_FILENO, msg, strlen(msg)); // Send confirmation to the socket

    // Process the customer data as needed, e.g., save to a file or database
}

void modify_customer(int sd)
{
    CustomerDTO customer;
    char buffer[BUFFER_SIZE];

    // Initialize the customer structure to zeros
    memset(&customer, 0, sizeof(CustomerDTO));

    // Ask for customer ID
    const char *msg = "Enter customer ID: ";
    write(STDOUT_FILENO, msg, strlen(msg)); // Send the prompt to the socket
    read(sd, buffer, sizeof(buffer));       // Read customer ID input
    customer.userId = atol(buffer);         // Convert customer ID to long

    // Ask for customer name
    msg = "Enter new customer name: ";
    write(STDOUT_FILENO, msg, strlen(msg)); // Send the prompt to the socket
    read(sd, customer.name, MAX_NAME_LEN);  // Read customer name

    // Ask for customer phone number
    msg = "Enter new customer phone: ";
    write(STDOUT_FILENO, msg, strlen(msg));  // Send the prompt to the socket
    read(sd, customer.phone, MAX_PHONE_LEN); // Read customer phone number

    int op = modify_customer_details(customer.userId, &customer);
    if (op == -2)
    {
        msg = "Customer details updated successfully!\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        return;
    }
    if (op == -1)
    {
        msg = "Error  Occurred Could not update Customer!\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        return;
    }
    // Confirm the modification
    msg = "Customer details updated successfully!\n";
    write(STDOUT_FILENO, msg, strlen(msg));

    // Implementation logic for modifying customer details
}

void viewCustomer(int sd)
{
    char buffer[BUFFER_SIZE];
    CustomerDTO customer;
    // Ask for customer ID
    const char *msg = "Enter customer ID: ";
    write(STDOUT_FILENO, msg, strlen(msg)); // Send the prompt to the socket
    read(sd, buffer, sizeof(buffer));       // Read customer ID input
    long userId = atol(buffer);             // Convert customer ID to long
    char wbuffer[BUFFER_SIZE];

    CustomerDTO emp;
    memset(&emp, 0, sizeof(CustomerDTO));
    int op = get_customer_data(userId, &customer);
    if (op == -2)
    {
        msg = "Invalid Id";
        write(STDOUT_FILENO, msg, strlen(msg));

        return;
    }
    if (op == -1)
    {
        msg = "error could not fetch details\n";
        write(STDOUT_FILENO, msg, strlen(msg));

        return;
    }
        bzero(wbuffer, sizeof(wbuffer));
        int n = snprintf(wbuffer, sizeof(wbuffer),
                         "Customer : UserId: %ld, Name: %s, Phone: %s, Balance: %d, Status: %s\n",
                         customer.userId,
                         customer.name,
                         customer.phone,
                         customer.balance,
                         customer.status);
            write(sd, wbuffer, n);
}

void viewAllCustomers(int sd)
{
    const char *msg = "Listing All Customers\n";
    write(STDOUT_FILENO, msg, sizeof(msg));
    char wbuffer[BUFFER_SIZE];
    int count = 1024;
    CustomerDTO customers[count];
    memset(customers, 0, sizeof(CustomerDTO) * MAX_FILES);
    int op = read_customers_data(customers);
    if (op == -1)
    {
        msg = "Failed to read customer data.\n";
        write(STDOUT_FILENO, msg, sizeof(msg));
    }

    for (int i = 0; i < op - 1; i++)
    {
        bzero(wbuffer, sizeof(wbuffer));
        int n = snprintf(wbuffer, sizeof(wbuffer),
                         "Customer %d: UserId: %ld, Name: %s, Phone: %s, Balance: %d, Status: %s\n",
                         i + 1,
                         customers[i].userId,
                         customers[i].name,
                         customers[i].phone,
                         customers[i].balance,
                         customers[i].status);

        // Use write to send the data through the socket descriptor (sd)
        if (n > 0)
        {
            write(sd, wbuffer, n);
        }
    }
    // Check the operation code
    if (op == 0)
    {
        msg = "There are more than 1024 customers\n";
        write(STDOUT_FILENO, msg, sizeof(msg));
    }
    // Implementation logic for viewing assigned loan applications
}

void process_loan_applications()
{
    printf("Process Loan Applications called.\n");
    // Implementation logic for processing loan applications
}

void approve_reject_loans()
{
    printf("Approve/Reject Loans called.\n");
    // Implementation logic for approving or rejecting loans
}

void view_assigned_loan_applications()
{
    printf("View Assigned Loan Applications called.\n");
    // Implementation logic for viewing assigned loan applications
}

int employee_session(int sd, long empId)
{
    char option[BUFFER_SIZE];
    const char *msg = "Hello";
    if(get_employee_data(empId, &currEmployee)<0) {
        msg = "could not open employee";
        write(STDOUT_FILENO, msg, sizeof(msg));
        char empid_char[MAX_USERNAME_LEN];
        snprintf(empid_char, MAX_USERNAME_LEN, "%ld", empId);
        logout(empid_char);

    }
    while (1)
    {
        printf("\n--- Customer Management Menu ---\n");
        printf("1. Add New Customer\n");
        printf("2. View All Customers\n");
        printf("3. View Customer\n");
        printf("4. Modify Customer Details\n");
        printf("5. Process Loan Applications\n");
        printf("6. Approve/Reject Loans\n");
        printf("7. View Assigned Loan Applications\n");
        printf("8. Logout\n");
        printf("Enter your choice: ");

        // Reading input using the read() system call
        ssize_t bytes_read = read(STDIN_FILENO, option, sizeof(option));
        // option[bytes_read - 1] = '\0'; // Remove newline

        int opt = atoi(option);
        switch (opt)
        {
        case 1:
            addCustomer(sd);
            break;
        case 2:
            viewAllCustomers(sd);
            break;
        case 3:
            viewCustomer(sd);
            break;
        case 4:
            modify_customer(sd);
            break;
        case 5:
            process_loan_applications();
            break;
        case 6:
            approve_reject_loans();
            break;
        case 7:
            view_assigned_loan_applications();
            break;
        case 8:
            printf("Exiting...\n");
            return 0;
        default:
            printf("Invalid option, please try again.\n");
            break;
        }
    }
}
