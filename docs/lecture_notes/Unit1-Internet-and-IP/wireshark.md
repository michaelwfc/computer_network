### wireshark

[网络顶级掠食者 Wireshark 抓包从入门到实战](https://www.bilibili.com/video/BV12X6gYUEqA/?spm_id_from=333.337.search-card.all.click&vd_source=b3d4057adb36b9b243dc8d7a6fc41295)

[www.wireshark.org](https://www.wireshark.org/download.html)

1. Request web page from www.cs.brown.edu
   check the IP address

2. Open wireshark
   1. 选择 网卡
   2. filter:
      tcp.port == 80 && ip.addr == 128.148.32.12

      tcp.port == 7897 && ip.addr == 128.148.32.12

3. open browser and request `www.cs.brown.edu` or `curl www.cs.brown.edu`
    issue1. 浏览器refresh 没有效果

![TCP-handshakes](../../../images/transport_layer/TCP-handshakes.png)
![TCP-handshakes-wireshark](../../../images/transport_layer/TCP-handshakes-wireshark.png)

[ssl-handshakes](https://www.ruanyifeng.com/blog/2014/09/illustration-ssl.html)
