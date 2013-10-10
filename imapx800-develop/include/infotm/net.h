/*
 * net.h
 * 
 * Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Main head file to export irnet API.
 *
 * Sololz <sololz.luo@gmail.com>.
 *      
 * Revision History: 
 * -----------------
 * 07/06/2011
 * File created by Sololz.
 */

#ifndef __NET_H__
#define __NET_H__

#include <asm/byteorder.h>

/* ############################################################################## */
/* Driver layer. */

extern int gmac_initialize(void);

/* ############################################################################## */
/* ETH layer API connecting driver and protocol. */

struct eth_device {
	unsigned char enetaddr[6];
	int	mem;
	volatile uint8_t **recv_packets;

	int (*init)(struct eth_device*);
	int (*send)(struct eth_device*, volatile void* packet, int length);
	int (*recv)(struct eth_device*);
	void (*halt)(struct eth_device*);
	void (*recv_process)(volatile uint8_t *inpkt, int len);
};

extern int eth_register(struct eth_device* dev);
extern struct eth_device *eth_get_dev(void);
extern void eth_prepare(void (*process)(volatile uint8_t *inpkt, int len), 
		volatile uint8_t **packets);
extern void eth_memcpy(int mem);
extern int eth_init(void);
extern int eth_send(volatile void *packet, int length);
extern int eth_rx(void);
extern void eth_halt(void);

/* ############################################################################## */
/* Sonet layer. */

/* Sonet operations. */
enum { 
	BOOTP,
	RARP,
	ARP,
	TFTP,
	DHCP,
	PING,
	DNS,
	NFS,
	CDP,
	NETCONS,
	SNTP,
	IRNET,
    SOIP,
};

/** Initialize sonet data structure. */
int sonet_init(void);

/** Free sonet. */
void sonet_free(void);

/** Access network though sonet API. */
int sonet_access(int request);

/* ############################################################################## */
/* Irnet layer. */

/** Prepare irnet protocol data. */
enum {
	IRNET_ACS_READ = 1,
	IRNET_ACS_WRITE,
};
void irnet_prepare(uint8_t *buf, uint32_t size, uint32_t acs);

/** Get irnet left access data size not consumed/received. */
inline uint32_t irnet_leftsize(void);

/**
 * Try to connect to pc through ethernet, only success if pc server 
 * is ready.
 * 0 represents not connected, 1 means ok.
 */
int eth_connect(void);


/**
 * Read data from ethernet, it will block till all required data
 * recieved.
 * Returns the size of data read, 0 means some errors occured.
 */
int eth_read(uint8_t *buf, int len);

/** Write connect to ethernet by sending data. */
int eth_write(uint8_t *buf, int len);

/* vs interface */
extern int eth_vs_reset(void);
extern int eth_vs_align(void);

#endif	/* __NET_H__ */
