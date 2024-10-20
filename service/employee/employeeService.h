#ifndef EMP_SERVICE_H
#define EMP_SERVICE_H

#define MAX_FILES 1024
#define MAX_FILENAME_LEN 256
#include "../../Structs/employeeDTO.h"
int list_directory_files(const char* , char (*)[]) ;

/**
 * 0 more than 1024
 * >0 no of elements +1
 * -1 error
 */
int read_employees_data(EmployeeDTO *);

/**
 * 0 success
 * -1 error
 * -2 invalid argument for id
 */
int get_employee_data(long , EmployeeDTO *); 

/**
 * 0 success
 * -1 error
 * -2 invalid argument for id
 */
int approveLoan( long );

/**
 * 0 success
 * -1 error
 * -2 invalid argument for id
 */
int rejectLoan( long loanId) ;

/**
 * 
 * 0 success
 * -1 error
 * -2 invalid empId
 * -3 invalid loanId
 */
int assignLoan( long empId, long loanId) ;
#endif