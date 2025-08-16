# 🛠️ Enhanced Network Flood Simulator (For Educational/Lab Use Only)

> **Disclaimer:** This program is intended **only for testing on your own servers or lab environments**. Unauthorized use against external systems or networks is illegal and can have serious consequences.

This project demonstrates **network packet generation, multithreading, and basic DoS simulation techniques** in C. It supports multiple protocols, including UDP, TCP, ICMP, HTTP, and raw protocols, and allows you to experiment with payloads, ports, and IP spoofing in a controlled environment.

---

## 🚀 Features

- Multi-threaded packet sending for high throughput.  
- Supports multiple protocols:  
  - **UDP**: Randomized source ports and payloads.  
  - **TCP**: SYN flood simulation.  
  - **ICMP**: Echo request (ping) simulation.  
  - **HTTP**: TCP-based GET request flood (no spoofing).  
  - **RAW**: Custom protocol simulation with spoofed headers.  
- Configurable packet rate per thread.  
- Randomized payloads and source ports for realistic traffic simulation.  
- IP spoofing support (except for HTTP).  

---

## 🛠️ Installation

### Requirements
- Linux OS (Ubuntu/Debian recommended)  
- C compiler (`gcc`)  
- Root privileges (required for raw sockets)  

### Install necessary packages

```bash
sudo apt update
sudo apt install build-essential


⚙️ Compilation
gcc -pthread -o flood_simulator flood_simulator.c


Note: You must run the program as root if using raw sockets:
sudo ./flood_simulator ...

📌 Usage
sudo ./flood_simulator <target_ip> <port> [rate_per_thread] [threads] [randomize_payload 0/1] [randomize_src_port 0/1] [protocol udp/tcp/icmp/http/raw] [proto_num for raw]

Parameters
Parameter	Description
target_ip	IP of your local test server or lab machine
port	Target port (ignored for ICMP/RAW, default 80 for HTTP)
rate_per_thread	Packets per second per thread (default: 100000)
threads	Number of threads (default: 4)
randomize_payload	0 = fixed payload, 1 = random payload (default: 1)
randomize_src_port	0 = fixed port, 1 = random port (default: 1)
protocol	Protocol to use: udp, tcp, icmp, http, raw
proto_num	Required for RAW mode (e.g., 255)
Examples

UDP Flood (default settings):

sudo ./flood_simulator 127.0.0.1 80


TCP SYN Flood, 200k packets/sec, 8 threads:

sudo ./flood_simulator 127.0.0.1 80 200000 8 1 1 tcp


HTTP GET Flood:

./flood_simulator 127.0.0.1 80 5000 4 0 0 http


Raw Packet Simulation (protocol 255):

sudo ./flood_simulator 127.0.0.1 0 100000 4 1 1 raw 255

🔧 How It Works

Multithreading: Each thread continuously sends packets to maximize throughput.

Protocol Handling: TCP, UDP, ICMP, HTTP, and RAW are handled differently with their respective headers.

Spoofing & Randomization: Source IPs, ports, and payloads can be randomized to simulate realistic traffic.

Checksums: IP, TCP, UDP, and ICMP checksums are calculated for raw packets to ensure validity.

Delays: usleep() controls packets per second per thread.

⚠️ Safety Notes

Always run on local or controlled lab machines.

Using raw sockets requires root privileges.

Avoid testing on external networks — it’s illegal.

Use htop or top to monitor CPU usage; high packet rates can max out resources.

🎯 Learning Outcomes

Low-level socket programming in C.

Understanding of network protocols (IP, TCP, UDP, ICMP, HTTP).

How multithreading and rate control affect network traffic.

Implementing checksums and raw packet manipulation.

📜 License

This project is for educational purposes only. No license for external attacks.


---

If you want, I can also make a **more visual version with badges, emojis for each protocol, and sections for “Screenshots / Stats / Notes”** so it looks more like a **professional GitHub project**.  

Do you want me to do that?