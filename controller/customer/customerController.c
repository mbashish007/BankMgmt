#include "customerController.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../admin/admin.h" // Include action.h where you define all functions
#include "../../login/login.h"
#include "../../sessions/sessions.h"
#include "../../utils/file/fileUtils.h"
#include "../../utils/loan/loan.h"
#include "../../utils/transaction/transaction.h"
#include "../../service/employee/employeeService.h"
#include "../../service/customer/customerService.h"

#include "../../Structs/txnDTO.h"
#include "../../Structs/loanDTO.h"

#define ADM_DIR "./data/users/admin/"
#define CUST_DIR "./data/users/customer/"
#define EMP_DIR "./data/users/employee/"
#define MGR_DIR "./data/users/manager/"
#define MAX_FILES 1024

CustomerDTO currCustomer;

void getInputCC(int sd, char *input, size_t size) {
    ssize_t bytesRead = read(sd, input, size);
        input[bytesRead - 1] = '\0';  
        if (input[bytesRead - 2] == '\r') {
        input[bytesRead - 2] = '\0';
    }
}

void view_balance(int sd, long customerId) {
    const char *msg;
    char wbuffer[BUFFER_SIZE];
    int balance ;
    memset(&currCustomer, 0, sizeof(CustomerDTO));
    
    if(get_customer_data(customerId, &currCustomer) < 0) {
        msg = "Error getting balance.\n";
        write(sd, msg, strlen(msg));
        return;

    }

    snprintf(wbuffer, sizeof(wbuffer), "Amount Balance: %d for customer %s\n", currCustomer.balance, currCustomer.name);
    write(sd, wbuffer, strlen(wbuffer));
}


void deposit_money(int sd, long customerId) {
    const char *msg;
    char wbuffer[BUFFER_SIZE];
    int depositAmount;

    msg = "Enter amount to deposit: ";
    write(sd, msg, strlen(msg));

    char input[BUFFER_SIZE];
    ssize_t bytesRead ;
    getInputCC(sd, input, BUFFER_SIZE);
     // Remove newline
    
    depositAmount = atoi(input);

    int op = depositMoney(customerId, &currCustomer,depositAmount);
    if(op == 0)
    snprintf(wbuffer, sizeof(wbuffer), "Deposited: %d. New balance: %d\n", depositAmount, currCustomer.balance);

    else 
      snprintf(wbuffer, sizeof(wbuffer), "Error Depositing\n");
    write(sd, wbuffer, strlen(wbuffer));
}

void withdraw_money(int sd, long customerId) {
    const char *msg;
    char wbuffer[BUFFER_SIZE];
    int wAmount;

    msg = "Enter amount to withdraw: ";
    write(sd, msg, strlen(msg));

    char input[BUFFER_SIZE];
    ssize_t bytesRead ;
    getInputCC(sd, input, BUFFER_SIZE);
   
    
    wAmount = atoi(input);

    memset(&currCustomer, 0, sizeof(CustomerDTO));
    int op = withdrawMoney(customerId, &currCustomer, wAmount);
    if(op == 0)
        snprintf(wbuffer, sizeof(wbuffer), "Successfully Withdrawn: %d. New balance: %d\n", wAmount, currCustomer.balance);
    else if(op == -3)
        snprintf(wbuffer, sizeof(wbuffer), "Error: InSufficient Balance\n");
    
      else if(op == -1)
        snprintf(wbuffer, sizeof(wbuffer), "Error: Withdrawing\n");

    write(sd, wbuffer, strlen(wbuffer));
}

