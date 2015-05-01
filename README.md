Judp
===================


这是一个Arduino库，可以让你建立一个udp服务器，接收接收远程的json格式的信息来控制你的Arduino设备。

----------

Usage
-------------

这个库依赖**aJson**，需要Arduino的**以太网扩展模块**。

> **Note:**

> - aJson获取方式：https://github.com/interactive-matter/aJson.git
> - 以太网卡模块只测试过W5100。


初始化过程：
```
byte mac[] = {0x20, 0x14, 0x05, 0x23, 0x18, 0x17};
Judp server(6666);

void setup() {
    Ethernet.begin(mac);
    server.begin();
}

void loop() {
    server.processing();
}
```

库里集成基本GPIO操作，下面的是客户端发送的消息格式：

> - {"name":"digitalWrite", "para":{"pin":1, "value":0}}
> - {"name":"digitalRead", "para":{"pin":1}}
> - {"name":"analogWrite", "para":{"pin":1, "value":0}}
> - {"name":"analogRead", "para":{"pin":1}}
> - {"name":"pinMode", "para":{"pin":1, "value":"OUTPUT"}}        //or "value":"INPUT"

返回消息的格式：
> - {"errno":0, "msg":"ok"}
> - {"errno":0, "msg":"ok", "resp":{"value":1}}

自定义命令消息：
```
/**首先要定义一个命令调用的函数**/
static aJsonObject* test(aJsonObject* root, const char* name)
{
	char* msg = "OK";
    int errno = 0;
    
    //1解析你是用的参数
    //2是用参数执行你的命令
    //3构建并返回一个回复用的aJsonObject

err_out:
    return createReplyJson(errno, msg);
}

byte mac[] = {0x20, 0x14, 0x05, 0x23, 0x18, 0x17};
/*然后是用该函数创建一个Worker对象*/
Worker worker_01("test", test); //command {“name”:"test", [your parameters]}
Judp server(6666);

void setup() {
    Ethernet.begin(mac);
    /*最后将worker对象加入到server中*/
    server.addWorker(&worker_01);
    server.begin();
}

void loop() {
    server.processing();
}
```
Have Fun!
