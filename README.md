# OS Jackfruit — Lightweight Container Runtime

---

## 1. Team Information

| Name            | SRN           |
| --------------- | ------------- |
| Atharv Kulkarni | PES2UG24CS092 |
| Arya M.G        | PES2UG24CS087 |
---

## 2. Build, Load, and Run Instructions

### Prerequisites

* Ubuntu 22.04 / 24.04
* Secure Boot OFF (for kernel module)
* Linux kernel headers installed

```bash
sudo apt update
sudo apt install -y build-essential linux-headers-$(uname -r)
```

---

### Build

```bash
make clean
make
```

Verify binaries:

```bash
ls engine monitor.ko memory_hog cpu_hog io_pulse
```

---

### Load Kernel Module

```bash
sudo rmmod monitor 2>/dev/null
sudo dmesg -C
sudo insmod monitor.ko
```

Verify:

```bash
sudo dmesg | tail
```

---

### Run Engine

```bash
sudo ./engine
```

---

### Launch Containers

Inside CLI:

```bash
start alpha
start beta
list
```

---

### CLI Commands

```bash
start <name>
list
stop <name>
count
exit
```

---

### Stop Containers

```bash
stop alpha
stop beta
```

---

### Clean Teardown

```bash
sudo pkill -9 -f alpha
sudo pkill -9 -f beta
ps aux | grep -E "alpha|beta"
```

---

## 3. Demo Screenshots

### Screenshot 1 — Multi-container supervision
<img width="1862" height="1169" alt="1_Multicontainer" src="https://github.com/user-attachments/assets/99997a6a-e922-497b-8c26-918f3b309916" />


Two containers (alpha and beta) running simultaneously.

---

### Screenshot 2 — Metadata tracking
<img width="699" height="334" alt="2_Meta_data" src="https://github.com/user-attachments/assets/6f1670b9-e188-4035-b256-e63b53c0a2af" />


Command:

```bash
ps -eo pid,comm,%cpu | grep -E "alpha|beta"
```

Shows container PID and CPU usage.

---

### Screenshot 3 — Bounded-buffer logging
<img width="919" height="1170" alt="3_Bounded_buffer_logging" src="https://github.com/user-attachments/assets/bda80c4b-388c-4fd0-9d47-4a1d272b3d7d" />


Command:

```bash
cat logs.txt
```

Shows logs produced by containers using producer-consumer model.

---

### Screenshot 4 — CLI and IPC
<img width="1859" height="1177" alt="4_CLI_IPC" src="https://github.com/user-attachments/assets/4bddcdee-523f-4242-956b-c51e14cc1b1f" />


Commands executed through CLI:

```bash
start alpha
start beta
count
```

Shows interaction between user and runtime.

---

### Screenshot 5 — Soft-limit warning
<img width="1849" height="417" alt="5 6_Soft Hard_Limit" src="https://github.com/user-attachments/assets/ad12d1ee-af2e-42f3-b73d-2f602ae1505a" />


Command:

```bash
sudo dmesg | tail -30
```

Output shows soft limit warning.

---

### Screenshot 6 — Hard-limit enforcement
<img width="1849" height="417" alt="5 6_Soft Hard_Limit" src="https://github.com/user-attachments/assets/bf3c7b40-9e58-41c3-ba4d-051474e5b6ec" />


Command:

```bash
sudo dmesg | tail -30
```

Shows container termination after exceeding hard limit.

---

### Screenshot 7 — Scheduling experiment
<img width="814" height="790" alt="7_Scheduler" src="https://github.com/user-attachments/assets/90b88924-73d1-4259-8711-eb9aef3e328b" />


Command:

```bash
ps -eo pid,comm,%cpu --sort=-%cpu | grep -E "alpha|beta"
```

Shows CPU usage difference:

* alpha → high CPU
* beta → low CPU

---

### Screenshot 8 — Clean teardown
<img width="1033" height="84" alt="8_Clean_teardown" src="https://github.com/user-attachments/assets/c5f966fb-1ff9-4758-9ff6-4672b5475796" />


Command:

```bash
ps aux | grep -E "alpha|beta"
```

Shows no remaining container processes.

---

## 4. Engineering Analysis

### 4.1 Isolation Mechanisms

The project uses Linux namespaces for isolation:

* PID namespace

  * Each container has its own process ID space

* UTS namespace

  * Each container has its own hostname

* Mount namespace

  * Each container has an isolated filesystem

The system uses `chroot()` to provide filesystem isolation.

---

### 4.2 Container Lifecycle

Container states:

* starting → created using clone
* running → executing workload
* stopped → manually terminated
* killed → terminated by kernel module

---

### 4.3 Logging and Synchronization

* Logging uses producer-consumer model
* Pipe connects container output to buffer
* Consumer thread writes logs to file

Synchronization uses:

* mutex
* condition variables

---

### 4.4 Memory Monitoring

* Kernel module monitors container memory usage
* Soft limit → warning
* Hard limit → process termination

Monitoring uses kernel-level access to process memory.

---

### 4.5 Scheduling Behavior

* alpha → CPU + memory intensive
* beta → light workload

Results:

* alpha gets higher CPU usage
* beta gets lower CPU usage

Demonstrates scheduling fairness.

---

## 5. Design Decisions

* Used `chroot()` instead of `pivot_root()` for simplicity
* Used threads for logging (shared memory)
* Used kernel module for accurate monitoring
* Used simple CLI for usability

---

## 6. Conclusion

This project demonstrates practical implementation of:

* Containerization using namespaces
* Scheduling behavior
* Kernel-level monitoring
* Inter-process communication
* Synchronization using bounded buffer

---

## Authors

Atharv Kulkarni  &  Arya M.G

P.E.S. College
