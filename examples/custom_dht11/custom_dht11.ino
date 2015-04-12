#include <Judp.h>
#include <aJSON.h>
#include <SPI.h>
#include <dht11.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#define DEBUG

dht11 DHT11;

static aJsonObject* environment(aJsonObject* root, const char* name)
{
    aJsonObject* para = NULL;
    aJsonObject* pin = NULL;
    aJsonObject* temp = NULL;
    aJsonObject* resp = NULL;
    char* msg = "OK";
    int errno = 0;
    int p;
    int chk;

    para = aJson.getObjectItem(root, "para");
    if (NULL == para) {
        errno = JUDP_ERR_NOPARA;
        msg = JUDP_ERR_NOPARA_STR;
        goto err_out;
    }

    pin = aJson.getObjectItem(para, "pin");
    if (NULL == pin) {
        errno = JUDP_ERR_PARA;
        msg = JUDP_ERR_PARA_STR;
        goto err_out;
    }

    p = pin->valueint;
    chk = DHT11.read(p);
    //Serial.print("Read sensor: ");
    switch (chk)
    {
    case DHTLIB_OK:
        //Serial.println("OK");
        break;
    case DHTLIB_ERROR_CHECKSUM:
        //Serial.println("Checksum error");
        msg = "Checksum error";
        errno = 255;
        goto err_out;
        break;
    case DHTLIB_ERROR_TIMEOUT:
        //Serial.println("Time out error");
        msg = "Time out error";
        errno = 255;
        goto err_out;
        break;
    default:
        //Serial.println("Unknown error");
        msg = "Unknown error";
        errno = 255;
        goto err_out;
        break;
    }

    temp = createReplyJson(errno, msg);
    if (NULL == temp) {
        errno = JUDP_ERR_CREATEJSON;
        msg = JUDP_ERR_CREATEJSON_STR;
        goto err_out;
    }

    resp = aJson.createObject();
    if (NULL == resp) {
        errno = JUDP_ERR_CREATEJSON;
        msg = JUDP_ERR_CREATEJSON_STR;
        aJson.deleteItem(temp);
        goto err_out;
    }

    aJson.addItemToObject(temp, "resp", resp);
    aJson.addNumberToObject(resp, "h", double(DHT11.humidity));
    aJson.addNumberToObject(resp, "t", double(DHT11.temperature));
    return temp;
err_out:
    return createReplyJson(errno, msg);
}


byte mac[] = {0x20, 0x14, 0x05, 0x23, 0x18, 0x17};
Judp server(6666);
Worker worker_01("environment", environment);

void setup() {
    Serial.begin(115200);
    Ethernet.begin(mac);
    server.addWorker(&worker_01);
    server.begin();
}

void loop() {
    server.processing();
}
