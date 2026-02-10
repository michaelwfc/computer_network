
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
    cppcheck \
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
    graphviz \
    python3-pip
```

### sponge (21年的版本)

The CS144 Sponge code was likely written/tested with:

Older GCC (GCC 10 or earlier)
Looser header dependencies (headers included other headers implicitly)

Your CS144 VM probably has:

Newer GCC (GCC 11, 12, or 13)
Stricter standards compliance
Explicit includes required


#### The Error Explained

The error occurs because the file `address.cc` is missing the `#include <array>` header. The compiler even tells you this:

```
'std::array' is defined in header '<array>'; this is probably fixable by adding '#include <array>'
```

This is a common issue with newer C++ compilers (GCC 11+) that are stricter about requiring explicit includes for standard library features.

#### Why This Happens

##### C++ Standard Library Headers

In older GCC versions (GCC 10 and earlier), some headers would implicitly include other headers. For example:
- `<memory>` might have indirectly included `<array>`
- `<string>` might have included other headers

##### Modern GCC (11+)

Newer compilers are **stricter** and require **explicit includes**:
- If you use `std::array`, you **must** include `<array>`
- If you use `std::string`, you **must** include `<string>`
- etc.

This improves:
- ✅ **Compilation speed** (fewer unnecessary includes)
- ✅ **Portability** (code works across different compilers)
- ✅ **Standards compliance** (follows C++ standard requirements)



---

If You Encounter More Similar Errors

You might see similar errors for other missing headers. Here's a quick reference:

| If you use... | You need... |
|---------------|-------------|
| `std::array` | `#include <array>` |
| `std::vector` | `#include <vector>` |
| `std::string` | `#include <string>` |
| `std::optional` | `#include <optional>` |
| `std::variant` | `#include <variant>` |
| `std::tuple` | `#include <tuple>` |

---




### GCC（GNU Compiler Collection）
- 出自 GNU 项目，最早是自由软件运动的核心工具之一，历史悠久（1987 年就有）。
- 是一个“整体式”编译器，前端、中间表示、后端耦合紧密。 优化做得很深，但代码基复杂，扩展不容易。
- 历史悠久，性能稳健，交叉编译/嵌入式成熟
  

### Clang（C Language Family Frontend for LLVM）
The Compiler

- 基于 LLVM 框架，2007 年由苹果主导开发。
- clang 是前端，负责编译 C/C++/Objective-C → LLVM IR。
- LLVM 后端做优化、生成目标代码。
- 模块化、错误信息清晰、IDE/工具链友好、研究和教学利器

