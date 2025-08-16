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









DDoS Flood Tool
This is an educational C program designed to demonstrate network packet crafting and flooding techniques using raw sockets. It supports UDP, TCP, ICMP, HTTP, and raw IP protocols with source IP spoofing (except for HTTP) for simulating network traffic. This tool is intended solely for educational purposes and should only be used in controlled environments with explicit permission.
WARNING: Using this tool to attack systems or networks without written authorization is illegal and can result in severe consequences, including fines or imprisonment. Always obtain permission before testing on any system, even your own, and respect local laws and network policies.
Features

Multi-Protocol Support: Sends UDP, TCP (SYN flood), ICMP (echo request), HTTP (GET requests), or raw IP packets.
Source IP Spoofing: Randomizes source IP addresses for UDP, TCP, ICMP, and raw IP packets to simulate distributed attacks (not available for HTTP).
Random Payloads: Optionally randomizes packet payloads to evade signature-based detection.
Random Source Ports: Optionally randomizes source ports for UDP and TCP to make filtering harder.
Multi-Threaded: Uses multiple threads to increase packet rate, configurable for performance.
Customizable Rates: Adjust packets per second per thread and number of threads.

Prerequisites
Before running this tool, ensure you have the following:

Kali Linux:

This guide assumes you’re using Kali Linux (2023 or later), which includes necessary tools. Other Linux distributions may work but require similar setup.
Verify your Kali version:lsb_release -a




Root Access:

The program uses raw sockets, requiring root privileges (sudo).
HTTP flooding may work without root, but running as root ensures consistency.


Development Tools:

Install gcc and make for compiling the code:sudo apt update
sudo apt install build-essential


Ensure libc6-dev for standard C libraries:sudo apt install libc6-dev




Network Interface:

A working network interface (e.g., eth0, wlan0) is required.
Check interfaces:ip a




Controlled Environment:

Do not run this on public networks or servers without permission.
Set up a local test lab (e.g., VirtualBox/VMware with two VMs: one for the attacker, one for the target).
Example target: A VM running a web server (e.g., Apache on 192.168.56.101).



Installation
Follow these steps to set up the tool on Kali Linux:

Save the Code:

Copy the C code into a file named ddos_flood.c.
Use a text editor like nano:nano ddos_flood.c


Paste the code, save (Ctrl+O, Enter, Ctrl+X in nano), and exit.


Compile the Code:

Compile with gcc, linking the threading library (-pthread):gcc -o ddos_flood ddos_flood.c -pthread


This creates an executable named ddos_flood.
If compilation fails, ensure build-essential and libc6-dev are installed.


Set Permissions:

Make the executable runnable:chmod +x ddos_flood





Usage
Run the program with sudo due to raw socket requirements. The basic syntax is:
sudo ./ddos_flood <target_ip> <port> [rate_per_thread] [threads] [randomize_payload 0/1] [randomize_src_port 0/1] [protocol udp/tcp/icmp/http/raw] [proto_num for raw]

Command-Line Arguments

<target_ip>: Target IP address (e.g., 192.168.1.100).
<port>: Target port (e.g., 80 for HTTP, 12345 for UDP/TCP). Use 0 for ICMP or raw IP.
[rate_per_thread]: Packets per second per thread (default: 100,000).
[threads]: Number of threads (default: 4).
[randomize_payload 0/1]: 1 for random payload, 0 for fixed ('A') (default: 1).
[randomize_src_port 0/1]: 1 for random source ports (UDP/TCP), 0 for fixed (default: 1).
[protocol udp/tcp/icmp/http/raw]: Protocol to use (default: udp).
[proto_num for raw]: Protocol number for raw IP (e.g., 255).

Example Commands

UDP Flood (default settings, target 192.168.1.100:12345):
sudo ./ddos_flood 192.168.1.100 12345


Sends 100,000 packets/sec per thread, 4 threads, random payload and source port.


TCP SYN Flood (port 80, 50,000 packets/sec, 8 threads, no random payload):
sudo ./ddos_flood 192.168.1.100 80 50000 8 0 1 tcp


ICMP Flood (port ignored, 2 threads):
sudo ./ddos_flood 192.168.1.100 0 100000 2 1 1 icmp


HTTP Flood (port 80, 1,000 requests/sec, 4 threads):
sudo ./ddos_flood 192.168.1.100 80 1000 4 1 1 http


Note: HTTP uses regular TCP sockets, no IP spoofing.


Raw IP Flood (protocol number 255):
sudo ./ddos_flood 192.168.1.100 0 100000 4 1 1 raw 255



Stopping the Program

Press Ctrl+C to stop the program.
If it doesn’t stop cleanly, find the process ID and kill it:ps aux | grep ddos_flood
sudo kill <pid>



Setting Up a Test Environment
For safe testing, create a local network using virtual machines:

Install VirtualBox/VMware:
sudo apt install virtualbox


Create Two VMs:

VM1 (Attacker): Kali Linux.
VM2 (Target): Another Linux VM (e.g., Ubuntu) with a web server:sudo apt install apache2
sudo systemctl start apache2


Note the target’s IP (e.g., 192.168.56.101).


Configure Network:

Set both VMs to a private network (e.g., VirtualBox Host-Only Adapter, 192.168.56.0/24).


Test Connectivity:

From the Kali VM:ping 192.168.56.101




Monitor the Target:

On the target VM, use tcpdump or wireshark to capture packets:sudo tcpdump -i eth0 host 192.168.56.101


Check server logs for impact:tail -f /var/log/apache2/error.log





Monitoring and Troubleshooting

Monitor Resources:

Check CPU/memory usage:top


Monitor network traffic:sudo iftop -i eth0




Verify Packets:

Use tcpdump to confirm packets are sent:sudo tcpdump -i eth0 host <target_ip>




Common Issues:

"Permission denied": Ensure you’re using sudo.
No packets sent: Check if the target IP is reachable or if your ISP blocks spoofed packets.
Program crashes: Reduce rate or threads (e.g., 50000 2).
Invalid protocol: Use udp, tcp, icmp, http, or raw (with proto_num for raw).



Performance Considerations

Packet Rate: Default 100,000 packets/sec per thread (~800 Mbps per thread). With 4 threads, this is ~3.2 Gbps, but actual output depends on your CPU, memory, and network interface.
Network Limits: Residential connections (e.g., 10–100 Mbps upload) can’t sustain high rates. Test in a local lab for best results.
Egress Filtering: Some ISPs block spoofed packets. Test locally to avoid issues.

Legal and Ethical Guidelines

DO NOT use this tool to harm systems or networks without explicit permission.
Misuse is illegal under laws like the U.S. Computer Fraud and Abuse Act or similar regulations worldwide.
Only use in controlled environments (e.g., your own lab) or with written authorization for security testing.
Learn defensive techniques (e.g., firewalls, IDS like Snort) to counter such tools.

Learning Resources

Raw Sockets: man 7 raw
Network Tools: Try hping3, nmap, or wireshark for comparison.
DDoS Defense: Study iptables, Cloudflare, or AWS Shield.
Books: "TCP/IP Illustrated" or "Practical Packet Analysis" for networking basics.

Disclaimer
This tool is for educational purposes only. The author and contributors are not responsible for misuse. Always obtain permission before testing, and respect legal and ethical boundaries.