# 1. 网络基本知识
## Q1.1: 目前流行的 sci-network 有哪些？
大致的流程是怎么样的，优缺点是什么，其他需要注意的事项，把这些描述总结为一张表

下面从**网络技术角度**介绍目前常见的加密隧道和代理技术。为了避免涉及针对特定审查系统的规避操作，重点放在协议特性、系统架构和适用场景上。

### 常见技术方案对比（2026）

| 方案               | 工作层     | 大致流程                                          | 优点         | 缺点                | 适用场景       | 客户端             | 服务端              |
| ---------------- | ------- | --------------------------------------------- | ---------- | ----------------- | ---------- | --------------- | ---------------- |
| SOCKS5 代理        | 应用层     | App → SOCKS5 → Proxy Server → Internet        | 简单、兼容性高    | 无加密，需要外层TLS/VPN保护 | 开发调试       | Clash、Firefox   | Dante            |
| HTTP CONNECT     | 应用层     | Browser → HTTP Proxy → Internet               | 浏览器支持好     | 主要支持TCP           | Web访问      | 浏览器             | Squid            |
| Shadowsocks 2022 | 应用层     | App → SS Client → SS Server → Internet        | 配置简单，性能高   | 流量模式较固定           | 个人代理       | Mihomo、sing-box | Shadowsocks-rust |
| Trojan           | 应用层     | App → Trojan Client → TLS → Server → Internet | 与正常HTTPS相似 | 需要TLS配置           | 远程访问       | sing-box        | Trojan-go        |
| VLESS            | 应用层     | App → VLESS Client → TLS → Server → Internet  | 开销较小       | 配置稍复杂             | 通用代理       | Xray、sing-box   | Xray-core        |
| Hysteria2        | 应用层     | App → QUIC → Server → Internet                | 高丢包环境性能优秀  | 依赖UDP             | 跨洲连接       | sing-box        | Hysteria         |
| TUIC             | 应用层     | App → QUIC → Server → Internet                | 延迟低        | 生态较小              | 游戏、实时应用    | sing-box        | TUIC             |
| WireGuard        | 网络层 VPN | IP Packet → WG Tunnel → Server → Internet     | 性能极高，内核支持  | 分流规则能力较弱          | 企业VPN、家庭组网 | WireGuard       | WireGuard        |
| OpenVPN          | 网络层 VPN | IP Packet → OpenVPN → Server                  | 成熟稳定       | CPU占用较高           | 企业环境       | OpenVPN         | OpenVPN          |
| IPSec            | 网络层 VPN | IP Packet → IPSec → Gateway                   | 企业标准       | 配置复杂              | 企业互联       | 系统内置            | StrongSwan       |

---

### 典型数据流

#### ① SOCKS5

```text
Chrome
   │
SOCKS5
   │
127.0.0.1:7890
   │
Proxy Server
   │
Internet
```

---

#### ② TUN + 代理核心

```text
Chrome
Steam
VSCode

    │

Windows TCP/IP Stack

    │

Wintun

    │

Mihomo

    │

Proxy Protocol

    │

Remote Server

    │

Internet
```

---

#### ③ WireGuard VPN

```text
Application

    │

IP Packet

    │

tun0

    │

WireGuard

    │

UDP

    │

WireGuard Server

    │

Internet
```

---

##### TUN 是否必须？

不是。

| 模式           | 是否需要TUN |
| ------------ | ------- |
| SOCKS5       | ❌       |
| HTTP Proxy   | ❌       |
| WireGuard    | ✅       |
| OpenVPN      | ✅       |
| Mihomo TUN模式 | ✅       |
| 浏览器手工代理      | ❌       |

TUN 的主要价值是：

* 不需要给每个程序单独配置代理
* 支持 UDP
* 可以接管整个系统流量

---

### 当前技术趋势（2026）

大多数现代客户端都开始融合 VPN 和代理能力。

常见客户端：

| 客户端             | 核心       |
| --------------- | -------- |
| Clash Verge Rev | Mihomo   |
| Hiddify         | sing-box |
| Nekoray         | sing-box |
| v2rayN          | Xray     |

---

### 从工程角度如何选择

| 需求           | 推荐方案                  |
| ------------ | --------------------- |
| 学习 VPN 原理    | WireGuard             |
| 学习 Linux 网络栈 | WireGuard + TUN       |
| 企业远程办公       | WireGuard / IPSec     |
| 家庭 NAS 远程访问  | WireGuard + Tailscale |
| 研究代理系统实现     | Mihomo / sing-box     |
| 研究 QUIC 技术   | Hysteria2 / TUIC      |

---

### 需要注意的事项

**安全性**

* 不要使用来源不明的代理软件。
* 定期更新客户端和服务端软件。
* 使用公钥认证登录服务器，禁用密码登录。

**性能**

