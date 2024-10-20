#ifndef CUSTOMER_SERVICE_H
#define CUSTOMER_SERVICE_H

#define MAX_FILES 1024
#define MAX_FILENAME_LEN 256

#include "../../Structs/customerDTO.h"

int add_new_customer(CustomerDTO *);

// Function to modify customer details
/**
 * returns
 * -2 invalid ID
 * -1 error
 * 0 success
 */
int modify_customer_details(long , CustomerDTO *) ;

/**
 * 0 success
 * -1 error
 * -2 invalid argument for id
 */
int get_customer_data(long , CustomerDTO *);

/**
 * 0 more than 1024
 * >0 no of elements +1
 * -1 error
 */
int read_customers_data(CustomerDTO *);

/**
 * 0 success 
 * -1 error
 * 
 */
int depositMoney(long custId, CustomerDTO* customer, int damount) ;

/**
 * 0 success 
 * -1 error
 * -3 insufficient balance
 */
int withdrawMoney(long custId, CustomerDTO* customer, int damount);

/**
 * 0 success 
 * -1 error
 * -3 insufficient balance
 * -4 Inactive recepient
 */
int transferMoney(long fromCustId,long toCustId,  int damount) ;

/**
 * 0 success 
 * -1 error
 * -2 Invalid cust id
 */
int processLoanMoney(long custId, long loanId, CustomerDTO* customer, int damount) ;

int activate_deactivate_customer(long custId) ;
#endif