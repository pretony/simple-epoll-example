#ifndef _BUSINESS_H
#define _BUSINESS_H

#include "clientServer.h"

enum class BusinessCmd {
    Business_NONE = 0,
    Business_ADD,
    Business_DEL,
    Business_EXIT,
};
bool process_business_real(ClientServer *clientServer, const char *data, size_t len);


#endif
