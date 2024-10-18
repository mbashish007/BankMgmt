#ifndef TXN_DTO_H
#define TXN_DTO_H

#include <time.h>

typedef struct {
    long txnId;
    long toCust;
    long fromCust;
    int amount;
    time_t timestamp;

} TxnDTO;

#endif