What it does:
- Compiles C/C++ code into executable programs
- Alternative to g++ (GCC's C++ compiler)
- Generally produces faster compile times
- Part of the LLVM project

#### Clang/LLVM 工具生态
- clangd: 提供 C/C++ 语言服务器（VSCode、Vim、IDEA 都依赖它）。
- clang-tidy: 自动代码检查和重构工具。
- asan/tsan/msan 等运行时检测工具。
- 更适合做“IDE 友好的智能分析”。

#### 在 WSL Ubuntu 用 Clang
```bash
sudo apt update
sudo apt install clang clangd lldb


clang main.c -o main
./main
```




### clangd
**clangd** is a **language server** for C/C++ that provides intelligent code completion, navigation, and diagnostics:

What it does:

- Provides IDE features (code completion, go-to-definition, etc.)
- Analyzes your code without compiling it
- Works with VS Code, Vim, Emacs, etc.
- Reads compile_commands.json to understand your project
- Does NOT compile your code

Features:
- **Code completion** - Suggests function names, variables, and members as you type
- **Go-to-definition** - Navigate to symbol definitions quickly
- **Find references** - Locate all uses of a function, variable, or class
- **Hover tooltips** - Show function signatures, documentation, and type information
- **Real-time diagnostics** - Highlight syntax errors and warnings as you type
- **Rename refactoring** - Safely rename symbols across the entire project

Integration:
- Works with editors like **VSCode**, Vim, Emacs, Neovim
- Implements the **Language Server Protocol (LSP)** 
- Uses the same parsing engine as Clang/LLVM


#### compile_commands.json
Why Use compile_commands.json?
compile_commands.json is a compilation database that tells tools like clangd (C++ language server) exactly how your project is compiled. This enables:
✅ Accurate code completion (IntelliSense)
✅ Go to definition (jump to function/class definitions)
✅ Error highlighting in real-time
✅ Find references
✅ Code navigation
✅ Proper include path resolution

Without it, VS Code doesn't know how to parse your C++ code correctly.



### bear
**bear** (Build EAR) is a **compilation database generator** that intercepts build commands:

#### Purpose:
- **Generates compile_commands.json** - A standardized JSON file listing all compilation commands
- **Intercepts build processes** - Records the exact compiler invocations during a build
- **Supports multiple build systems** - Works with make, ninja, cmake, and others

#### Usage:
```bash
# Generate compilation database for your project
bear -- make

# Or with specific build system
bear -- cmake --build .
```

#### Benefits:
- Enables **clangd** and other tools to understand complex compilation flags
- Allows **IDE features** to work properly with complex build setups
- Supports **static analysis tools** that need exact compilation commands
- Needed for advanced **refactoring and navigation**

#### In the context of Sponge project:
After running `bear -- make` in your build directory, you'd get a [compile_commands.json](file:///home/cs144/computer_network/sponge/build/compile_commands.json) file that clangd can use to provide accurate code intelligence for the Sponge networking library with all its specific compiler flags and include paths.

Both tools work together to provide a rich development experience for C++ projects like Sponge.


### clang-tidy
**clang-tidy** is a **static analysis tool** for C++ code that:
- Performs **comprehensive code analysis** to find bugs, style violations, and potential issues
- Provides **diagnostic warnings** with suggestions for improvements
- Enforces **coding standards** and best practices
- Can automatically **fix some issues** with the `-fix` flag
- In the Sponge project: run with `$ make -j$(nproc) tidy`

**Example uses:**
- Detects unused variables
- Finds potential memory leaks
- Checks for modern C++ best practices
- Validates code style compliance

### cppcheck
**cppcheck** is another **static analysis tool** that:
- Analyzes C/C++ code **without compiling** it
- Focuses on finding **defects and bugs** like buffer overruns, memory leaks, and null pointer dereferences
- Provides **portable analysis** across different platforms
- In the Sponge project: run with `$ make cppcheck`
- Often catches different types of issues than clang-tidy

### clang-format
**clang-format** is a **code formatting tool** that:
- Automatically **formats C/C++ code** according to predefined style rules
- Ensures **consistent code style** across the project
- Supports various formatting styles (Google, LLVM, Mozilla, etc.)
- In the Sponge project: run with `$ make format`
- Helps maintain clean, readable code without manual formatting


## Increase VM Disk Space

### Step 1: Shutdown the VM

```bash
# From inside the VM
sudo shutdown -h now
```

Or from VirtualBox: Right-click VM → Close → Power Off

---

### Step 2: Resize Virtual Disk (On Windows Host)

#### Open PowerShell as Administrator

**Press Windows Key**, type `PowerShell`, right-click **Windows PowerShell**, select **Run as administrator**

#### Find Your VDI File Location

```powershell
# Navigate to VirtualBox
cd "C:\Program Files\Oracle\VirtualBox"

# List all VMs to find yours
.\VBoxManage list vms
#PS C:\Program Files\Oracle\VirtualBox> .\VBoxManage.exe list vms
#"CS144 Fall 2025" {086f1a3d-3423-4083-b6e0-04f95b23923c}

# Show disk info (replace with your VM name)
.\VBoxManage showvminfo "cs144" | findstr "SATA"
#PS C:\Program Files\Oracle\VirtualBox> .\VBoxManage showvminfo "CS144 Fall 2025" | findstr "SATA"
#1: 'SATA', Type: IntelAhci, Instance: 0, Ports: 1 (max 30), Bootable


```

**Common VDI locations:**
- `C:\Users\YourUsername\VirtualBox VMs\cs144\cs144.vdi`
- `C:\Users\YourUsername\VirtualBox VMs\cs144vm\cs144vm.vdi`
 <!-- C:\Users\michael\VirtualBox VMs\CS144 Fall 2025\cs144-fall-2025-x86-disk001.vdi -->

Or find it in VirtualBox GUI:
1. Open VirtualBox
2. Select your CS144 VM
3. Click **Settings** → **Storage**
4. Click on the disk under "Controller: SATA"
5. Note the file path shown

#### Resize the Disk

```powershell
# Increase to 12GB (12288 MB)

.\VBoxManage modifyhd "C:\Users\YourUsername\VirtualBox VMs\cs144\cs144.vdi" --resize 12288
```

.\VBoxManage modifyhd "C:\Users\michael\VirtualBox VMs\CS144 Fall 2025\cs144-fall-2025-x86-disk001.vdi" --resize 12288

**Replace the path** with your actual VDI file location!

**Expected output:**
```
0%...10%...20%...30%...40%...50%...60%...70%...80%...90%...100%
```

---

### Step 3: Start VM and Expand Partition

Start your VM from VirtualBox, then SSH in:

```bash
ssh -p 2222 cs144@localhost
```

#### Check Current Partition Layout

```bash
# View partitions
lsblk

# Should show something like:
# NAME   MAJ:MIN RM  SIZE RO TYPE MOUNTPOINT
# sda      8:0    0   30G  0 disk          ← New size (e.g., 30G)
# ├─sda1   8:1    0    1M  0 part
# └─sda2   8:2    0  7.8G  0 part /        ← Old size (7.8G)

cs144@cs144vm:~$ lsblk
NAME   MAJ:MIN RM  SIZE RO TYPE MOUNTPOINTS
sda      8:0    0   12G  0 disk
├─sda1   8:1    0    1M  0 part
└─sda2   8:2    0    8G  0 part /
sr0     11:0    1 1024M  0 rom

```

#### Expand the Partition

Ubuntu 24+ typically uses a simple partition scheme. Here's how to expand:

##### Option A: Using growpart (Simplest)

```bash
# Install cloud-guest-utils
sudo apt update
sudo apt install cloud-guest-utils

# Grow partition 2 on /dev/sda
sudo growpart /dev/sda 2
#cs144@cs144vm:~$ sudo growpart /dev/sda 2
#CHANGED: partition=2 start=4096 old: size=16771072 end=16775167 new: size=25161695 end=25165790

# Resize the filesystem
sudo resize2fs /dev/sda2

cs144@cs144vm:~$ sudo resize2fs /dev/sda2
resize2fs 1.47.2 (1-Jan-2025)
Filesystem at /dev/sda2 is mounted on /; on-line resizing required
old_desc_blocks = 1, new_desc_blocks = 2
The filesystem on /dev/sda2 is now 3145211 (4k) blocks long.


# Verify new size
df -h

cs144@cs144vm:~$ df -h
Filesystem      Size  Used Avail Use% Mounted on
tmpfs           341M  1.1M  340M   1% /run
/dev/sda2        12G  7.1G  4.2G  63% /
tmpfs           1.7G     0  1.7G   0% /dev/shm
tmpfs           5.0M     0  5.0M   0% /run/lock
tmpfs           1.7G  4.0K  1.7G   1% /tmp
tmpfs           1.0M     0  1.0M   0% /run/credentials/systemd-journald.service
tmpfs           1.0M     0  1.0M   0% /run/credentials/systemd-resolved.service
tmpfs           1.0M     0  1.0M   0% /run/credentials/systemd-networkd.service
tmpfs           1.0M     0  1.0M   0% /run/credentials/getty@tty1.service
tmpfs           341M   16K  341M   1% /run/user/100
```

##### Option B: Using parted (If growpart doesn't work)

```bash
# Install parted
sudo apt install parted

# Start parted
sudo parted /dev/sda

# In parted prompt:
(parted) print
# Note the partition number (usually 2)

(parted) resizepart 2
# When prompted for "End?", type: 100%

(parted) quit

# Resize the filesystem
sudo resize2fs /dev/sda2

# Verify
df -h
```

##### Option C: Using fdisk (Advanced)

```bash
# Backup partition table first
sudo sfdisk -d /dev/sda > ~/partition-backup.txt

# Delete and recreate partition (data will be preserved if done correctly)
sudo fdisk /dev/sda

# In fdisk:
# Press 'p' to print partitions (note start sector of sda2)
# Press 'd' then '2' to delete partition 2
# Press 'n' then 'p' then '2' to create new partition
# Use the SAME start sector as before
# Use default for end sector (max size)
# Press 'N' when asked to remove signature
# Press 'w' to write changes

# Resize the filesystem
sudo resize2fs /dev/sda2

# Verify
df -h
```

---

### Step 4: Verify New Space

```bash
# Check disk space
df -h

# Should now show something like:
# Filesystem      Size  Used Avail Use% Mounted on
# /dev/sda2        28G  7.1G   20G  27% /
#                 ^^^^       ^^^^
#               (bigger)  (more available)
```

---


--------------------------------------------------------------------



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



# Compiler & IDE Features

## Which Compiler Should You Use for CS144?
For Building/Compiling Code:
Use g++ (GCC) ← Recommended for CS144
```bash
# Your CMakeLists.txt already uses g++ by default
cd ~/computer_network/sponge/build
cmake ..
make -j$(nproc)
```
Why g++?

✅ CS144 labs are designed and tested with GCC
✅ Most compatible with course requirements
✅ Default on Ubuntu/Linux
✅ What the autograder uses


## For IDE Features (IntelliSense):
Use clangd ← Recommended

Why clangd?

✅ Faster and more accurate than Microsoft C/C++
✅ Better code completion
✅ More reliable go-to-definition
✅ Industry standard for C++ development



## Vscode settings

### C++ IDE ：WSL Vscode settings Reference
- IDE : [vscode](https://code.visualstudio.com/docs/languages/cpp)
- vscode extension: Install recommended C/C++ extension in VSCode and reload
- https://code.visualstudio.com/docs/cpp/cpp-debug
- https://code.visualstudio.com/docs/cpp/launch-json-reference


### Vscode setting files

#### 1. settings.json: Set up VS Code settings
```json
{
  "C_Cpp.default.compilerPath": "/usr/bin/g++",
  //  Disable Microsoft C/C++ IntelliSense when using clangd
  "C_Cpp.intelliSenseEngine": "Disabled",

  // === clangd for IntelliSense  === 
  "clangd.path": "/usr/bin/clangd",
  "clangd.arguments": [
        "--compile-commands-dir=${workspaceFolder}/sponge/build",
        "--background-index",
        "--clang-tidy",
        "--completion-style=detailed",
        "--header-insertion=iwyu"
    ],

  // ===  CMake configuration === 
   "cmake.configureOnOpen": false, // Don't auto-configure
   //CMake Tools extension is looking for a CMake project at workspace root
  // This tells CMake Tools to look for a CMake project in your workspace
  "cmake.buildDirectory": "${workspaceFolder}/sponge/build",
  "cmake.sourceDirectory": "${workspaceFolder}/sponge/",

```

#### 2. install the extensions

```bash
{
    "recommendations": [
        "ms-vscode.cpptools",
        "llvm-vs-code-extensions.vscode-clangd",
        "ms-vscode.cpptools-extension-pack",
    ]
}

```

### Vscode Debug setting files
#### 1. launch.json: build launch configuration(debugger settings)
#### 2. tasks.json: Set up build task in 
#### 3. c_cpp_properties.json: build C/C++ properties file  in 
 c_cpp_properties.json (compiler path and IntelliSense settings)

### How to Set Up clangd in VS Code

#### Set Up clangd in VS Code
- Step 1: Generate compile_commands.json
compile_commands.json can be generated by bear or cmake command
```bash
bear -- make
# or
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
# CMake usually does this by default, but this forces it
```
Note: CMake automatically generates compile_commands.json in the build directory.

- Step 2: Create Symlink to Project Root (Recommended)
clangd looks for compile_commands.json in the project root, but CMake creates it in the build directory. Create a symlink:
```bash
cd ~/computer_network/sponge

# Create symbolic link from root to build directory
ln -s build/compile_commands.json compile_commands.json

# Verify the symlink
ls -la compile_commands.json
# lrwxrwxrwx 1 cs144 cs144 27 Feb  3 23:34 compile_commands.json -> build/compile_commands.json

# Should show:
# lrwxrwxrwx ... compile_commands.json -> build/compile_commands.json
```

- Step 4: Install clangd on the VM
```bash
# Install clangd
sudo apt update
sudo apt install clangd

# Or install a specific version (if needed)
sudo apt install clangd-14

# Verify installation
clangd --version
```

- Step 3: Install clangd Extension in VS Code
1. In VS Code (connected to VM via Remote-SSH)
2. Press Ctrl + Shift + X to open Extensions
3. Search for "clangd"
4. Install "clangd" by LLVM
5. Disable or uninstall the Microsoft C/C++ extension (it conflicts with clangd)




- Step 5: Configure VS Code Settings
Press Ctrl + Shift + P and type "Preferences: Open Settings (JSON)"
Add these settings:
```json
{
    "clangd.arguments": [
        "--compile-commands-dir=${workspaceFolder}/sponge/build",
        "--background-index",
        "--clang-tidy",
        "--completion-style=detailed",
        "--header-insertion=iwyu"
    ],
    "clangd.path": "clangd",
    "C_Cpp.intelliSenseEngine": "Disabled",
    //  "cmake.configureOnOpen": true,
    "cmake.buildDirectory": "${workspaceFolder}/sponge/build",
    "files.associations": {
        "*.cc": "cpp",
        "*.hh": "cpp"
    }
}
```

#### Verify It's Working
Check 1: clangd Status
Look at the bottom-right corner of VS Code. You should see:
"clangd" indicator (may show "indexing..." at first)


Check 2: Test Code Completion
Open a C++ file (e.g., apps/webget.cc) and try:



## How to debug in VS Code


Example lab0: how can I debug t_byte_stream_one_write in vscode? 


### Step 1: Find the Test Executable

```bash
cd ~/computer_network/sponge/build
find . -name "*byte_stream_one_write*" -type f
# Should show something like: ./tests/byte_stream_one_write


# ./tests/CMakeFiles/byte_stream_one_write.dir/byte_stream_one_write.cc.o.d
# ./tests/CMakeFiles/byte_stream_one_write.dir/byte_stream_one_write.cc.o
# ./tests/byte_stream_one_write
# ./.cache/clangd/index/byte_stream_one_write.cc.2E9F1558D0F41316.idx

```

### Step 2: Create VS Code Debug Configuration

Create/edit `.vscode/launch.json` in your project root:

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug byte_stream_one_write",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/tests/byte_stream_one_write",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}/build",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "build",
            "miDebuggerPath": "/usr/bin/gdb"
        }
    ]
}
```

### Step 3: Create Build Task (Optional)

Create `.vscode/tasks.json`:

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build",
            "type": "shell",
            "command": "make",
            "args": ["-j8"],
            "options": {
                "cwd": "${workspaceFolder}/build"
            },
            "group": {
                "kind": "build",
                "isDefault": true
            }
        }
    ]
}
```

