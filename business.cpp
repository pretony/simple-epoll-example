#include <memory>
#include "business.h"
#include "json/json.h"

void do_add()
{
    printf("%s\n", __FUNCTION__);
}

void do_del()
{
    printf("%s\n", __FUNCTION__);
}

void do_exit(ClientServer *clientServer)
{
    printf("%s\n", __FUNCTION__);
    clientServer->bExit = true;
}

bool process_business_real(ClientServer *clientServer, const char *data, size_t len)
{
    Json::CharReaderBuilder b;
    std::unique_ptr<Json::CharReader> reader(b.newCharReader());
    JSONCPP_STRING errs;
    Json::Value root;
    bool ok = reader->parse(data, data + len, &root, &errs);
    if(ok && (errs.size() == 0))
    {
        BusinessCmd cmd = (BusinessCmd)root[0]["Cmd"].asInt();
        switch(cmd)
        {
        case BusinessCmd::Business_ADD:
            do_add();
            break;
        case BusinessCmd::Business_DEL:
            do_del();
            break;
        case BusinessCmd::Business_EXIT:
            do_exit(clientServer);
            break;
        default:
            break;
        }
    }

    return true;
}
