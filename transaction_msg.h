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


static char* txMsgKindToStr(txMsgKind type) {
    switch (type) {
        case BEGIN_TX:
            return "BEGIN_TX";
        case JOIN_TX:
            return "JOIN_TX";
        case COMMIT_TX:
            return "COMMIT_TX";
        case COMMIT_CRASH_TX:
            return "COMMIT_CRASH_TX";
        case PREPARE_TX:
            return "PREPARE_TX";
        case ABORT_TX:
            return "ABORT_TX";
        case ABORT_CRASH_TX:
            return "ABORT_CRASH_TX";
        case SUCCESS_TX:
            return "SUCCESS_TX";
        case FAILURE_TX:
            return "FAILURE_TX";
        case POLL_STATE_TX:
            return "POLL_STATE_TX";
        default:
            return "INVALID";
    }
}

#endif //A3_TRANSACTION_MSG_H
