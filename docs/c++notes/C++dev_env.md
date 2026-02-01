
# Ubuntu LTS Versions and GCC Support

Here's a quick overview of the most popular Ubuntu LTS versions, the default g++ (GCC) version they ship with, and the C++ standard they support 

| Ubuntu Version | Default g++ Version | Default C++ Standard | Supported C++ Standards |
|----------------|---------------------|----------------------|-------------------------|
| Ubuntu 24.04 LTS | g++ 13 | C++17 | C++98, C++03, C++11, C++14, C++17, C++20, C++23 (partial) |
| Ubuntu 22.04 LTS | g++ 11 | C++17 | C++98, C++03, C++11, C++14, C++17, C++20 (partial), C++23 (some via PPA) |
| Ubuntu 20.04 LTS | g++ 9 | C++14 | C++98, C++03, C++11, C++14, C++17, C++20 (partial) |
| Ubuntu 18.04 LTS | g++ 7 | C++14 | C++98, C++03, C++11, C++14, C++17 (partial) |
| Ubuntu 16.04 LTS | g++ 5 | C++11 | C++98, C++03, C++11, C++14 (partial) |

## Compilers

C/C++ extension does not include a C++ compiler. So, you will need to install one or use which is already installed on your computer.

- WSL(Linux): gcc + GDB


### Why compile over interpret?

It allows us to generate more efficient machine code!
- Interpreters only see one small part of code at a time
- Compilers see everything
However, compilation takes time!



### Upgrade GCC 11 to 12 or 13+

Issue:
In C++20, std::string is allowed in constexpr — but only if the standard library fully implements that feature. GCC only fully supports constexpr std::string in GCC 12 and later.


Because Ubuntu 22.04 only comes with GCC 11 by default, and the ubuntu-toolchain-r/test PPA allows you to install and use newer compilers.

Install a newer GCC version:

```bash
# This adds a PPA (Personal Package Archive) to your system's APT sources.
# The ppa:ubuntu-toolchain-r/test is a well-known official PPA that provides newer versions of GCC, G++, etc. for Ubuntu.
# It enables you to install GCC 12, 13, 14, etc., even if your Ubuntu release only ships with GCC 11.

sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install g++-13

# Then switch to it:
# It registers a new version of g++ (version 13) with the update-alternatives system, so you can easily switch between multiple installed versions of g++ on your system.
# /usr/bin/g++	The symbolic link to manage (the "master link").
# g++	The name of the group (what you’ll use with update-alternatives --config).
# /usr/bin/g++-13	The actual path to the specific g++ version you want to register (in this case, version 13).
# 100	The priority. Higher numbers = higher preference when auto-selecting the default.
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100

ll /usr/bin/ |grep g++
# lrwxrwxrwx  1 root root          21 Jul 29 17:59 g++ -> /etc/alternatives/g++*
# lrwxrwxrwx  1 root root          23 May 13  2023 g++-11 -> x86_64-linux-gnu-g++-11*
# lrwxrwxrwx  1 root root          23 Jul 11  2023 g++-13 -> x86_64-linux-gnu-g++-13*
# lrwxrwxrwx  1 root root           6 Aug  5  2021 x86_64-linux-gnu-g++ -> g++-11*
# -rwxr-xr-x  1 root root      932680 May 13  2023 x86_64-linux-gnu-g++-11*
# -rwxr-xr-x  1 root root     1018800 Jul 11  2023 x86_64-linux-gnu-g++-13*



# get back

sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 50
sudo update-alternatives --config g++
```



# Linux installation(cs144 vm)

## Reference

