#include "../include/types.h"

#define NET_BUFFER_SIZE 1500
#define NET_MAX_PACKETS 8

typedef struct {
    u8 buffer[NET_BUFFER_SIZE];
    u16 len;
    u32 src_ip;
    u32 dst_ip;
    u16 src_port;
    u16 dst_port;
} NetPacket;

static u32 local_ip = 0;
static u32 gateway_ip = 0x0A000202;
static u32 dns_ip = 0x0A000203;
static u8 local_mac[6] = {0x52, 0x54, 0x00, 0x12, 0x34, 0x56};

static NetPacket rx_packets[NET_MAX_PACKETS];
static u32 rx_write_idx = 0;
static u32 rx_read_idx = 0;

void net_init(void) {
    local_ip = 0x0A0002FF;
    gateway_ip = 0x0A000202;
    dns_ip = 0x0A000203;
}

u32 net_get_local_ip(void) {
    return local_ip;
}

u32 net_get_dns_ip(void) {
    return dns_ip;
}

u32 net_get_gateway_ip(void) {
    return gateway_ip;
}

void net_enqueue_packet(const u8 *data, u16 len, u32 src_ip, u32 dst_ip, u16 src_port, u16 dst_port) {
    if (rx_write_idx >= NET_MAX_PACKETS) {
        rx_write_idx = 0;
    }

    NetPacket *pkt = &rx_packets[rx_write_idx];
    pkt->len = len < NET_BUFFER_SIZE ? len : NET_BUFFER_SIZE;
    pkt->src_ip = src_ip;
    pkt->dst_ip = dst_ip;
    pkt->src_port = src_port;
    pkt->dst_port = dst_port;

    u32 i;
    for (i = 0; i < pkt->len; ++i) {
        pkt->buffer[i] = data[i];
    }

    ++rx_write_idx;
    if (rx_write_idx >= NET_MAX_PACKETS) {
        rx_write_idx = 0;
    }
}

NetPacket *net_dequeue_packet(void) {
    if (rx_read_idx == rx_write_idx) {
        return 0;
    }

    NetPacket *pkt = &rx_packets[rx_read_idx];
    ++rx_read_idx;
    if (rx_read_idx >= NET_MAX_PACKETS) {
        rx_read_idx = 0;
    }

    return pkt;
}

u16 net_checksum(const u8 *data, u32 len) {
    u32 sum = 0;
    u32 i;
    for (i = 0; i < len; i += 2) {
        u16 word = (data[i] << 8) | (i + 1 < len ? data[i + 1] : 0);
        sum += word;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    return (u16)(~sum);
}
