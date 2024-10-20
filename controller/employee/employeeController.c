#include "employeeController.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../admin/admin.h" // Include action.h where you define all functions
#include "../../login/login.h"
#include "../../sessions/sessions.h"
#include "../../utils/file/fileUtils.h"
#include "../../utils/transaction/transaction.h"
#include "../../utils/loan/loan.h"
#include "../../service/employee/employeeService.h"
#include "../../service/customer/customerService.h"

#include "../../Structs/txnDTO.h"
#include "../../Structs/loanDTO.h"

#define ADM_DIR "./data/users/admin/"
#define CUST_DIR "./data/users/customer/"
#define EMP_DIR "./data/users/employee/"
#define MGR_DIR "./data/users/manager/"
#define MAX_FILES 1024

EmployeeDTO currEmployee;

void writeECMsg(int sd, const char* msg) {
    write(sd, msg, strlen(msg));
}

void getInputEC(int sd, char *input, size_t size) {
    ssize_t bytesRead = read(sd, input, size);
        input[bytesRead - 1] = '\0';  
        if (input[bytesRead - 2] == '\r') {
        input[bytesRead - 2] = '\0';
    }
}
// Stub function implementations
void addCustomer(int sd)
{
    int sd1 = sd;
    CustomerDTO customer;
    char buffer[BUFFER_SIZE];

    // Initialize the customer structure
    memset(&customer, 0, sizeof(CustomerDTO));

    // Ask for customer name
    const char *msg = "Enter customer name: ";
    write(sd, msg, strlen(msg)); // Send the message to the socket
    getInputEC(sd, customer.name, MAX_NAME_LEN);  // Read customer name from the socket

    // Ask for customer phone
    msg = "Enter customer phone: ";
    write(sd, msg, strlen(msg));  // Send the message to the socket
    getInputEC(sd, customer.phone, MAX_PHONE_LEN); // Read customer phone from the socket

    // Ask for customer status
    strncpy(customer.status, "active", sizeof("active")); // Read customer status from the socket

    customer.balance = 0; // Convert balance to an integer

    int op = add_new_customer(&customer);
    if (op == -1)
    {
        msg = "Customer added successfully!\n";
        write(sd, msg, strlen(msg));
        return;
    }
    // Confirm customer addition
    snprintf(buffer, BUFFER_SIZE, "Customer added successfully! with ID: %ld\n", customer.userId);
    write(sd, buffer, strlen(buffer)); 

  
}

void modify_customer(int sd)
{
    CustomerDTO customer;
    char buffer[BUFFER_SIZE];

    // Initialize the customer structure to zeros
    memset(&customer, 0, sizeof(CustomerDTO));

    // Ask for customer ID
    const char *msg = "Enter customer ID: ";
    write(sd, msg, strlen(msg)); // Send the prompt to the socket
    getInputEC(sd, buffer, sizeof(buffer));       // Read customer ID input
    customer.userId = atol(buffer);         // Convert customer ID to long

    // Ask for customer name
    msg = "Enter new customer name: ";
    write(sd, msg, strlen(msg)); // Send the prompt to the socket
    getInputEC(sd, customer.name, MAX_NAME_LEN);  // Read customer name

    // Ask for customer phone number
    msg = "Enter new customer phone: ";
    write(sd, msg, strlen(msg));  // Send the prompt to the socket
    getInputEC(sd, customer.phone, MAX_PHONE_LEN); // Read customer phone number

    int op = modify_customer_details(customer.userId, &customer);
    if (op == -2)
    {
        msg = "Customer details updated successfully!\n";
        write(sd, msg, strlen(msg));
        return;
    }
    if (op == -1)
    {
        msg = "Error  Occurred Could not update Customer!\n";
        write(sd, msg, strlen(msg));
        return;
    }
    // Confirm the modification
    msg = "Customer details updated successfully!\n";
    write(sd, msg, strlen(msg));

    // Implementation logic for modifying customer details
}

