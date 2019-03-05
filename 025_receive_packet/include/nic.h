#pragma once

#define NIC_BUS_NUM	0x00
#define NIC_DEV_NUM	0x19
#define NIC_FUNC_NUM	0x0

#define NIC_REG_IMS	0x00d0
#define NIC_REG_IMC	0x00d8
#define NIC_REG_RCTL	0x0100
#define NIC_REG_RDBAL	0x2800
#define NIC_REG_RDBAH	0x2804
#define NIC_REG_RDLEN	0x2808
#define NIC_REG_RDH	0x2810
#define NIC_REG_RDT	0x2818

#define NIC_RCTL_EN	(1U << 1)
#define NIC_RCTL_SBP	(1U << 2)
#define NIC_RCTL_UPE	(1U << 3)
#define NIC_RCTL_MPE	(1U << 4)
#define NIC_RCTL_LPE	(1U << 5)
#define NIC_RCTL_LBM_NO_LOOPBACK	(0b00 << 6)
#define NIC_RCTL_LBM_LOOPBACK	(0b11 << 6)
#define NIC_RCTL_RDMTS_TH_1_2	(0b00 << 8)
#define NIC_RCTL_RDMTS_TH_1_4	(0b01 << 8)
#define NIC_RCTL_RDMTS_TH_1_8	(0b10 << 8)
#define NIC_RCTL_MO_B47_36	(0b00 << 12)
#define NIC_RCTL_MO_B46_35	(0b01 << 12)
#define NIC_RCTL_MO_B45_34	(0b10 << 12)
#define NIC_RCTL_MO_B43_32	(0b11 << 12)
#define NIC_RCTL_BAM	(1U << 15)
#define NIC_RCTL_BSIZE_2048B	(0b00 << 16)
#define NIC_RCTL_BSIZE_1024B	(0b01 << 16)
#define NIC_RCTL_BSIZE_512B	(0b10 << 16)
#define NIC_RCTL_BSIZE_256B	(0b11 << 16)
#define NIC_RCTL_BSIZE_16384B	(0b01 << 16)
#define NIC_RCTL_BSIZE_8192B	(0b10 << 16)
#define NIC_RCTL_BSIZE_4096B	(0b11 << 16)
#define NIC_RCTL_VFE	(1U << 18)
#define NIC_RCTL_CFIEN	(1U << 19)
#define NIC_RCTL_CFI	(1U << 20)
#define NIC_RCTL_DPF	(1U << 22)
#define NIC_RCTL_PMCF	(1U << 23)
#define NIC_RCTL_BSEX	(1U << 25)
#define NIC_RCTL_SECRC	(1U << 26)

#define NIC_RDESC_STAT_DD	(1U << 0)
#define NIC_RDESC_STAT_EOP	(1U << 1)
#define NIC_RDESC_STAT_IXSM	(1U << 2)
#define NIC_RDESC_STAT_VP	(1U << 3)
#define NIC_RDESC_STAT_TCPCS	(1U << 5)
#define NIC_RDESC_STAT_IPCS	(1U << 6)
#define NIC_RDESC_STAT_PIF	(1U << 7)

#define PACKET_BUFFER_SIZE	2048
#define PACKET_BUFFER_BIT	NIC_RCTL_BSIZE_2048B

void nic_init(void);
unsigned int get_nic_reg_base(void);
unsigned int get_nic_reg(unsigned short reg);
void set_nic_reg(unsigned short reg, unsigned int val);
void dump_nic_ims(void);
unsigned short receive_packet(void *buf);
unsigned short dump_packet(void);
