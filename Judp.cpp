#include "Judp.h"
#include <stdlib.h>
#include <string.h>

//void freeMem(char* message);
aJsonObject* createReplyJson(int errno, const char* msg)
{
    aJsonObject* root = aJson.createObject();
    aJson.addNumberToObject(root, "errno", errno);
    if (NULL != msg) {
        aJson.addStringToObject(root, "msg", msg);
    }

    return root;
}

char* createReplyStr(int errno, const char* msg)
{
    char* string = NULL;
    aJsonObject* temp = createReplyJson(errno, msg);
    if (NULL == temp) {
        return NULL;
    }

    string = aJson.print(temp);
    aJson.deleteItem(temp);
#if 0
    Serial.print("reply str: ");
    Serial.println(string);
#endif /*DEBUG*/
    return string;
}

static aJsonObject* common(aJsonObject* root, const char* name)
{
    aJsonObject* para = NULL;
    aJsonObject* pin = NULL;
    aJsonObject* temp = NULL;
    aJsonObject* value = NULL;
    char* msg = JUDP_ERR_LACK_PARAMETER_STR;
    char* vstr = NULL;
    int errno = JUDP_ERR_LACK_PARAMETER;
    int p;
    int v;
    bool rv = false;

    para = aJson.getObjectItem(root, "para");
    if (NULL == para) goto err_out;

    pin = aJson.getObjectItem(para, "pin");
    if (NULL == pin) goto err_out;

    p = pin->valueint;
    if (strcmp(name, JUDP_JOB_DIGITALWRITE) == 0) {
        value = aJson.getObjectItem(para, "value");
        if (NULL == value) goto err_out;
        v = value->valueint;
        digitalWrite(p, v);
        rv = false;
    } else if (strcmp(name, JUDP_JOB_DIGITALREAD) == 0) {
        v = digitalRead(p);
        rv = true;
    } else if (strcmp(name, JUDP_JOB_ANALOGREAD) == 0) {
        v = analogRead(p);
        rv = true;
    } else if (strcmp(name, JUDP_JOB_ANALOGWRITE) == 0) {
        value = aJson.getObjectItem(para, "value");
        if (NULL == value) goto err_out;
        v = value->valueint;
        analogWrite(p, v);
        rv = false;
    } else if (strcmp(name, JUDP_JOB_PINMODE) == 0) {
        value = aJson.getObjectItem(para, "value");
        if (NULL == pin) goto err_out;
        vstr = value->valuestring;
        if (strcmp(vstr, "OUTPUT") == 0) {
            pinMode(p, OUTPUT);
        } else if (strcmp(vstr, "INPUT") == 0) {
            pinMode(p, INPUT);
        } else {
            errno = JUDP_ERR_PARAMETER;
            msg = JUDP_ERR_PARAMETER_STR;
            goto err_out;
        }
    } else {
        errno = JUDP_ERR_PARAMETER;
        msg = JUDP_ERR_PARAMETER_STR;
        goto err_out;
    }

    errno = JUDP_ERR_OK;
    msg = JUDP_ERR_OK_STR;
    aJson.deleteItem(root);

    temp = createReplyJson(errno, msg);
    if (NULL == temp) {
        errno = JUDP_ERR_CREATEJSON;
        msg = JUDP_ERR_CREATEJSON_STR;
        goto err_out;
    }

    if (rv) {
        aJsonObject* resp = aJson.createObject();
        if (NULL == resp) {
            errno = JUDP_ERR_CREATEJSON;
            msg = JUDP_ERR_CREATEJSON_STR;
            aJson.deleteItem(temp);
            goto err_out;
        }
        aJson.addItemToObject(temp, "resp", resp);
        aJson.addNumberToObject(resp, "value", v);
    }
    return temp;
err_out:
    aJson.deleteItem(root);
    return createReplyJson(errno, msg);
}


Worker worker_00(JUDP_JOB_COMMON, common);

Worker::Worker(const char* name, aJsonObject* (*job)(aJsonObject*, const char*))
{
   strncpy(_name, name, JUDP_WORKER_NAME_LEN - 1);
   _job = job;
}