void viewCustomer(int sd)
{
    char buffer[BUFFER_SIZE];
    CustomerDTO customer;
    // Ask for customer ID
    const char *msg = "Enter customer ID: ";
    write(sd, msg, strlen(msg)); // Send the prompt to the socket
    getInputEC(sd, buffer, sizeof(buffer));       // Read customer ID input
    long userId = atol(buffer);             // Convert customer ID to long
    char wbuffer[BUFFER_SIZE];

    CustomerDTO emp;
    memset(&emp, 0, sizeof(CustomerDTO));
    int op = get_customer_data(userId, &customer);
    if (op == -2)
    {
        msg = "Invalid Id";
        write(sd, msg, strlen(msg));

        return;
    }
    if (op == -1)
    {
        msg = "error could not fetch details\n";
        write(sd, msg, strlen(msg));

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
    write(sd, msg, strlen(msg));
    char wbuffer[BUFFER_SIZE];
    int count = 1024;
    CustomerDTO customers[count];
    memset(customers, 0, sizeof(CustomerDTO) * MAX_FILES);
    int op = read_customers_data(customers);
    if (op == -1)
    {
        msg = "Failed to read customer data.\n";
        write(sd, msg, strlen(msg));
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
        write(sd, msg, strlen(msg));
    }
   
}

void process_loan_applications( int sd)
{
    char inputbuffer[BUFFER_SIZE];
     char wbuffer[BUFFER_SIZE];
    bzero(inputbuffer, BUFFER_SIZE);
    bzero(wbuffer, BUFFER_SIZE);

    long loanId;
    //Read loanId
    const char* msg = "Enter Loan ID tp process:  ";
    write(sd, msg, strlen(msg));
    ssize_t bytesRead ;
    getInputEC(sd, inputbuffer, BUFFER_SIZE);
      
    loanId = atol(inputbuffer);
    LoanDTO loan ;
    int op = get_loan_data(loanId, &loan);
    if( op<0 ) {
        if(op == -1) {
            msg = "Error Getting Loan Data\n";
            write(sd, msg, strlen(msg));
        }else if(op == -2) {
            msg = "Error: Invalid loan id entered\n";
            write(sd, msg, strlen(msg));
        }
        return;
    }

    int n = snprintf(wbuffer, sizeof(wbuffer),
         "Loan : LoanId: %ld, Type: %s, CustomerId: %ld, EmpId: %ld, Amount: %d, Interest: %d, Tenure: %d months, Amount Paid Back: %d, Status: %s\n",
                                
         loan.loanId,                 
         loan.type,                   
         loan.customerId,             
         loan.empId,                  
         loan.amount,                 
         loan.interest,               
         loan.tenure,                 
         loan.amountPayedBack,        
         loan.status); 

        // Use write to send the data through the socket descriptor (sd)
        if (n > 0)
        {
            write(sd, wbuffer, n);
        }
    
    msg = "1. Approve Loan\n2. Reject Loan\nAny other key to go back\nEnter your choice: ";
    write(sd, msg, strlen(msg));

    // Read user input
    getInputEC(sd, inputbuffer, BUFFER_SIZE);
        int mop = atoi(inputbuffer);
        if (mop==1) {
            // Call the approve loan function
            if (approveLoan(loanId) == 0) {
                snprintf(wbuffer, sizeof(wbuffer), "Loan %ld approved successfully.\n", loanId);
            } else {
                snprintf(wbuffer, sizeof(wbuffer), "Error approving loan %ld.\n", loanId);
            }
        } else if (mop == 2) {
            // Call the reject loan function
            if (rejectLoan(loanId) == 0) {
                snprintf(wbuffer, sizeof(wbuffer), "Loan %ld rejected successfully.\n", loanId);
            } else {
                snprintf(wbuffer, sizeof(wbuffer), "Error rejecting loan %ld.\n", loanId);
            }
        } else {
            // Any other input goes back
            snprintf(wbuffer, sizeof(wbuffer), "Going back to the previous menu...\n");
        }

        // Output the result
        write(sd, wbuffer, strlen(wbuffer));

}

void approve_reject_loans()
{
    // Implementation logic for approving or rejecting loans
}

void view_assigned_loan_applications(int sd, long empId)
{
    const char *msg = "All Assigned Loan Applications \n";
    write(sd, msg, strlen(msg));

   char wbuffer[BUFFER_SIZE];
    int count = MAX_FILES;
    LoanDTO loans[count];
    memset(loans, 0, sizeof(LoanDTO) * MAX_FILES);
    int op = read_loans_for_emp(loans, empId);
    if (op == -1)
    {
        msg = "Failed to read loan data.\n";
        write(sd, msg, strlen(msg));
    }

    for (int i = 0; i < op - 1; i++)
    {
        bzero(wbuffer, sizeof(wbuffer));
        int n = snprintf(wbuffer, sizeof(wbuffer),
         "Loan %d: LoanId: %ld, Type: %s, CustomerId: %ld, EmpId: %ld, Amount: %d, Interest: %d, Tenure: %d months, Amount Paid Back: %d, Status: %s\n",
         i + 1,                          
         loans[i].loanId,                 
         loans[i].type,                   
         loans[i].customerId,             
         loans[i].empId,                  
         loans[i].amount,                 
         loans[i].interest,               
         loans[i].tenure,                 
         loans[i].amountPayedBack,        
         loans[i].status); 

        // Use write to send the data through the socket descriptor (sd)
        if (n > 0)
        {
            write(sd, wbuffer, n);
        }
    }
    // Check the operation code
    if (op == 0)
    {   
        snprintf(wbuffer, BUFFER_SIZE, "There are more than %d number of loans\n", MAX_FILES);
        write(sd, wbuffer, strlen(wbuffer));
    }
}

void emp_view_customer_txns(int sd, long empId) {

        const char* msg;
        char input[BUFFER_SIZE];
        char wbuffer[BUFFER_SIZE];
      
        int count = 1024;
        TxnDTO txns[count];
        memset(txns, 0, sizeof(TxnDTO) * MAX_FILES);

        msg = "Enter Customer Id: ";
        write(sd, msg, strlen(msg));
        ssize_t bytesRead ;
        getInputEC(sd, input, BUFFER_SIZE);
        
        long customerId = atol(input);
          snprintf(wbuffer, sizeof(wbuffer), "Listing Last %d Transactions of Customer: %ld\n", MAX_FILES, customerId);
           write(sd, wbuffer, strlen(wbuffer));
        int op = read_txns_for_cust(txns, customerId);
        if (op == -1)
        {
            msg = "Failed to read transactions data.\n";
            write(sd, msg, strlen(msg));
        }

        char tsString[26];


        for (int i = 0; i < op - 1; i++)
        {
            ctime_r(&txns[i].timestamp, tsString);

            int n = snprintf(wbuffer, sizeof(wbuffer),
                            "Txn %d: TxnId: %ld, from Id: %ld, to Id: %ld, Amount: %d, timestamp: %s\n",
                            i + 1,
                            txns[i].txnId,
                            txns[i].fromCust,
                            txns[i].toCust,
                            txns[i].amount,
                            tsString);

       
            if (n > 0)
            {
                write(sd, wbuffer, n);
            }
        }
        // Check the operation code
        if (op == 0)
        {
            msg = "There are more than 1024 txns\n";
            write(sd, msg, strlen(msg));
        }
}


int employee_session(int sd, long empId)
{
    char option[BUFFER_SIZE];
    const char *msg = "Hello";
    if(get_employee_data(empId, &currEmployee)<0) {
        msg = "could not open employee";
        write(sd, msg, strlen(msg));
        char empid_char[MAX_USERNAME_LEN];
        snprintf(empid_char, MAX_USERNAME_LEN, "%ld", empId);
        logout(empid_char);

    }
    while (1)
    {
        writeECMsg(sd, "\n--- Employee Menu ---\n");
        writeECMsg(sd, "1. Add New Customer\n");
        writeECMsg(sd, " 2. View All Customers\n");
        writeECMsg(sd, " 3. View Customer\n");
        writeECMsg(sd, " 4. Modify Customer Details\n");
        writeECMsg(sd, " 5. View Customer Txns\n");
        writeECMsg(sd, " 6. View all Assigned Loan Applications\n");
        writeECMsg(sd, " 7. Process Loan Application\n");
        writeECMsg(sd, " 8. Change Password\n");
        writeECMsg(sd, " 9. Logout\n");
        writeECMsg(sd, " Enter your choice: ");

        // Reading input using the getInputEC() system call
        ssize_t bytes_read ;
        getInputEC(sd, option, sizeof(option));
         // Remove newline

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
            emp_view_customer_txns(sd, empId);
            
            break;
        case 6:
            view_assigned_loan_applications(sd, empId);
            break;
        case 7:
            process_loan_applications(sd);           
            break;
        case 8:
            {
            const char* msg = "Enter New Password:  ";
            char passwordBuffer[MAX_PASSWORD_LEN];
            write(sd, msg, strlen(msg));
            getInputEC(sd, passwordBuffer, MAX_PASSWORD_LEN);
            char empId_char[MAX_USERNAME_LEN];
            snprintf(empId_char, MAX_USERNAME_LEN, "%ld", empId);
            if(update_user_password(empId_char, passwordBuffer) < 0) {
                msg = "Error: Could not update Password\n";
                write(sd, msg, strlen(msg));
            } else {
                msg = "Success: Password Updated Successfully\n";
                write(sd, msg, strlen(msg));
            }

            break;
            }
        case 9:
            const char* msg = "Logging Out ...\n";
            write(sd, msg, strlen(msg));
             char empId_char[MAX_USERNAME_LEN];
            snprintf(empId_char, MAX_USERNAME_LEN, "%ld", empId);
            logout(empId_char);
            return 0;
        default:
            writeECMsg(sd, " Invalid option, please try again.\n");
            break;
        }
    }
}