* VPN 通常处理完整 IP 包，协议开销较低。
* 应用层代理通常具有更灵活的分流能力。
* QUIC 类协议对高延迟、高丢包网络更友好。

**运维**

* VPS 并不等于云平台。
* 公有云（AWS、GCP、Azure）提供 VPC、IAM、负载均衡等完整能力。
* 廉价 VPS 更适合学习 Linux、SSH、iptables、WireGuard 等基础网络技术。

如果以 CS144 的视角来看，这些技术最终都可以归结为一个问题：

> **应用产生的 TCP 字节流，最终是如何被封装、加密、转发，并在远端恢复成原始网络通信的？**

这是理解现代 VPN 和代理系统实现的核心。


## Q2: 什么是 VPN？

VPN : Virtual Private Network
中文：虚拟专用网络
VPN 的目标： 让两台机器看起来像在同一个局域网。
核心思想： 在公网之上构建一个私有 IP 网络。


例如：

```text
Home PC : 10.0.0.2
         Internet (encrypted tunnel)
Office Server: 10.0.0.1
```
虽然隔着互联网。但看起来像：

```
10.0.0.2 <----局域网----> 10.0.0.1
```
---

### VPN 传输的是什么？

VPN 传输的是： IP Packet

例如：
```
IP
 ├── TCP
 │    └── HTTP
 └── UDP
```
整个 IP 包被封装。


### VPN 的核心流程

#### 给客户端分配 IP ： 10.0.0.2

#### 传输 IP 包
不是 TCP, 不是 SOCKS5, 而是 完整 IP packet

#### 建立私有网络

客户端之间互通， 服务器可访问客户端

### WireGuard VPN

客户端

```text
10.0.0.2
```

服务器

```text
10.0.0.1
```

访问

```text
10.0.0.1:22
```

就像在家里访问

```text
192.168.1.x
```

一样。

---


## Q3: VPN 和节点代理有什么区别？

这是最重要的问题。

### VPN

```text
Browser
↓
TCP/IP
↓
tun0
↓
WireGuard
↓
VPN Server
↓
Internet
```

客户端认为：VPN Server 就是默认网关 

### 代理节点

```text
Chrome
↓
SOCKS5
↓
Mihomo
↓
Proxy Protocol
↓
Server
↓
Internet
```

---

### 本质区别

VPN:    
工作在网络层 
发送的是完整IP包： 传输单位 = IP Packet
```
Application
    ↓
TCP/IP
    ↓
WireGuard
    ↓
Internet
```


代理:    
工作在应用层
发送的是：TCP流/UDP流 , 传输单位 = TCP/UDP Stream,不是完整 IP 包。
```
Application
    ↓
SOCKS5
    ↓
Proxy
    ↓
Internet
```

---

#### 举例

访问 youtube.com

VPN:
```
Client
    |
IP Packet
    |
WireGuard
    |
Server
    |
Google
```

客户端发 IP to dst=142.250.xxx
服务器收到 完整 IP 包


代理:
```
Client
    |
CONNECT www.google.com:443
    |
Proxy
    |
Google
```
客户端发 CONNECT youtube.com:443
服务器根本看不到客户端的 IP 包。服务器自己建立 TCP

---

## Q4: 什么是 WireGuard？

WireGuard 是一种 VPN 协议。

可以理解成：

```text
IP Packet
↓
ChaCha20
↓
UDP
↓
Internet
```

---

WireGuard 作者目标：

> 替代 IPSec

> 替代 OpenVPN

---

WireGuard 特点

代码量小

约四万行

性能高

Linux 内核支持

配置简单

使用 Noise 协议

ChaCha20

Poly1305

---

### 工作流程

客户端 : 10.0.0.2 发送 TCP SYN to dst=8.8.8.8

WireGuard : 加密, 封装 UDP to dst=203.0.113.5 , port=51820

服务器: 解密, 恢复原始包, src=10.0.0.2, dst=8.8.8.8


### 现在 VPN 都是 WireGuard 吗？

不是。

常见 VPN

| 协议         | 状态     |
| ---------- | ------ |
| OpenVPN    | 仍然很多   |
| IPSec      | 企业大量使用 |
| SSL VPN    |           |
| Cisco AnyConnect|      |
| Palo Alto GlobalProtect| |
| WireGuard  | 增长最快   |
| L2TP/IPSec | 逐渐减少   |
| PPTP       | 淘汰     |

---
因为：

审计体系成熟
兼容历史设备
支持企业认证

但新系统越来越偏向 WireGuard。WireGuard 确实已经成为新部署 VPN 的首选。

### 为什么 WireGuard 很火？

以前 VPN 主流：

OpenVPN,几十万行代码。
```
TLS
+
TCP/UDP
+
大量代码
```


IPSec, 配置复杂。
```
IKE
ESP
AH
NAT-T
...

```


WireGuard

作者的目标： 做一个像 SSH 一样简单的 VPN

特点：