void transfer_funds(int sd, long customerId) {
    const char *msg;
    char wbuffer[BUFFER_SIZE];
    char recipientUserIdBuff[MAX_NAME_LEN];
    long recipientId;
    int tAmount;

    msg = "Enter recipient userId: ";
    write(sd, msg, strlen(msg));

    // Read recipient username
    ssize_t bytesRead ;
     getInputCC(sd, recipientUserIdBuff, MAX_NAME_LEN);
   
       
    

    recipientId = atol(recipientUserIdBuff);

    msg = "Enter amount to transfer: ";
    write(sd, msg, strlen(msg));

    // Read transfer amount
    getInputCC(sd, wbuffer, BUFFER_SIZE);

    tAmount = atoi(wbuffer);
    int op = transferMoney(customerId, recipientId, tAmount);
    
    if(op == -1) 
        msg = "Error: Could not transfer amount\n";
    
    else if(op == -2) 
        msg = "Error: Invalid Customer Id \n";
    
    else if(op == -3)
        msg = "Error: Insufficient Balance.\n";
    
    else if(op == -4)
        msg = "Error: Inactive recepient. \n";
    
    else if(op == 0)
        msg = "Success: Transfer Funds\n";

    write(sd, msg, strlen(msg));
}

void apply_for_loan(int sd, long customerId) {
    const char *msg;
    LoanDTO newLoan;
    char wbuffer[BUFFER_SIZE];

    // Prompt for customerId
    
    newLoan.customerId = customerId;

    //Initially assigned to no one
    newLoan.empId = -1;

    msg = "Enter Loan Type (1/2/3): 1.House Loan 2. Car Loan 3. Personal Loan: ";
    write(sd, msg, strlen(msg));
    getInputCC(sd, wbuffer, BUFFER_SIZE);
    int ltype = atoi(wbuffer);
    if(ltype == 1) strncpy(newLoan.type, LOAN_TYPE_HOUSE, sizeof(LOAN_TYPE_HOUSE));
    else if(ltype == 2) strncpy(newLoan.type, LOAN_TYPE_CAR, sizeof(LOAN_TYPE_CAR));
    else if(ltype == 3) strncpy(newLoan.type, LOAN_TYPE_PERSONAL, sizeof(LOAN_TYPE_PERSONAL));
    else {
        msg = "Error Invalid Loan type\n";
        write(sd, msg, strlen(msg));
        return;
    }
    // Prompt for loan amount
    msg = "Enter Loan Amount: ";
    write(sd, msg, strlen(msg));
    getInputCC(sd, wbuffer, BUFFER_SIZE);
    newLoan.amount = atoi(wbuffer);

    // Prompt for interest rate
    msg = "Enter Interest Rate: ";
    write(sd, msg, strlen(msg));
    getInputCC(sd, wbuffer, BUFFER_SIZE);
    newLoan.interest = atoi(wbuffer);

    // Prompt for loan tenure (in months)
    msg = "Enter Tenure in Months : ";
    write(sd, msg, strlen(msg));
    getInputCC(sd, wbuffer, BUFFER_SIZE);
    newLoan.tenure = atoi(wbuffer);

    // Initially 0
    newLoan.amountPayedBack = 0;
    strcpy(newLoan.status, LOAN_STATUS_UNASSIGNED);
 

    if(createLoan(&newLoan)<0) {
 
        snprintf(wbuffer, sizeof(wbuffer),"Error Creating Loan Application\n");

    }else {
        snprintf(wbuffer, sizeof(wbuffer),"Loan Id: %ld Application Applied Successfully\n", newLoan.loanId);
    }
    
    write(sd, wbuffer, strlen(wbuffer));
}

