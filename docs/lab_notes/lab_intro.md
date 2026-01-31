
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