#ifndef TXN_UTIL_H
#define TXN_UTIL_H

#define MAX_FILES 1024
#define MAX_FILENAME_LEN 256

#include "../../Structs/txnDTO.h"

/**
 * 0 success
 * -1 error
 */
int writeTXN(TxnDTO *);
/**
 * 0 success
 * -1 error
 * -2 invalid argument for id
 */
int get_txn_data(long , TxnDTO *);

/**
 * 0 more than 1024
 * >0 no of elements +1
 * -1 error
 */
int read_txns_data(TxnDTO *) ;
/**
 * 0 more than 1024
 * >0 no of elements +1
 * -1 error
 */
int read_txns_for_cust(TxnDTO *, long );

#endif