void view_applied_loans(int sd, long customerId) {
     const char *msg = "All Applied Loan Applications \n";
    write(sd, msg, strlen(msg));

   char wbuffer[BUFFER_SIZE];
    int count = MAX_FILES;
    LoanDTO loans[count];
    memset(loans, 0, sizeof(LoanDTO) * MAX_FILES);
    int op = read_loans_for_cust(loans, customerId);
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

void change_password(int sd, long customerId) {
    const char *msg;
    char wbuffer[BUFFER_SIZE];
    char newPassword[MAX_PASSWORD_LEN];

    msg = "Enter new password: ";
    write(sd, msg, strlen(msg));

    ssize_t bytesRead ;
    getInputCC(sd, newPassword, MAX_PASSWORD_LEN);
    
    
    char custId_char[MAX_USERNAME_LEN];
    snprintf(custId_char, MAX_USERNAME_LEN, "%ld", customerId);
    if(update_user_password(custId_char, newPassword) < 0) {
        msg = "Error: Password could not be changed.\n";
        write(sd, msg, strlen(msg));
        return ;
    }


    snprintf(wbuffer, sizeof(wbuffer), "Password changed successfully.\n");
    write(sd, wbuffer, strlen(wbuffer));
}

void add_feedback(int sd, long customerId) {
    char wbuffer[BUFFER_SIZE];
    snprintf(wbuffer, sizeof(wbuffer), "Feedback submitted for customer: %s\n", currCustomer.name);
    write(sd, wbuffer, strlen(wbuffer));
}

void view_transaction_history(int sd, long customerId) {
    
        printf("%ld",customerId);
        const char *msg = "Listing Last 1024 Customers Transactions\n";
        write(sd, msg, strlen(msg));
        char wbuffer[BUFFER_SIZE];
        int count = 1024;
        TxnDTO txns[count];
        memset(txns, 0, sizeof(TxnDTO) * MAX_FILES);
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





int customer_session(int sd, long customerId) {
    char wbuffer[BUFFER_SIZE];
    int choice;

    memset(&currCustomer, 0, sizeof(CustomerDTO));
    const char *msg = "Hello";
    if(get_customer_data(customerId, &currCustomer)<0) {
        msg = "could not open employee";
        write(sd, msg, strlen(msg));
        char custid_char[MAX_USERNAME_LEN];
        snprintf(custid_char, MAX_USERNAME_LEN, "%ld", customerId);
        logout(custid_char);
    }

    while(1) {
    //printing the menu
    snprintf(wbuffer, sizeof(wbuffer),
            "\n--- Customer Menu ---\n"
            "1. View Balance\n 2. Deposit Money\n");
    write(sd, wbuffer, strlen(wbuffer));

    snprintf(wbuffer, sizeof(wbuffer),
            "3. Withdraw Money\n"
            "4. Transfer Funds\n");
    write(sd, wbuffer, strlen(wbuffer));

    snprintf(wbuffer, sizeof(wbuffer),
            "5. Apply for a Loan\n"
            "6. View all loans Applied\n"
            );
            
    write(sd, wbuffer, strlen(wbuffer));

    snprintf(wbuffer, sizeof(wbuffer),
            "7. View Transaction History\n"
            "8. Add Feedback\n"
           );
    write(sd, wbuffer, strlen(wbuffer));

    snprintf(wbuffer, sizeof(wbuffer),
            "9. Change Password\n"
            "10. Logout\n Enter choice: ");
    write(sd, wbuffer, strlen(wbuffer));

    // Read the user's choice
    char input[BUFFER_SIZE];
    ssize_t bytesRead ;
    getInputCC(sd, input, BUFFER_SIZE);
     // Remove newline
    
    choice = atoi(input);  // Convert choice to integer

    // Handle user choice
    switch (choice) {
        case 1: view_balance(sd, customerId);
                break;
        case 2:
            deposit_money(sd, customerId);
            break;
        case 3:
            withdraw_money(sd, customerId);
            break;
        case 4:
            transfer_funds(sd, customerId);
            break;
        case 5:
            apply_for_loan(sd, customerId);
            break;
        case 6:
            view_applied_loans(sd, customerId);
            break;
        case 9:
            change_password(sd, customerId);
            break;
        case 8:
            add_feedback(sd, customerId);
            break;
        case 7:
            view_transaction_history(sd, customerId);
            break;
        case 10:
            char custId_char[MAX_USERNAME_LEN];
            snprintf(custId_char, MAX_USERNAME_LEN, "%ld", customerId);
            logout(custId_char);
            return 0;
        default:
            snprintf(wbuffer, sizeof(wbuffer), "Invalid choice. Please try again.\n");
            write(sd, wbuffer, strlen(wbuffer));
            break;
        }
    }
}