### Step 4: Set Breakpoints and Debug

1. Open `libsponge/byte_stream.cc`
2. Click left margin next to line numbers to set breakpoints:
   - Set breakpoint in `write()` function
   - Set breakpoint in `bytes_written()` function

3. Press **F5** or click "Run and Debug" → "Debug byte_stream_one_write"

4. Use debug controls:
   - **F10** - Step Over
   - **F11** - Step Into
   - **Shift+F11** - Step Out
   - **F5** - Continue

5. Inspect variables:
   - Hover over variables
   - Check "Variables" panel
   - Check "Watch" panel

---


#### step 5: Build object to Debug mode

```bash
cd ~/computer_network/sponge/build
rm -rf *

cmake .. -DCMAKE_BUILD_TYPE=Debug
# cs144@cs144vm:~/computer_network/sponge/build$ cmake .. -DCMAKE_BUILD_TYPE=Debug
# CMake Deprecation Warning at CMakeLists.txt:1 (cmake_minimum_required):
#   Compatibility with CMake < 3.10 will be removed from a future version of
#   CMake.

#   Update the VERSION argument <min> value.  Or, use the <min>...<max> syntax
#   to tell CMake that the project requires at least <min> but has been updated
#   to work with policies introduced by <max> or earlier.


# -- The C compiler identification is GNU 14.2.0
# -- The CXX compiler identification is GNU 14.2.0
# -- Detecting C compiler ABI info
# -- Detecting C compiler ABI info - done
# -- Check for working C compiler: /usr/bin/cc - skipped
# -- Detecting C compile features
# -- Detecting C compile features - done
# -- Detecting CXX compiler ABI info
# -- Detecting CXX compiler ABI info - done
# -- Check for working CXX compiler: /usr/bin/c++ - skipped
# -- Detecting CXX compile features
# -- Detecting CXX compile features - done
# -- Building in Debug mode as requested.
# --   NOTE: You can choose a build type by calling cmake with one of:
# --     -DCMAKE_BUILD_TYPE=Release   -- full optimizations
# --     -DCMAKE_BUILD_TYPE=Debug     -- better debugging experience in gdb
# --     -DCMAKE_BUILD_TYPE=RelASan   -- full optimizations plus address and undefined-behavior sanitizers
# --     -DCMAKE_BUILD_TYPE=DebugASan -- debug plus sanitizers
# -- Found Doxygen: /usr/bin/doxygen (found version "1.9.8") found components: doxygen dot
# -- Found clang-format version 2
# -- Found clang-tidy version 2
# -- Found cppcheck
# -- Configuring done (1.2s)
# -- Generating done (0.2s)
# -- Build files have been written to: /home/cs144/computer_network/sponge/build


make -j8
```

#### step 6: Debug in VS Code
