#include <lib.h>

#define PACKET_BUFFER_SIZE	1024

void dump_frame(unsigned char buf[], unsigned short len);

int is_arp(unsigned char buf[], unsigned short len);

int is_arp_naive(unsigned char buf[], unsigned short len);

short conv_endian16(short value);

struct ether_hdr {
    char dst_addr[6];
    char src_addr[6];
    short type;
};

// void fill(unsigned char buf_s[], unsigned char idx, unsigned char* src, unsigned char len) {
// 	for (int i = idx; i < idx + len; i++) {
// 		buf_s[i] = *(src + i);
// 	}
// }

// void fill_bytes(unsigned char buf_s[], unsigned char idx, long long val, unsigned char bytes_cnt) {
// 	for (int i = idx; i < idx + bytes_cnt; i++) {
// 		buf_s[i] = val >> (8 * (bytes_cnt - (i - idx))); // ビッグエンディアンなので、valの上のバイトからbufに詰めていく
// 	}
// }

// unsigned char src_mac_addr[] = {0x68, 0xf7, 0x28, 0x69, 0x24, 0xcc};
// unsigned char src_ip_addr[] = {192, 168, 1, 0};

// void set_src_mac(unsigned char buf_s[]) {
// 	for (int i = 0; i < 6; i++) {
// 		buf_s[i + 6] = src_mac_addr[i];
// 	}
// }

// void set_dst_mac(unsigned char buf_s[], unsigned char* dst_mac) {
// 	for (int i = 0; i < 6; i++) {
// 		buf_s[i] = *(dst_mac + i);
// 	}
// }

int main(void)
{
	puts("SERVER STARTED\r\n");
	while (1) {
		unsigned char buf[PACKET_BUFFER_SIZE];
		unsigned char buf_s[PACKET_BUFFER_SIZE] = { 0 };
		unsigned short len;

		/* 受信 */
		while (!(len = receive_frame(buf)));
		if (is_arp(buf, len)) {
			while(1) {}
		}
		dump_frame(buf, len);
		puts("__RECEIVED\r\n");

		/* 送信 */
		// buf_s[0] = 0xbe;
		// buf_s[1] = 0xef;
		// buf_s[2] = 0xbe;
		// buf_s[3] = 0xef;
		// len = 4;
		// dump_frame(buf_s, len);
		// unsigned char stat = send_frame(buf_s, len);
		// puts("__SENDED ");
		// puth(stat, 2);
		// puts("\r\n");

		if (buf[0] == 0xff && buf[1] == 0xff && buf[2] == 0xff && buf[3] == 0xff && buf[4] == 0xff && buf[5] == 0xff) {
			if (buf[12] == 0x08 && buf[13] == 0x06) {
				puts("LOOKS LIKE ARP\r\n");
			}
			long long x = 0;
			while(x < 1000000000) {
				x += 1;
			}

			// set_dst_mac(buf_s, (buf + 6));
			// set_src_mac(buf_s);
			// buf_s[12] = 0x08;
			// buf_s[13] = 0x06;
			// fill_bytes(buf_s, 14, 0x0001, 2);
			// fill_bytes(buf_s, 16, 0x0800, 2);
			// fill_bytes(buf_s, 18, 6, 1);
			// fill_bytes(buf_s, 19, 4, 1);
			// fill_bytes(buf_s, 20, 2, 2);
			// fill(buf_s, 22, src_mac_addr, 6);
			// fill(buf_s, 28, src_ip_addr, 4);
			// fill(buf_s, 32, (buf + 6), 6);
			// todo: parse the arp request and fill target ip, and check the 
		}
	}

	return 0;
}

void dump_frame(unsigned char buf[], unsigned short len)
{
	unsigned short i;
	for (i = 0; i < len; i++) {
		if (0 < i) {
			if (buf[i-1] == 0x06 && buf[i] == 0x08) {
				puts("ARP TYPE FOUND\r\n");
				long long x = 0;
				while(x < 1000000000) {
					i += 1;
				}
			}
		}
		puth(buf[i], 2);
		putc(' ');

		if (((i + 1) % 24) == 0)
			puts("\r\n");
		else if (((i + 1) % 4) == 0)
			putc(' ');
	}

	putc('_');
	puth(len, 4);

	if (len > 0)
		puts("\r\n");
}

void memcpy(void *dst, void *src, unsigned long long size)
{
	unsigned char *d = (unsigned char *)dst;
	unsigned char *s = (unsigned char *)src;
	for (; size > 0; size--)
		*d++ = *s++;
}

int strncmp(char *s1, char *s2, unsigned long long n)
{
	char is_equal = 1;

	for (; (*s1 != '\0') && (*s2 != '\0'); s1++, s2++, n--) {
		if (*s1 != *s2 || n == 1) {
			is_equal = 0;
			break;
		}
	}

	if (is_equal) {
		if (*s1 != '\0') {
			return 1;
		} else if (*s2 != '\0') {
			return -1;
		} else {
			return 0;
		}
	} else {
		return (int)(*s1 - *s2);
	}
}

short conv_endian16(short value) {
    short out;
    out = value >> 8;
    out |= value << 8;
    return out;
}

int is_arp(unsigned char buf[], unsigned short len)
{
	struct ether_hdr *hdr;
	hdr = (struct ether_hdr*) buf;
	// memcpy(&hdr, buf, sizeof(struct ether_hdr));
	char pat[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	if (strncmp(hdr->dst_addr, pat, 6)) {
		puts("BROADCAST FOUND.");
		// puth(hdr.type, 10);
		// long long i = 0;
		// while(i < 1000000000) {
		// 	i += 1;
		// }
		if (hdr->type == 0x0806) { // why not little endian..?
			puts("TYPE IS ARP");
			return 1;
		}
	}
	return 0;
}

int is_arp_naive(unsigned char buf[], unsigned short len)
{
	struct ether_hdr hdr;
	memcpy(&hdr, buf, sizeof(struct ether_hdr));
	char pat[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	if (strncmp(hdr.dst_addr, pat, 6)) {
		puts("BROADCAST FOUND.");
		if (buf[12] == 0x06 && buf[13] == 0x08) {
			puts("type 06 08");
			return 1;
		}
		if (buf[12] == 0x08 && buf[13] == 0x06) {
			puts("type 08 06");
			return 1;
		}
	}
	return 0;
}
