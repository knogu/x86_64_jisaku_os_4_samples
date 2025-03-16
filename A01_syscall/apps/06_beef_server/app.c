#include <lib.h>

#define PACKET_BUFFER_SIZE	1024

void dump_frame(unsigned char buf[], unsigned short len);

int is_arp(unsigned char buf[], unsigned short len);

struct ether_hdr {
    char dst_addr[6];
    char src_addr[6];
    short type;
} __attribute__((__packed__));

int main(void)
{
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
		buf_s[0] = 0xbe;
		buf_s[1] = 0xef;
		buf_s[2] = 0xbe;
		buf_s[3] = 0xef;
		len = 4;
		dump_frame(buf_s, len);
		unsigned char stat = send_frame(buf_s, len);
		puts("__SENDED ");
		puth(stat, 2);
		puts("\r\n");
	}

	return 0;
}

void dump_frame(unsigned char buf[], unsigned short len)
{
	unsigned short i;
	for (i = 0; i < len; i++) {
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
	struct ether_hdr hdr;
	memcpy(&hdr, buf, sizeof(struct ether_hdr));
	char pat[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	if (strncmp(hdr.dst_addr, pat, 6)) {
		puts("BROADCAST FOUND. SRC: ");
		puts(hdr.src_addr);
		if (conv_endian16(hdr.type) == 0x0806) {
			puts("TYPE IS ARP");
			return 1;
		}
	}
	return 0;
}
