
21 年的 sponge 版本

# Reference

- [FAQ-Answers to common questions about lab assignment](https://cs144.github.io/lab_faq.html)
- [CS144's lab0](https://web.archive.org/web/20220827011711/https://cs144.github.io/doc/lab0)
- [slides](https://github.com/khanhnamle1994/computer-networking)
- [lab-repo](https://github.com/PKUFlyingPig/CS144-Computer-Network)
- [lab-repo2](https://github.com/top-mind/cs144-minnow-nju/tree/main)

# Lab env

## 1. install cs144 vm image on VirtualBox
- [Setting up your CS144 VM](https://stanford.edu/class/cs144/vm_howto/)
- [Setting up your CS144 VM using VirtualBox](https://stanford.edu/class/cs144/vm_howto/vm-howto-image.html)

Make sure your VM is actually running in VirtualBox
Check VirtualBox port forwarding settings:
1. Select your VM → Settings → Network → Adapter 1 → Advanced → Port Forwarding
2. Verify there's a rule: Host Port 2222 → Guest Port 22

## 2. Try the connection with cmd

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

## Connection VM with vscode


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

##  How to Work with the existed local code  
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