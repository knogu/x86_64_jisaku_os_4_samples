#include <x86.h>
#include <intr.h>
#include <pic.h>
#include <acpi.h>
#include <fb.h>
#include <kbc.h>
#include <fbcon.h>
#include <fs.h>
#include <hpet.h>
#include <sched.h>
#include <common.h>
#include <syscall.h>
#include <proc.h>

struct __attribute__((packed)) platform_info {
	struct framebuffer fb;
	void *rsdp;
};

#define INIT_APP	"test"


/* PCIの定義 */
#define PCI_CONF_DID_VID	0x00

#define CONFIG_ADDRESS	0x0cf8
#define CONFIG_DATA	0x0cfc

union pci_config_address {
	unsigned int raw;
	struct __attribute__((packed)) {
		unsigned int reg_addr:8;
		unsigned int func_num:3;
		unsigned int dev_num:5;
		unsigned int bus_num:8;
		unsigned int _reserved:7;
		unsigned int enable_bit:1;
	};
};


/* NICの定義 */
#define NIC_BUS_NUM	0x00
#define NIC_DEV_NUM	0x19
#define NIC_FUNC_NUM	0x0

unsigned int get_pci_conf_reg(
	unsigned char bus, unsigned char dev, unsigned char func,
	unsigned char reg);

void dump_vid_did(unsigned char bus, unsigned char dev, unsigned char func);

void start_kernel(void *_t __attribute__((unused)), struct platform_info *pi,
		  void *_fs_start)
{
	/* フレームバッファ周りの初期化 */
	fb_init(&pi->fb);
	set_fg(255, 255, 255);
	set_bg(0, 70, 250);
	clear_screen();

	/* ACPIの初期化 */
	acpi_init(pi->rsdp);

	/* CPU周りの初期化 */
	gdt_init();
	intr_init();

	/* 周辺ICの初期化 */
	pic_init();
	hpet_init();
	kbc_init();



	/* NICのベンダーID・デバイスIDを取得 */

	/* CONFIG_ADDRESSを設定 */
	union pci_config_address conf_addr;
	conf_addr.raw = 0;
	conf_addr.bus_num = 0;
	conf_addr.dev_num = 0;
	conf_addr.func_num = 0;
	conf_addr.reg_addr = 0x0c;
	conf_addr.enable_bit = 1;
	io_write32(CONFIG_ADDRESS, conf_addr.raw);

	/* CONFIG_DATAを読み出す */
	unsigned int conf_data = io_read32(CONFIG_DATA);

	unsigned short header_type = (conf_data >> 16) & 0xffu;

	/* 表示 */
	puts("HEADER TYPE");
	puth(header_type, 4);
	puts("\r\n");
	puts("IS SINGLE");
	puth((header_type & 0x80u) == 0, 4);
	puts("\r\n");

	for (int device=0; device < 32; ++device) {
		unsigned int conf_data = get_pci_conf_reg(
			0, device, 0, PCI_CONF_DID_VID);
	
		/* 読み出したデータからベンダーID・デバイスIDを取得 */
		unsigned short vendor_id = conf_data & 0x0000ffff;	
		if (vendor_id == 0xffffu) {
			continue;
		}
		unsigned short device_id = conf_data >> 16;
		puts("DEVICE INDEX ");
		puth(device, 8);

		puts("  DEVICE ID ");
		puth(device_id, 8);

		puts("  VENDOR ID ");
		puth(vendor_id, 4);
		puts("\r\n");

		unsigned int class_code = get_pci_conf_reg(
			0, device, 0, 0x08);
		unsigned short base = (class_code >> 24) & 0xffu;
    	unsigned short sub = (class_code >> 16) & 0xffu;
		puts("BASE CLASS ");
		puth(base, 4);
		puts("  SUB CLASS ");
		puth(sub, 4);
		puts("\r\n");
		puts("\r\n");
	}

	/* haltして待つ */
	while (1)
		cpu_halt();



	/* システムコールの初期化 */
	syscall_init();

	/* ファイルシステムの初期化 */
	fs_init(_fs_start);

	/* スケジューラの初期化 */
	sched_init();

	/* CPUの割り込み有効化 */
	enable_cpu_intr();

	/* スケジューラの開始 */
	sched_start();

	/* initアプリ起動 */
	exec(open(INIT_APP));

	/* haltして待つ */
	while (1)
		cpu_halt();
}

unsigned int get_pci_conf_reg(
	unsigned char bus, unsigned char dev, unsigned char func,
	unsigned char reg)
{
	/* CONFIG_ADDRESSを設定 */
	union pci_config_address conf_addr;
	conf_addr.raw = 0;
	conf_addr.bus_num = bus;
	conf_addr.dev_num = dev;
	conf_addr.func_num = func;
	conf_addr.reg_addr = reg;
	conf_addr.enable_bit = 1;
	io_write32(CONFIG_ADDRESS, conf_addr.raw);

	/* CONFIG_DATAを読み出す */
	return io_read32(CONFIG_DATA);
}

void dump_vid_did(unsigned char bus, unsigned char dev, unsigned char func)
{
	/* PCIコンフィグレーション空間のレジスタを読み出す */
	unsigned int conf_data = get_pci_conf_reg(
		bus, dev, func, PCI_CONF_DID_VID);

	/* 読み出したデータからベンダーID・デバイスIDを取得 */
	unsigned short vendor_id = conf_data & 0x0000ffff;
	unsigned short device_id = conf_data >> 16;

	/* 表示 */
	puts("VENDOR ID ");
	puth(vendor_id, 4);
	puts("\r\n");
	puts("DEVICE ID ");
	puth(device_id, 4);
	puts("\r\n");
}