| 项目   | WireGuard |
| ---- | --------- |
| 代码量  | 约4万行      |
| 性能   | 非常高       |
| 内核支持 | Linux原生   |
| 加密算法 | 现代        |
| 配置   | 简单        |


---


## Q5: 什么是TUN 模式？必须使用TUN才能 sci-network 吗？

TUN 不是一种代理协议，也不是 VPN 协议。

它只是操作系统提供的一个虚拟三层网卡（Virtual Layer-3 Network Interface）。

### 普通程序访问网络

假设 Chrome 访问 Google。
```
Chrome
   │
TCP Socket
   │
Windows TCP/IP Stack
   │
Physical NIC
   │
Internet
```

### 开启 TUN 以后

系统里多出一个虚拟网卡：
```
Chrome
   │
TCP Socket
   │
Windows TCP/IP Stack
   │
tun0      ← 虚拟网卡
   │
Mihomo
   │
Proxy protocol
   │
Internet
```


### TUN 到底干什么？

TUN 设备收发的是：IP Packet

例如：
```
IPv4 Header

src=192.168.1.100
dst=142.250.190.78

TCP Header

Payload
```

Mihomo 读取 TUN：
```
read(tun_fd, buf, ...)
```

得到完整 IP 包。 然后：
```
Mihomo
↓
规则匹配
↓
加密
↓
发送给服务器
```


### 必须使用 TUN 才能 sci-network 吗？
答案： 完全不是。 TUN 只是其中一种流量接管方式。

#### 方式一：SOCKS5 最古老

```
Chrome
↓
SOCKS5
↓
127.0.0.1:7890
↓
代理服务器
```

Chrome 需要手动设置：
```
SOCKS5
127.0.0.1
7890
```

#### 方式二：HTTP Proxy
```
Chrome
↓
HTTP CONNECT
↓
127.0.0.1:7891
```

#### 方式三：TUN
```
Steam
VSCode
Chrome
Git
curl


↓

tun0


↓

Mihomo
```
所有程序自动走代理。


这些问题都涉及现代网络代理系统的核心概念。我们尽量站在**计算机网络和系统实现**的角度来理解，而不是特定用途。



---

### TUN 的优势

| 方式         | 需要配置程序 | UDP支持 |
| ---------- | ------ | ----- |
| SOCKS5     | 是      | 有限    |
| HTTP Proxy | 是      | 否     |
| TUN        | 否      | 是     |
| VPN        | 否      | 是     |

---

### 为什么很多代理软件支持 TUN？

因为这样可以伪装成 VPN。

例如

Clash

sing-box

---

内部流程

```text
Chrome


↓

tun0


↓

Mihomo


↓

Reality


↓

Server


↓

YouTube
```

---

实际上

Mihomo

自己实现了一个小型 VPN。

---

### VPN 和代理节点的对比

| 项目    | VPN           | 代理节点              |
| ----- | ------------- | ----------------- |
| 工作层   | Network Layer | Application Layer |
| 传输内容  | IP packet     | TCP stream        |
| 客户端IP | 有虚拟IP         | 无                 |
| 默认网关  | VPN服务器        | 本地路由              |
| UDP支持 | 天然支持          | 协议决定              |
| 分流能力  | 较弱            | 很强                |
| 性能    | 高             | 高                 |
| 规则控制  | 困难            | 容易                |
| 典型软件  | WireGuard     | Mihomo/Xray       |
| TUN需求 | 必须            | 可选                |

---

###  现代客户端（2026）的趋势

很多工具已经开始融合两种模式。

例如：

```text
Hiddify


sing-box


Clash Verge Rev


```

内部实际上是：

```text
Application


↓

TUN


↓

sing-box


↓

WireGuard
Reality
Trojan
Hysteria2


↓

Server



```

所以从系统设计角度看：

> **VPN 和代理已经越来越像了。**

VPN 最初是：在网络层传输 IP 包

代理最初是：在应用层转发 TCP 流

而今天的代理客户端（Mihomo、sing-box）通过 **TUN + 用户态网络栈**，已经能够像 VPN 一样接管整个系统流量，同时保留代理协议灵活分流的优势。

对于学习 CS144 的视角，可以这样理解：

```text
CS144 TCP implementation
        │
        ▼
Linux TCP/IP Stack
        │
        ▼
TUN device
        │
        ▼
Mihomo / sing-box
        │
        ▼
WireGuard or other encrypted transport
        │
        ▼
Remote Server
```

这正是现代代理软件和 VPN 软件在系统实现层面逐渐融合的原因。

## Q6: 什么是ssh? 
ssh 原理是什么？ 使用什么协议，工具，技术栈？ssh 远程连接和科学上网有什么区别

SSH 是一个非常值得深入学习的协议，因为它几乎是现代 Linux 运维、云计算、DevOps 和网络工程的基础工具。

---

### 1. 什么是 SSH？

SSH（Secure Shell）是一种**安全远程登录协议**。

