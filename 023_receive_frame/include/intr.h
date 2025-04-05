#ifndef _INTR_H_
#define _INTR_H_

void set_intr_desc(unsigned char intr_no, void *handler);
void intr_init(void);
void intr_load(void);
void set_intr_desc_type(unsigned char intr_no, void *handler, unsigned short type);

#endif
