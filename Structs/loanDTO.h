#ifndef LOAN_DTO_H
#define LOAN_DTO_H

#define LOAN_TYPE_LEN 9
#define LOAN_STATUS_LEN 12
#define LOAN_STATUS_UNASSIGNED "unassigned"
#define LOAN_STATUS_ASSIGNED "assigned" 
#define LOAN_STATUS_APPROVED "approved"
#define LOAN_STATUS_REJECTED "rejected"

#define LOAN_TYPE_HOUSE "house"
#define LOAN_TYPE_CAR "car"
#define LOAN_TYPE_PERSONAL  "personal"

typedef struct 
{
    long loanId;
    char type [LOAN_TYPE_LEN];
    long customerId;
    long empId;
    int amount;
    int interest;
    int tenure;
    int amountPayedBack;
    char status[LOAN_STATUS_LEN];

    /* data */
}LoanDTO;


#endif
