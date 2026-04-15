# linux-kernel-module-keylogger

## Description
This project is a Linux kernel module that listens for keyboard input and keeps track of recent keystrokes. It looks for “complex” sequences that include a mix of lowercase letters, uppercase letters, numbers, and symbols using a sliding window.
>##### Disclaimer:
>This project demonstrates keyboard input monitoring in the Linux kernel for learning purposes only. Using this software to capture or monitor sensitive information without consent is unethical and may be illegal. Use responsibly and only in authorized environments.

## Technologies Used
- Kernel C
- Makefile
- Linux Kernel Modules

## How to Run

### 1. Clone the Repository
```bash
git clone https://github.com/elsberrysilas/linux-kernel-module-keylogger.git
```
### 2. Build the Kernel Module
```bash
make
```
### 3. Insert Module
```bash
sudo insmod keylogger.ko
```
Note: Secure Boot may need to be disabled to load this kernel module.

### 4. Interacting with the Module
```bash
cat /proc/potential_passwords
```
### 6. Remove the Module
```bash
sudo rmmod keylogger
```
## What I Learned
- How to develop and load Linux kernel modules
- How to capture and track keystorkes

## Future Improvements
- I’d like to improve how input is analyzed so it’s better at picking up password sequences and not just random keystrokes sequences.
-  Make the keylogger handle caps lock.
