//
// Created by Cody on 3/23/2020.
//

#ifndef A3_TRANSACTION_MSG_H
#define A3_TRANSACTION_MSG_H
#include <stdint.h>

enum txState {
    TX_NOTINUSE = 100,
    TX_INPROGRESS,
    TX_VOTING,
    TX_ABORTED,
    TX_COMMITTED
};

typedef enum {
    BEGIN_TX = 2000,
    JOIN_TX,
    COMMIT_TX,
    COMMIT_CRASH_TX,
    PREPARE_TX,
    ABORT_TX,
    ABORT_CRASH_TX,
    SUCCESS_TX,
    FAILURE_TX,
    POLL_STATE_TX
} txMsgKind;

typedef struct  {
    uint32_t   msgID;
    uint32_t   tid;        // Transaction ID
    uint32_t   state;      // State for polling
} txMsgType;

#endif //A3_TRANSACTION_MSG_H
