
#include "mgrController.h"
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

EmployeeDTO currManager;

void getInputMC(int sd, char *input, size_t size) {
    ssize_t bytesRead = read(sd, input, size);
        input[bytesRead - 1] = '\0';  
        if (input[bytesRead - 2] == '\r') {
        input[bytesRead - 2] = '\0';
    }
}

void mviewCustomer(int sd)
{
    char buffer[BUFFER_SIZE];
    CustomerDTO customer;
    // Ask for customer ID
    const char *msg = "Enter customer ID: ";
    write(sd, msg, strlen(msg)); // Send the prompt to the socket
    getInputMC(sd, buffer, sizeof(buffer));       // Read customer ID input
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

void mviewAllCustomers(int sd)
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

void view_unassigned_loans(int sd) {
    const char *msg = "All UnAssigned Loan Applications \n";
    write(sd, msg, strlen(msg));

   char wbuffer[BUFFER_SIZE];
    int count = MAX_FILES;
    LoanDTO loans[count];
    memset(loans, 0, sizeof(LoanDTO) * MAX_FILES);
    int op = read_loans_for_emp(loans, -1);
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

void activate_or_deactivate_customer(int sd) {
    const char* msg;
    char wbuffer[BUFFER_SIZE];
    char inputbuffer[BUFFER_SIZE];
    long custId;
    snprintf(wbuffer, sizeof(wbuffer), "Enter customer ID to activate/deactivate: ");
    write(sd, wbuffer, strlen(wbuffer));

    ssize_t bytesRead ;
    getInputMC(sd, inputbuffer, BUFFER_SIZE); 
    custId = atol(inputbuffer);
    int op = activate_deactivate_customer(custId);
    if(op == -2) {
        snprintf(wbuffer, sizeof(wbuffer), "Customer ID : %ld is Invalid\n", custId);
        

    } else if (op == -1){
        snprintf(wbuffer, sizeof(wbuffer), "Error: Customer ID : %ld could not be activated/deactivated\n", custId);
    } else {
        snprintf(wbuffer, sizeof(wbuffer), "Customer ID : %ld Status flipped Successfully", custId);
    }

       
    write(sd, wbuffer, strlen(wbuffer));
}

void assign_loan_application(int sd) {
    const char* msg;
    char wbuffer[BUFFER_SIZE];
    char inputbuffer[BUFFER_SIZE];
    long loanId;
    long empId;

    snprintf(wbuffer, sizeof(wbuffer), "Enter loan application ID to assign: ");
    write(sd, wbuffer, strlen(wbuffer));

    ssize_t bytesRead ;
    getInputMC(sd, inputbuffer, BUFFER_SIZE);

    loanId = atol(inputbuffer);

    bzero(inputbuffer, sizeof(inputbuffer));
    snprintf(wbuffer, sizeof(wbuffer), "Enter Employee ID to assign: ");
    write(sd, wbuffer, strlen(wbuffer));
    
    getInputMC(sd, inputbuffer, BUFFER_SIZE);
    inputbuffer[bytesRead - 1] = '\0';  
    empId = atol(inputbuffer);
    
    int op = assignLoan(empId, loanId);
     if(op == -3) {
        snprintf(wbuffer, sizeof(wbuffer), "Loan ID : %ld is Invalid\n", loanId);
        

    } else if (op == -2){
        snprintf(wbuffer, sizeof(wbuffer), "Error: Employee ID : %ld is Invalid\n", empId);
    } else if (op == -1){
        snprintf(wbuffer, sizeof(wbuffer), "Error: Loan ID : %ld could not be assigned to Employee: %ld\n", loanId, empId);
    } 
    
    else {
                snprintf(wbuffer, sizeof(wbuffer), "Success: Loan ID : %ld assigned to Employee: %ld\n", loanId, empId);
    }


    write(sd, wbuffer, strlen(wbuffer));
    
}


int mgr_session(int sd, long empId) {
    char wbuffer[BUFFER_SIZE];
    char inputbuffer[BUFFER_SIZE];
    ssize_t bytesRead;
    const char* msg;
    while (1) {

        
        msg =  "\n--- Bank Controller Menu ---\n";
        write(sd, msg, strlen(msg));
        
        msg = "1. View all Customer Accounts\n";
        write(sd, msg, strlen(msg));
        
        msg = "2. View Customer\n";
        write(sd, msg, strlen(msg));
        
        msg = "3. Activate/Deactivate Customer Accounts\n";
        write(sd, msg, strlen(msg));

        msg = "4. View Unassigned Loans Application\n";
        write(sd, msg, strlen(msg));

        msg =  "5. Assign Loan Application Processes to Employees\n";
        write(sd, msg, strlen(msg));

        msg = "6. Review Customer Feedback\n";
        write(sd, msg, strlen(msg));

        msg = "7. Change Password\n";
        write(sd, msg, strlen(msg));

        msg =  "8. Logout\n";
        write(sd, msg, strlen(msg));

        msg =  "Enter your choice: ";
        write(sd, msg, strlen(msg));

        // Read user input
         getInputMC(sd, inputbuffer, BUFFER_SIZE);
          // Remove newline
        int menuOp = atoi(inputbuffer);
            switch (menuOp) {
                case 1:
                    mviewAllCustomers(sd);
                    break;
                case 2:
                    mviewCustomer(sd);
                    break;
                case 3:
                    activate_or_deactivate_customer(sd);
                    break;
                case 4: view_unassigned_loans(sd);
                    break;
                case 5:
                    assign_loan_application(sd);
                    break;
                case 6:
                    // review_customer_feedback(sd);
                    break;
                case 7:{
                        const char* msg = "Enter New Password:  ";
                        char passwordBuffer[MAX_PASSWORD_LEN];
                        write(sd, msg, strlen(msg));
                        getInputMC(sd, passwordBuffer, MAX_PASSWORD_LEN);
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
                case 8:
                     const char* msg = "Logging Out ...\n";
                    write(sd, msg, strlen(msg));
                    char empId_char[MAX_USERNAME_LEN];
                    snprintf(empId_char, MAX_USERNAME_LEN, "%ld", empId);
                    logout(empId_char);
                    return 0;
                    
                default:
                    snprintf(wbuffer, sizeof(wbuffer), "Invalid option, please try again.\n");
                    write(sd, wbuffer, strlen(wbuffer));
                    break;
            
        }
    }
}
