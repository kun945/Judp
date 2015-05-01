#ifndef __JUDP_H__
#define __JUDP_H__

#include <aJSON.h>
#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#define JUDP_WORKER_NAME_LEN            16
#define JUDP_BUFFER_LEN                 128
#define JUDP_WORKER_MAX                 10

#define JUDP_ERR_OK                     0
#define JUDP_ERR_JSONPARSE              1
#define JUDP_ERR_NONAME                 2
#define JUDP_ERR_NOWORKER               3
#define JUDP_ERR_LACK_PARAMETER         4
#define JUDP_ERR_PARAMETER              5
#define JUDP_ERR_CREATEJSON             6

#define JUDP_ERR_OK_STR                     "OK"
#define JUDP_ERR_JSONPARSE_STR              "parse error"
#define JUDP_ERR_NONAME_STR                 "no command"
#define JUDP_ERR_NOWORKER_STR               "unknown command"
#define JUDP_ERR_LACK_PARAMETER_STR         "lack of parameter"
#define JUDP_ERR_PARAMETER_STR              "parameter error"
#define JUDP_ERR_CREATEJSON_STR             "create reply fail"


#define JUDP_JOB_COMMON                 "common"
#define JUDP_JOB_DIGITALWRITE           "digitalWrite"
#define JUDP_JOB_DIGITALREAD            "digitalRead"
#define JUDP_JOB_ANALOGWRITE            "analogWrite"
#define JUDP_JOB_ANALOGREAD             "analogRead"
#define JUDP_JOB_PINMODE                "pinMode"

class Worker
{
    public:
        Worker(const char* name, aJsonObject* (*job)(aJsonObject*, const char*));
        bool isMyJob(const char* name);
        char* doJob(aJsonObject*, const char*);

    private:
        char _name[JUDP_WORKER_NAME_LEN];
        aJsonObject* (*_job)(aJsonObject*, const char*);
};

struct JList
{
    Worker* worker;
    struct JList* next;
};

class Judp
{
    public:
        Judp(int port);
        void begin();
        void processing();
        bool addWorker(Worker* worker);
        void sendto(IPAddress& ip, int port, byte* data, int len);

    private:
        int _port;
        JList _jlist;
        EthernetUDP _udp;
};

extern aJsonObject* createReplyJson(int errno, const char* msg);
#endif /*  __JUDP_H__ */
