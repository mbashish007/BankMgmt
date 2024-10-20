#ifndef LOAN_UTIL_H
#define LOAN_UTIL_H

#include "../../Structs/loanDTO.h"

/**
 * 0 success
 * -1 error
 */
int createLoan(LoanDTO *) ;

/**
 * 0 success
 * -1 error
 * -2 invalid argument for id
 */
int get_loan_data(long , LoanDTO *) ;

/**
 * 0 more than 1024
 * >0 no of elements +1
 * -1 error
 */
int read_loans_data(LoanDTO *) ;

/**
 * 0 more than 1024
 * >0 no of elements +1
 * -1 error
 */
int read_loans_for_cust(LoanDTO *, long ) ;

/**
 * 0 more than 1024
 * >0 no of elements +1
 * -1 error
 */
int read_loans_for_emp(LoanDTO *, long ); 

#endif