它解决的问题是：

> 如何通过不可信的互联网，安全地控制另一台计算机。

---

早期使用的是

```text
Telnet
```

Telnet 的问题：

```text
Client  -----------------------> Server

username = root
password = 123456

（明文传输）
```

任何中间设备都可以看到密码。

SSH 的目标：

```text
Client
   │
Encrypted Channel
   │
Server
```

即使有人截获数据包，也无法看到内容。

---

### 2. SSH 工作在哪一层？

SSH 属于：

```text
Application Layer (L7)
```

它依赖：

```text
TCP
```

默认端口：

```text
22
```

协议栈：

```text
SSH
TCP
IP
Ethernet
```

---

### 3. SSH 使用什么软件？

客户端：

Linux/macOS

```bash
ssh user@host
```

Windows

* OpenSSH
* PuTTY
* Termius
* MobaXterm

服务器：

最常见的是

```text
OpenSSH
```

---

### 4. SSH 的工作原理

假设：

客户端

```text
192.168.1.10
```

服务器

```text
203.0.113.20
```

执行：

```bash
ssh root@203.0.113.20
```

---

#### 第一步：TCP 建立连接

```text
Client                    Server

  SYN  ------------------>

       <---------------- SYN ACK

  ACK  ------------------>
```

连接建立。

---

#### 第二步：协议版本交换

客户端：

```text
SSH-2.0-OpenSSH_9.7
```

服务器：

```text
SSH-2.0-OpenSSH_9.8
```

---

#### 第三步：密钥交换

SSH2 使用：

```text
ECDH

Curve25519

Diffie Hellman
```

双方协商一个共享密钥。

客户端：

```text
private_a
```

服务器：

```text
private_b
```

生成：

```text
shared_secret
```

但共享密钥不会直接发送。

---

#### 第四步：验证服务器身份

服务器发送：

```text
Host Public Key
```

客户端检查：

```text
~/.ssh/known_hosts
```

第一次连接：

```text
Are you sure you want to continue?
```

保存指纹。

---

#### 第五步：建立加密通道

之后所有数据：

```text
AES-GCM

ChaCha20
```

加密。

---

#### 第六步：用户认证

认证方式：

##### 密码认证

```text
password
```

---

##### 公钥认证

客户端：

```text
id_ed25519
```

服务器：

```text
authorized_keys
```

客户端签名：

```text
sign(challenge)
```

服务器验证。

---

### 5. SSH 为什么安全？

因为：

密码不会明文传输。

服务器身份可验证。

所有流量都加密。

防止中间人攻击。

---

### 6. SSH 能做什么？

#### 远程终端

```bash
ssh root@server
```

---

#### 文件复制

```bash
scp file.txt server:/tmp
```

---

#### SFTP

```bash
sftp server
```

---

#### 端口转发

本地转发

```bash
ssh -L 8080:localhost:80 server
```

远程转发

```bash
ssh -R 8080:localhost:80 server
```

动态代理

```bash
ssh -D 1080 server
```

---

### 7. SSH 动态代理是什么？

这是最容易和科学上网混淆的地方。

```bash
ssh -D 1080 user@server
```

本地出现：

```text
127.0.0.1:1080
```

这是一个：

```text
SOCKS5 Proxy
```

浏览器配置：

```text
SOCKS5

127.0.0.1

1080
```

数据流：

```text
Chrome

↓

SOCKS5

↓

SSH Client

↓

Encrypted SSH

↓

Server

↓

youtube.com
```

---

### 8. SSH 和 VPN 的区别

SSH

```text
Application Layer
```

VPN

```text
Network Layer
```

---

SSH

转发 TCP stream

---

VPN

转发 IP packet

---

### 9. SSH 和代理节点有什么区别？

假设使用 Trojan

```text
Chrome

↓

SOCKS5

↓

Mihomo

↓

Trojan

↓

Server

↓

Internet
```

---

SSH

```text
Chrome

↓

SOCKS5

↓

SSH Client

↓

SSH Server

↓

Internet
```

---

本质上很相似。

---

### 10. SSH 和科学上网的区别

这是最重要的问题。

SSH 设计目标：

远程管理服务器

远程执行命令

文件传输

---

代理协议设计目标：

灵活流量分流

支持 UDP

支持 QUIC

支持大量客户端

---

VPN 设计目标：

建立虚拟局域网

提供虚拟 IP

---

### 11. 对比表

|          | SSH        | Proxy           | VPN       |
| -------- | ---------- | --------------- | --------- |
| 工作层      | L7         | L7              | L3        |
| 协议       | SSH        | Trojan/VLESS/SS | WireGuard |
| 默认端口     | 22         | 任意              | 任意        |
| 传输内容     | TCP stream | TCP/UDP stream  | IP packet |
| 虚拟IP     | 否          | 否               | 是         |
| 远程Shell  | 支持         | 否               | 否         |
| 文件传输     | 支持         | 否               | 否         |
| SOCKS5代理 | 支持         | 支持              | 否         |
| 分流能力     | 弱          | 强               | 一般        |
| 典型软件     | OpenSSH    | Mihomo          | WireGuard |