bool Worker::isMyJob(const char* name)
{
    if ((!strcmp(_name, JUDP_JOB_COMMON)) && \
            (!strcmp(name, JUDP_JOB_DIGITALWRITE) || \
             !strcmp(name, JUDP_JOB_DIGITALREAD) || \
             !strcmp(name, JUDP_JOB_ANALOGWRITE) || \
             !strcmp(name, JUDP_JOB_ANALOGREAD) || \
             !strcmp(name, JUDP_JOB_PINMODE))) {
        return true;
    }
    if (strcmp(name, _name) == 0) {
        return true;
    } else {
        return false;
    }
}

char* Worker::doJob(aJsonObject* root, const char* name)
{
    aJsonObject* temp;
    temp = _job(root, name);
    if (NULL == temp) {
        return NULL;
    }

    char* string = aJson.print(temp);
    aJson.deleteItem(temp);
#if 0
    Serial.print("reply str: ");
    Serial.println(string);
#endif /*DEBUG*/
    return string;
}

Judp::Judp(int port)
{
    _port = port;
    _jlist.worker = NULL;
    _jlist.next = NULL;
}

void Judp::begin()
{
    this->addWorker(&worker_00);
    _udp.begin(_port);
}

void Judp::processing()
{
    int packetSize = _udp.parsePacket();
    if (packetSize) {
#if 0
        Serial.print("size: ");
        Serial.println(packetSize);
        Serial.print("From: ");
        IPAddress remote = _udp.remoteIP();
        for (int i =0; i < 4; i++)
        {
            Serial.print(remote[i], DEC);
            if (i < 3) {
                Serial.print(".");
            }
        }
        Serial.print(":");
        Serial.println(_udp.remotePort());
#endif /*DEBUG*/

        char* msg = JUDP_ERR_JSONPARSE_STR;
        char* reply = NULL;
        char buffer[JUDP_BUFFER_LEN];
        int errno = JUDP_ERR_JSONPARSE;
        int len = _udp.read(buffer, JUDP_BUFFER_LEN - 1);
        buffer[len] = '\0';
        JList* current = NULL;
        aJsonObject* name = NULL;
        aJsonObject* root = aJson.parse(buffer);
        if (NULL == root) goto reply_client;

        name = aJson.getObjectItem(root, "name");
        if (NULL == name) {
            errno = JUDP_ERR_NONAME;
            msg = JUDP_ERR_NONAME_STR;
            aJson.deleteItem(root);
            //goto delete_json;
            goto reply_client;
        }

        current = &_jlist;
        while (NULL != current) {
            if (current->worker->isMyJob(name->valuestring)) {
                break;
            }
            current = current->next;
        }
        if (NULL == current) {
            errno = JUDP_ERR_NOWORKER;
            msg = JUDP_ERR_NOWORKER_STR;
            aJson.deleteItem(root);
            //goto delete_json;
            goto reply_client;
        }

        reply = current->worker->doJob(root, name->valuestring);

//delete_json:
        //aJson.deleteItem(root);

reply_client:
        _udp.beginPacket(_udp.remoteIP(), _udp.remotePort());
        if (NULL == reply) {
            Serial.print("??????????????");
            reply = createReplyStr(errno, msg);
        }

        if (NULL != reply) {
            _udp.write(reply);
            free(reply);
        } else {
            _udp.write(JUDP_ERR_CREATEJSON_STR);
        }
        _udp.endPacket();
    }

    return;
}

void Judp::sendto(IPAddress& ip, int port, byte* data, int len)
{
    _udp.beginPacket(ip, port);
    _udp.write(data, len);
    _udp.endPacket();
    return;
}

bool Judp::addWorker(Worker* worker)
{
    JList* current = NULL;
    current = &_jlist;
    if (NULL == current->worker) {
        current->worker = worker;
        current->next = NULL;
        return true;
    }

    JList* new_node = (JList*)malloc(sizeof(JList));
    if (NULL == new_node) {
        return false;
    }

    new_node->worker = worker;
    new_node->next = NULL;
    while (NULL != current->next) {
        current = current->next;
    }
    current->next = new_node;
    return true;
}
