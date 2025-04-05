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
#include <pci.h>
#include <nic.h>

void nic_handler(void);

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

struct __attribute__((packed)) platform_info {
	struct framebuffer fb;
	void *rsdp;
};

union CapabilityHeader {
    unsigned int data;
    struct {
        unsigned char cap_id;
        unsigned char next_ptr;
        unsigned short reserved;
    } bits;
};

#define NIC_INTR_NO	32

void do_nic_interrupt(void);

struct MSICapability {
    union {
      uint32_t data;
      struct {
        uint32_t cap_id : 8;
        uint32_t next_ptr : 8;
        uint32_t msi_enable : 1;
        uint32_t multi_msg_capable : 3;
        uint32_t multi_msg_enable : 3;
        uint32_t addr_64_capable : 1;
        uint32_t per_vector_mask_capable : 1;
        uint32_t : 7;
      } __attribute__((packed)) bits;
    } __attribute__((packed)) header ;

    uint32_t msg_addr;
    uint32_t msg_upper_addr;
    uint32_t msg_data;
    uint32_t mask_bits;
    uint32_t pending_bits;
  } __attribute__((packed));

#define INIT_APP	"test"

unsigned int get_nic_reg_base(void);

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
	// nic_init();


	unsigned int nic_dev_num;
	unsigned short nic_vendor_id;
	unsigned short nic_device_identifier;
	unsigned int nic_reg_base;
	for (int device=0; device < 32; ++device) {
		unsigned int conf_data = get_pci_conf_reg(
			0, device, 0, PCI_CONF_DID_VID);
	
		/* 読み出したデータからベンダーID・デバイスIDを取得 */
		unsigned short cur_vendor_id = conf_data & 0x0000ffff;	
		if (cur_vendor_id == 0xffffu) {
			continue;
		}
		unsigned short cur_device_id = conf_data >> 16;

		unsigned int class_code = get_pci_conf_reg(0, device, 0, 0x08);
		unsigned short base = (class_code >> 24) & 0xffu;
    	unsigned short sub = (class_code >> 16) & 0xffu;
		if (base == 0x02 && sub == 0) {
			nic_dev_num = device;
			nic_vendor_id = cur_vendor_id;
			nic_device_identifier = cur_device_id;
			nic_reg_base = get_pci_conf_reg(0, device, 0, 0x10) & PCI_BAR_MASK_MEM_ADDR;
			break;
		}
	}

	puts("NIC DEVICE INDEX ");
	puth(nic_dev_num, 8);

	puts("  DEVICE ID ");
	puth(nic_device_identifier, 8);

	puts("  VENDOR ID ");
	puth(nic_vendor_id, 4);
	puts("\r\n");

	puts("  NIC REG BASE ");
	puth(nic_reg_base, 8);
	puts("\r\n");

	unsigned char msi_cap_addr = 0, msix_cap_addr = 0;
	unsigned char cap_addr = get_pci_conf_reg(0, nic_dev_num, 0, 0x34) & 0xffu;
	int i = 0;
	while (cap_addr != 0 && i < 64) {
		puts("  CAP ADDR ");
		puth(cap_addr, 2);
		puts(" ");
		union CapabilityHeader header;
		header.data = get_pci_conf_reg(0, nic_dev_num, 0, cap_addr);
		if (header.bits.cap_id == 0x05) {
			msi_cap_addr = cap_addr;
		} else if (header.bits.cap_id == 0x11) {
			msix_cap_addr = cap_addr;
		}
		cap_addr = header.bits.next_ptr;
		i += 1;
	}

	puts("  MSI CAP ADDR ");
	puth(msi_cap_addr, 4);

	puts("  MSIX CAP ADDR ");
	puth(msix_cap_addr, 4);
	
	struct MSICapability msi_cap;
	msi_cap.header.data = get_pci_conf_reg(0, nic_dev_num, 0, msi_cap_addr);
	msi_cap.msg_addr = get_pci_conf_reg(0, nic_dev_num, 0, msi_cap_addr + 4);

	unsigned char msg_data_addr = msi_cap_addr + 8;
	if (msi_cap.header.bits.addr_64_capable) {
		msi_cap.msg_upper_addr = get_pci_conf_reg(0, nic_dev_num, 0, msi_cap_addr + 8);
		msg_data_addr = msi_cap_addr + 12;
	}

	msi_cap.msg_data = get_pci_conf_reg(0, nic_dev_num, 0, msg_data_addr);
	if (msi_cap.header.bits.per_vector_mask_capable) {
		msi_cap.mask_bits = get_pci_conf_reg(0, nic_dev_num, 0, msg_data_addr + 4);
		msi_cap.pending_bits = get_pci_conf_reg(0, nic_dev_num, 0, msg_data_addr + 8);
	}
	msi_cap.header.bits.multi_msg_enable = 0;
	msi_cap.header.bits.msi_enable = 1;

	unsigned char bsp_local_apic_id = (0xfee00020) >> 24;
	uint32_t msg_addr = 0xfee00000u | (bsp_local_apic_id << 12);
    uint32_t msg_data = NIC_INTR_NO;
	msi_cap.msg_addr = msg_addr;
    msi_cap.msg_data = msg_data;

	set_pci_conf_reg(0, nic_dev_num, 0, msi_cap_addr, msi_cap.header.data);
	set_pci_conf_reg(0, nic_dev_num, 0, msi_cap_addr + 4, msi_cap.msg_addr);
	msg_data_addr = msi_cap_addr + 8;
    if (msi_cap.header.bits.addr_64_capable) {
		set_pci_conf_reg(0, nic_dev_num, 0, msi_cap_addr + 8, msi_cap.msg_upper_addr);
      	msg_data_addr = cap_addr + 12;
    }
	set_pci_conf_reg(0, nic_dev_num, 0, msg_data_addr, msi_cap.msg_data);
	if (msi_cap.header.bits.per_vector_mask_capable) {
		set_pci_conf_reg(0, nic_dev_num, 0, msg_data_addr + 4, msi_cap.mask_bits);
		set_pci_conf_reg(0, nic_dev_num, 0, msg_data_addr + 8, msi_cap.pending_bits);
	}

	set_intr_desc(NIC_INTR_NO, nic_handler);

	/* CPUの割り込み有効化 */
	enable_cpu_intr();

	puts("  CONFIG DONE ");	

	/* haltして待つ */
	while (1)
		cpu_halt();



	/* システムコールの初期化 */
	syscall_init();

	/* ファイルシステムの初期化 */
	fs_init(_fs_start);

	/* スケジューラの初期化 */
	sched_init();

	/* スケジューラの開始 */
	sched_start();

	/* initアプリ起動 */
	exec(open(INIT_APP));

	/* haltして待つ */
	while (1)
		cpu_halt();
}

void do_nic_interrupt(void)
{
	puts("\r\nNIC INTR \r\n");
	/* PICへ割り込み処理終了を通知(EOI) */
	set_pic_eoi(NIC_INTR_NO);
}