---

### 12. SSH 技术栈

```text
Termius

PuTTY

OpenSSH Client


        │


SSH Protocol


        │


TCP


        │


IP


        │


Internet


        │


OpenSSH Server


        │


bash


systemd


filesystem



```

---

### 13. 从 CS144 的视角理解 SSH

SSH 本质上就是：

```text
stdin/stdout

↓

SSH binary packet protocol

↓

TCP byte stream

↓

Linux TCP stack

↓

Internet

↓

Linux TCP stack

↓

sshd

↓

pty

↓

bash
```

其中：

```text
Terminal Input

↓

SSH Message

↓

TCP Segment

↓

IP Packet

↓

Ethernet Frame
```

---

所以可以把 SSH 理解成：

> **一个运行在 TCP 之上的安全远程终端协议，同时附带文件传输和 SOCKS5 动态代理能力。**

而现代代理软件（Mihomo、Xray、sing-box）则更像是：

> **一个支持多协议、多节点、规则路由和 TUN 接管能力的增强版 SSH 动态代理系统。**


# 2. 科学上网

## Q2.1: 当前科学上网的流行的 代理客户端，Proxy协议，proxy框架, proxy内核(引擎)

- [盘点 科学上网 常见协议和内核 ](https://www.youtube.com/watch?v=K_fnYsWUjq8)

各自总结为一张表格，加入基本描述，优缺点，和其他你觉得需要增加的字段

我会尽量用**中立的网络工程视角**来整理。很多术语经常被混用，其实它们位于不同层次。

### 一、代理客户端（GUI Applications）

代理客户端主要负责：
* 提供 GUI
* 管理订阅
* 维护规则
* TUN 开关：创建 TUN 网卡
* 启停调用 内核（比如Mihomo Core）
* 编辑配置
* 系统代理设置
* 流量统计
* 节点测速

| 客户端             | 平台                      | 底层内核          | TUN | 特点             | 优点           | 缺点        | 活跃度   |
| --------------- | ----------------------- | ------------- | --- | -------------- | ------------ | --------- | ----- |
| Clash Verge Rev | Win/Linux/macOS         | Mihomo        | ✓   | 当前最流行 GUI      | 美观、稳定、规则生态丰富 | 依赖 Mihomo | ⭐⭐⭐⭐⭐ |
| v2rayN          | Windows                 | Xray          | ✓   | 老牌 Windows 客户端 | 配置全面         | UI 较传统    | ⭐⭐⭐⭐  |
| Nekoray         | Win/Linux               | sing-box      | ✓   | 偏技术向           | 支持协议最多       | 学习成本高     | ⭐⭐⭐⭐⭐ |
| Hiddify         | Win/macOS/Linux/Android | sing-box      | ✓   | 新兴跨平台客户端       | 简洁           | 社区较小      | ⭐⭐⭐⭐  |
| Shadowrocket    | iOS                     | sing-box/Xray | ✓   | iOS 最流行        | 功能强          | 付费        | ⭐⭐⭐⭐⭐ |
| Streisand       | iOS                     | sing-box      | ✓   | 新项目            | UI 现代        | 生态较小      | ⭐⭐⭐   |

---

### 二、代理协议（Proxy Protocols）

代理协议决定： 客户端如何和远端服务器通信
从网络工程角度来看，这些协议本质上是在公共互联网之上，再构建一个只有客户端和服务器知道的加密 Overlay Network


#### 应用层代理协议


| 协议             | 工作层 | 传输层    | 是否加密 | 是否有虚拟IP | 特点         | 优点        | 缺点      | 推荐程度  |
| ---------------- | --- | ----------- | ---- | -------------- | ------------ | --------- | --------- | --------- |
| SOCKS5           | L5  | TCP         | ❌    | ❌       | 通用代理协议     | 简单        | 无加密     | ⭐⭐⭐   |
| HTTP CONNECT     | L7  | TCP         | ❌    | ❌       | 浏览器代理      | 兼容性高      | 不支持 UDP | ⭐⭐⭐   |
| Shadowsocks      |     |             |       |           |               |               |            |            |
| Shadowsocks 2022 | L7  | TCP/UDP     | ✓    | ❌       | 轻量加密代理     | 性能高       | 特征较固定   | ⭐⭐⭐⭐  |
| VMess            | L7  | TCP/WS/gRPC | ✓    | ❌       | V2Ray 原生协议 | 历史悠久      | 复杂      | ⭐⭐    |
| VLESS            | L7  | TCP/WS/gRPC | TLS  | ❌       | VMess 精简版  | 简洁        | 依赖 TLS  | ⭐⭐⭐⭐⭐ |
| Trojan           | L7  | TCP         | TLS  | ❌       | HTTPS 风格   | 隐蔽性较好     | 需要证书    | ⭐⭐⭐⭐⭐ |
| Hysteria2        | L7  | QUIC        | TLS  | ❌       | 基于 QUIC    | 高丢包环境表现优秀 | 依赖 UDP  | ⭐⭐⭐⭐⭐ |
| TUIC             | L7  | QUIC        | TLS  | ❌       | 低延迟        | 性能优秀      | 社区较小    | ⭐⭐⭐⭐  |

简要总结 各种 proxy protocol的 核心原理

- Shadowsocks (SS)
2012 年由 Clowwindy 设计。  本质上：一个轻量级 SOCKS5 加密隧道
SS2022 是新版协议。
支持 AES-256-GCM，ChaCha20-Poly1305 现在很多机场仍然提供 SS 节点。

- ShadowsocksR （SSR）
SSR = ShadowsocksR 曾经流行过。 增加了 混淆，协议插件 现在基本已经退出主流。


---






#### 当前（2026）最流行组合

| 排名 | 协议组合             | 流行度   |
| -- | ---------------- | ----- |
| 1  | VLESS + Reality  | ⭐⭐⭐⭐⭐ |
| 2  | Trojan + TLS     | ⭐⭐⭐⭐  |
| 3  | Hysteria2(QUIC)  | ⭐⭐⭐⭐  |
| 4  | TUIC             | ⭐⭐⭐⭐  |
| 5  | Shadowsocks 2022 | ⭐⭐⭐   |
| 6  | VMess            | ⭐⭐    |

#### VPN 协议

| 协议         | 工作层   | 是否分配虚拟IP | 内核支持     | 优点    | 缺点    |
| ---------- | ----- | -------- | -------- | ----- | ----- |
| WireGuard  | L3    | ✓        | Linux 原生 | 高性能   | 分流能力弱 |
| OpenVPN    | L3    | ✓        | 否        | 成熟    | 较慢    |
| IPSec      | L3    | ✓        | 原生       | 企业标准  | 配置复杂  |
| L2TP/IPSec | L2/L3 | ✓        | 原生       | 老设备支持 | 已过时   |
| PPTP       | L2    | ✓        | 原生       | 简单    | 不安全   |




---

### 三、代理框架（Framework）

框架是：

> 实现各种协议的代码库

类似：

```text
Linux
 ├── TCP
 ├── UDP

Proxy Framework
 ├── VLESS
 ├── Trojan
 ├── SS
 ├── Hysteria2
```

| 框架               | 语言   | 支持协议                                  | 是否支持TUN | 生态   | 优点    | 缺点   |
| ---------------- | ---- | ------------------------------------- | ------- | ---- | ----- | ---- |
| Xray-core        | Go   | VLESS/Trojan/VMess/SS                 | ✓       | 大    | 协议成熟  | 配置复杂 |
| sing-box         | Go   | WireGuard/TUIC/Hysteria2/Trojan/VLESS | ✓       | 快速增长 | 协议丰富  | 更新频繁 |
| Mihomo           | Go   | Trojan/VLESS/SS/TUIC/Hysteria2        | ✓       | 最大   | 规则系统强 | 偏客户端 |
| Shadowsocks-rust | Rust | SS2022                                | ✗       | 中    | 性能高   | 功能单一 |


#### Mihomo 做的是：

捕获流量
路由决策
选择节点
实现代理协议

#### Xray/sing-box 做的是：

实现 VLESS、Reality、Trojan 等协议
与远端 VPS 建立隐蔽隧道

#### 真正决定GFW 能不能识别并阻断流量的，是协议设计本身，例如：

VLESS + Reality
Trojan + TLS
Hysteria2 (QUIC)
TUIC

从网络工程角度来看，这些协议本质上是在公共互联网之上，再构建一个只有客户端和服务器知道的加密 Overlay Network


---

### 四、代理内核（Engine）

严格来说，**框架和内核往往是同一个东西**。

但从用户视角：

> 内核 = 被 GUI 调用的可执行程序

| 内核        | GUI             | TUN | Rule Engine | DNS | 典型定位    |
| --------- | --------------- | --- | ----------- | --- | ------- |
| Mihomo    | Clash Verge Rev | ✓   | ⭐⭐⭐⭐⭐       | ✓   | 桌面客户端首选 |
| sing-box  | Hiddify/Nekoray | ✓   | ⭐⭐⭐         | ✓   | 新一代统一核心 |
| Xray-core | v2rayN          | ✓   | ⭐⭐          | ✓   | 协议实验场   |
| WireGuard | 官方客户端           | ✓   | ❌           | ❌   | VPN     |
| OpenVPN   | 官方客户端           | ✓   | ❌           | ❌   | 企业 VPN  |

---

### 五、服务器管理面板

| 面板            | 后端        | 特点     |
| ------------- | --------- | ------ |
| 3x-ui         | Xray      | 最流行    |
| x-ui          | Xray      | 老版本    |
| Hiddify Panel | sing-box  | 新兴     |
| wg-easy       | WireGuard | VPN 管理 |
| NetBird       | WireGuard | 企业零信任  |

---

### 六、2026 年主流组合

| 场景         | 客户端             | 内核        | 协议             | 服务端       |
| ---------- | --------------- | --------- | -------------- | --------- |
| Windows 桌面 | Clash Verge Rev | Mihomo    | Trojan / VLESS | Xray      |
| Linux 桌面   | Nekoray         | sing-box  | Hysteria2      | sing-box  |
| Android    | Hiddify         | sing-box  | TUIC           | sing-box  |
| iPhone     | Shadowrocket    | sing-box  | Trojan         | sing-box  |
| 企业 VPN     | WireGuard App   | WireGuard | WireGuard      | WireGuard |
| NAS 远程访问   | Tailscale       | WireGuard | WireGuard      | Headscale |

---

### 七、从系统架构角度总结

```text
GUI Client
─────────────────────────
Clash Verge Rev
Hiddify
Nekoray
Shadowrocket


Proxy Engine
─────────────────────────
Mihomo
sing-box
Xray-core


Proxy Protocol
─────────────────────────
Trojan
VLESS
Shadowsocks
Hysteria2
TUIC


Transport
─────────────────────────
TCP
TLS
QUIC
UDP


Server
─────────────────────────
Xray
sing-box
WireGuard


Internet
─────────────────────────
YouTube
GitHub
Google
OpenAI
```

---

如果让我给出 **2026 年技术趋势排名**（仅从工程设计和社区活跃度看）：

| 排名 | 项目        | 原因           |
| -- | --------- | ------------ |
| ①  | sing-box  | 协议最全，发展最快    |
| ②  | Mihomo    | GUI 生态最成熟    |
| ③  | WireGuard | VPN 事实标准     |
| ④  | Xray-core | 历史最悠久，协议实验活跃 |
| ⑤  | OpenVPN   | 企业存量仍然巨大     |

对于正在学习 **CS144 和 Linux 网络栈** 的工程师，我最推荐研究的是：

* **WireGuard**（理解 VPN 和 TUN）
* **Mihomo**（理解规则引擎和流量分流）
* **sing-box**（理解现代代理核心设计）
* **Xray-core**（理解协议扩展和传输抽象）

这四个项目基本覆盖了现代网络代理和 VPN 技术栈的大部分核心思想。



## Q2.2 什么是 Proxy Protocols？ proxy protocols 工作原理是什么？


这是一个非常好的问题。 实际上，大多数 Proxy Protocol 的核心思想都非常简单：

> **代理协议 = 如何把用户想访问的目标地址和数据，安全地传输给远程代理服务器。**

它们本质上都在解决下面这个问题：

```text
Client                           Proxy Server
--------------------------------------------------

我要访问：

youtube.com:443


以及后续数据：

TLS Handshake
HTTP/2
Video data


↓↓↓

如何安全地发给 Proxy Server？

↓↓↓

Proxy Server 再替我连接 youtube.com
```

---

### Proxy Protocol 的通用模型

所有代理协议基本都遵循类似流程：

```text
Application

Chrome
↓
SOCKS5
127.0.0.1:7890
↓
Proxy Core
(Mihomo/Xray/sing-box)
↓
Proxy Protocol
↓
Internet
↓
Remote Server
↓
connect(youtube.com,443)
↓
youtube.com
```

---

#### 数据包结构

通常包含三部分

```text
Authentication


Destination


Payload
```

例如

```text
+----------------+
| Authentication |
+----------------+

+----------------+
| youtube.com    |
| 443            |
+----------------+

+----------------+
| TLS bytes      |
+----------------+
```

---

### Proxy protocols 表

| 协议        | 工作层 | 核心思想        | 加密位置 | 认证方式 | 传输      |
| --------- | --- | ----------- | ---- | ---- | ------- |
| SS        | L7  | SOCKS5+AEAD | 协议内部 | 密码   | TCP/UDP |
| SSR       | L7  | SS+混淆       | 协议内部 | 密码   | TCP     |
| VMess     | L7  | UUID+时间戳    | 协议内部 | UUID | TCP     |
| VLESS     | L7  | 简化 VMess    | TLS  | UUID | TCP     |
| Trojan    | L7  | HTTPS 外观    | TLS  | 密码   | TCP     |
| Hysteria2 | L7  | QUIC 隧道     | QUIC | 密码   | UDP     |
| TUIC      | L7  | QUIC 多路复用   | QUIC | UUID | UDP     |
| WireGuard | L3  | VPN         | 协议内部 | 公钥   | UDP     |

---

从协议设计角度总结

所有 Proxy Protocol 实际上都在回答四个问题：

| 问题           | SS             | Trojan         | VLESS          | Hysteria2            |
| ------------ | -------------- | -------------- | -------------- | -------------------- |
| 如何认证？        | 密码             | 密码             | UUID           | 密码                   |
| 如何加密？        | AEAD           | TLS            | TLS            | QUIC                 |
| 如何告诉服务器目标地址？ | Destination 字段 | Destination 字段 | Destination 字段 | QUIC Stream Metadata |
| 如何传输数据？      | TCP            | TCP            | TCP            | UDP/QUIC             |

所以你会发现：

> **Proxy Protocol 本质上就是一个“远程版 SOCKS5 CONNECT 协议”。**

SOCKS5 告诉本地代理：

```text
CONNECT youtube.com:443
```

而 SS、Trojan、VLESS、Hysteria2 等协议做的事情，就是：

> **把这个 CONNECT 请求以及后续数据，以不同的认证、加密、封装方式，安全地发送给远端代理服务器。**

这其实是一个非常优雅的设计思想。






### 1. Shadowsocks (SS)

核心思想 : Shadowsocks = SOCKS5 + 对称加密

工作流程

Chrome

```text
CONNECT youtube.com:443
```

Mihomo

构造

```text
youtube.com


443


TLS bytes
```

加密

```text
ChaCha20


AES-GCM
```

发送给服务器

---

服务器

解密

获得

```text
youtube.com


443
```

执行

```cpp
connect(youtube.com,443)
```

开始转发

---

##### 协议格式

SS2022 大致

```text
Salt


Encrypted


Destination


Payload


Authentication tag
```

---



#### 特点

优点

实现简单

性能高

CPU占用低

缺点

协议特征比较固定

容易被 DPI 统计识别

---

### 2. ShadowsocksR (SSR)

SSR = SS + Obfuscation

---

SS 的流量

```text
Random bytes
```

SSR

包装成

```text
HTTP


TLS


Random padding
```

---

增加

#### protocol

认证方式

---

#### obfs

流量混淆

---

例如

```text
tls1.2_ticket_auth
```

---

#### 特点

优点

比 SS 更难识别

缺点

设计比较混乱

没人维护

协议老旧

---

### 3. VMess

VMess 是 V2Ray 原生协议

---

##### 工作流程

客户端

发送

```text
UUID


timestamp


destination


payload
```

---

服务器

验证 UUID

检查时间戳

连接目标

---

#### 包格式

```text
Header


UUID


Timestamp


Options


Destination


Payload
```

---

#### 特点

优点

支持动态端口

缺点

协议复杂

加密重复

---

### 4. VLESS

VLESS = VMess Less

---

核心思想

> 去掉 VMess 自带加密

依赖 TLS

---

客户端

```text
UUID


Destination


Payload
```

---

TLS 已经加密

所以

协议本身不再加密

---

#### 特点

优点

简单

性能高

扩展容易

缺点

依赖 TLS

---

### 5. Trojan

设计理念：

> 看起来像正常 HTTPS

---

工作流程

建立 TLS

---

发送密码

```text
password


destination


payload
```

---

服务器

验证密码

---

连接目标

---

协议结构

```text
TLS


Password


CRLF


Destination


Payload
```

---

#### 特点

优点

与 HTTPS 十分相似

缺点

需要证书

---

### 6. Hysteria2

基于 QUIC

---

客户端

```text
QUIC stream


Destination


Payload
```

---

QUIC

已经有

加密

拥塞控制

多路复用

---

服务器

打开目标连接

---

#### 特点

优点

高丢包网络优秀

视频体验好

缺点

依赖 UDP

---

### 7. TUIC

TUIC

≈

QUIC + VLESS

---

利用

QUIC stream

---

支持

0-RTT

---

#### 特点

优点

延迟低

缺点

生态较小

---

### 8. WireGuard

严格来说

不是 Proxy Protocol

---

它是 VPN

---

传输

完整 IP packet

---

客户端

```text
IP packet
```

---

加密

---

UDP

---

服务器

解密

---

恢复 IP

---


## Q2.3 使用 Clash Verge Rev 科学上网的工作原理
使用本地客户端：  Clash Verge Rev（PC） ，内核是Mihomo ， 端口是7897 ，代理使用订阅之后选择日本的节点，
首先加一下它的主要想法，它是通过加密的 proxy 协议实现的吗？它是如果成功绕开GFW的访问被中国GFW 屏蔽的网站的核心技术是什么？
然后详细讲解一下科学上网的工作原理的每个步骤



## Q2.4 如何构建一个安全的加密隧道和流量转发系统
从网络技术和代理系统设计的角度解释如何构建一个安全的加密隧道和流量转发系统，
如何利用开源软件可以自己部署一套可以绕开GFW，访问被中国国内屏蔽的网站的技术栈，
客服端使用 Clash Verge Rev，服务端如何部署，设置，需要哪些软件和硬件资源，