- [FAQ-Answers to common questions about lab assignment](https://cs144.github.io/lab_faq.html)
- [CS144's lab0](https://web.archive.org/web/20220827011711/https://cs144.github.io/doc/lab0)
- [slides](https://github.com/khanhnamle1994/computer-networking)
- [lab-repo](https://github.com/PKUFlyingPig/CS144-Computer-Network)
- [lab-repo2](https://github.com/top-mind/cs144-minnow-nju/tree/main)


### CS144 VM setting
### 1. install cs144 vm image on VirtualBox
- [Setting up your CS144 VM](https://stanford.edu/class/cs144/vm_howto/)
- [Setting up your CS144 VM using VirtualBox](https://stanford.edu/class/cs144/vm_howto/vm-howto-image.html)
- [Setting up your CS144 VM(Ubuntu 18.04) using VirtualBox](https://stanford.edu/class/cs144/vm_howto/vm-howto-iso.html)


Make sure your VM is actually running in VirtualBox
Check VirtualBox port forwarding settings:
1. Select your VM → Settings → Network → Adapter 1 → Advanced → Port Forwarding
2. Verify there's a rule: Host Port 2222 → Guest Port 22

### 2. Try the connection with ssh on cmd

```bash
cmd
Microsoft Windows [版本 10.0.26100.7623]
(c) Microsoft Corporation。保留所有权利。

C:\Users\michael>ssh -p 2222 cs144@localhost
The authenticity of host '[localhost]:2222 ([127.0.0.1]:2222)' can't be established.
ED25519 key fingerprint is SHA256:a+uYorVaxtZe54PWem+MSxUzX/VUIwAVFhlT+KS8fAM.
This key is not known by any other names.
Are you sure you want to continue connecting (yes/no/[fingerprint])? yes
Warning: Permanently added '[localhost]:2222' (ED25519) to the list of known hosts.
cs144@localhost's password:
Welcome to Ubuntu 25.04 (GNU/Linux 6.14.0-32-generic x86_64)

 * Documentation:  https://help.ubuntu.com
 * Management:     https://landscape.canonical.com
 * Support:        https://ubuntu.com/pro

 System information as of Sat Jan 31 01:12:02 AM UTC 2026

  System load:             0.0
  Usage of /:              36.0% of 7.77GB
  Memory usage:            8%
  Swap usage:              0%
  Processes:               141
  Users logged in:         1
  IPv4 address for enp0s3: 10.0.2.15
  IPv6 address for enp0s3: fd17:625c:f037:2:a00:27ff:fe6a:ed7e


0 updates can be applied immediately.


Last login: Sat Jan 31 01:10:01 2026 from 10.0.2.2

cs144@cs144vm:~$ lsb_release -a
No LSB modules are available.
Distributor ID: Ubuntu
Description:    Ubuntu 25.04
Release:        25.04
Codename:       plucky
cs144@cs144vm:~$
```

### Connection VM with vscode


Install Remote - SSH extension in VS Code
Press Ctrl+Shift+P
Type: Remote-SSH: Connect to Host
Select Add New SSH Host
Enter: ssh -p 2222 cs144@localhost
Select config file (first option is fine)
Press Ctrl+Shift+P again
Select Remote-SSH: Connect to Host
Choose localhost
Enter password: cs144

```bash
# ctrl+shift+` :open the terminal on vscode
cs144@cs144vm:~$ lsb_release -a
No LSB modules are available.
Distributor ID: Ubuntu
Description:    Ubuntu 25.04
Release:        25.04
Codename:       plucky
cs144@cs144vm:~$ pwd
/home/cs144
cs144@cs144vm:~$ 
```

####  How to Work with the existed local code  
Best workflow:

1. On local: Use Git to sync between VM and Windows when needed
   - Initialize git in your local project
   - Push to a Git hosting service 
 

2. On VM:
   - Connect VS Code via Remote-SSH to the VM
   - Clone on the VM: 
      - `cd ~`
      - `git clone https://github.com/michaelwfc/computer_network.git`
   - Edit, compile, and debug entirely on the VM through VS Code 
   - Commit and push from VM
 
3. On local:  Pull on Windows when needed

## Set proxy on VM

The VM can't access GitHub because it needs to use your Windows proxy. Here are several solutions:

1. Configure Clash to Allow LAN Connections
Your Clash might be blocking connections from the VM. Fix this:

- Open Clash on Windows
- Go to Settings/General
- Enable "Allow LAN" or "Allow connections from LAN"
- Note the port (7897 in your case)
- Restart Clash if needed


2. VM set proxy for git
```bash
# option 1: Configure Git to Use Proxy (Quickest)
# In the VM terminal
# Use the host IP that the VM can reach (VirtualBox default gateway)
# Note: 10.0.2.2 is the default IP address that VirtualBox VMs use to reach the host machine.
git config --global http.proxy http://10.0.2.2:7897
git config --global https.proxy http://10.0.2.2:7897

# If you need to remove proxy later:
git config --global --unset http.proxy
git config --global --unset https.proxy


# Check your default gateway
cs144@cs144vm:~$ ip route | grep default
default via 10.0.2.2 dev enp0s3 proto dhcp src 10.0.2.15 metric 100
# Should show something like: default via 10.0.2.2 dev enp0s3

cs144@cs144vm:~$ netstat -rn | grep "^0.0.0.0"
0.0.0.0         10.0.2.2        0.0.0.0         UG        0 0          0 enp0s3

# Now try cloning
git clone https://github.com/michaelwfc/computer_network.git

```


3. VM Set System-Wide Proxy (More Permanent)

```bash
# Edit your bash profile
nano ~/.bashrc

# Add these lines at the end:
export http_proxy="http://10.0.2.2:7897"
export https_proxy="http://10.0.2.2:7897"

# Save (Ctrl+O, Enter, Ctrl+X)

# Reload the configuration
source ~/.bashrc

# Test connectivity to proxy:
curl -x http://10.0.2.2:7897 https://www.google.com
```


## Packages for Lab

(21 年的 sponge 版本)
- [BYO Linux installation](https://stanford.edu/class/cs144/vm_howto/vm-howto-byo.html)


You’re welcome to use any Linux installation, in a VM or not, provided that it has a reasonably recent kernel (4.x should be fine) and a C++23 compiler.

To run the labs, you’ll need the following software:

g++ version 8.x
clang-tidy version 6 or 7
clang-format version 6 or 7
cmake version 3 or later
libpcap development headers (libpcap-dev on Debian-like distributions)
git
iptables
mininet 2.2.0 or later
tcpdump
telnet
wireshark
socat
netcat-openbsd
GNU coreutils
bash
doxygen
graphviz

### install the required packages
```bash
# Install all required packages
sudo apt install -y \
    build-essential \
    clang \
    clang-tidy \
    clang-format \
    gcc-doc \
    pkg-config \
    glibc-doc \
    cmake \
    libpcap-dev \
    git \
    iptables \
    mininet \
    tcpdump \
    tshark \
    telnet \
    wireshark \
    socat \
    netcat-openbsd \
    coreutils \
    bash \
    doxygen \
    graphviz
```



# C++ dev env on windows

## C++ on WSL

- https://code.visualstudio.com/docs/cpp/config-wsl
- https://code.visualstudio.com/docs/remote/wsl
  
### WSL Ubuntu env Installation

 - install wsl and linux distribution
 - install vscode
 - install vscode for local wsl

```bash
wsl -l -v 
# connect to wsl terminal of Ubuntu-22.04
wsl -d Ubuntu-22.04 
lsb_release -a
# No LSB modules are available.
# Distributor ID: Ubuntu
# Description:    Ubuntu 22.04.5 LTS
# Release:        22.04
# Codename:       jammy

g++ --version
# michael@DESKTOP-2KLOSPO:/mnt/e/projects/computer_network$ g++ --version
# g++ (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0
# Copyright (C) 2021 Free Software Foundation, Inc.
# This is free software; see the source for copying conditions.  There is NO
# warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

whereis g++
# michael@DESKTOP-2KLOSPO:/mnt/e/projects/computer_network$ whereis g++
# g++: /usr/bin/g++ /usr/share/man/man1/g++.1.gz

gdb --version
# michael@DESKTOP-2KLOSPO:/mnt/e/projects/computer_network$ gdb --version
# GNU gdb (Ubuntu 12.1-0ubuntu1~22.04.3) 12.1
# Copyright (C) 2022 Free Software Foundation, Inc.
# License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
# This is free software: you are free to change and redistribute it.
# There is NO WARRANTY, to the extent permitted by law.

whereis gdb
# michael@DESKTOP-2KLOSPO:/mnt/e/projects/computer_network$ whereis gdb
# gdb: /usr/bin/gdb /etc/gdb /usr/include/gdb /usr/share/gdb /usr/share/man/man1/gdb.1.gz
```





# Hello World

## Source code
```cpp
// This directive includes the standard input-output stream library, which allows us to use std::cout for printing to the console.
#include <iostream>

int hello_cpp() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
```

## Compile and run

```bash
# Compile and run
 # g++ is the compiler, outputs binary to main
g++ main.cpp –o main
# This actually runs our program 
./main		

g++ C++tutorials/STL/vector.cpp lib/include/printUtils.cpp -Ilib/include -o vector
./vector
```


## Coding suggestions

- Before submitting each assignment, please run `cmake --build build --target format` (to standardize the formatting), and make sure to remove any dummy code or comments that were included with the starter code.
- Also (optionally) run `cmake --build build --target tidy` to receive suggestions for improvements related to C++ programming practices.


## WSL Vscode settings

### C++ IDE ：WSL Vscode settings Reference
- IDE : [vscode](https://code.visualstudio.com/docs/languages/cpp)
- vscode extension: Install recommended C/C++ extension in VSCode and reload
- https://code.visualstudio.com/docs/cpp/cpp-debug
- https://code.visualstudio.com/docs/cpp/launch-json-reference

### Vscode setting files
- c_cpp_properties.json (compiler path and IntelliSense settings)
- tasks.json (build instructions)
- launch.json (debugger settings)

