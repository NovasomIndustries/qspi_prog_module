/*
 * spi_module.h -- simple synchronous userspace interface to SPI devices
 *
 * Copyright (C) 2012 Novasis
 *	Filippo Visocchi, <filippo.visocchi@novasis.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 export ARCH=arm; export CROSS_COMPILE=arm-linux-; export PATH=$PATH:/Devel/NovaSOM535/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin;$make -f Makefile $file
 */


#ifndef SPIDEV_H
#define SPIDEV_H

#define DPRINTK(fmt, args...) printk(KERN_DEBUG "%s: " fmt, __func__ , ## args)
#define SPRINTK(fmt, args...) printk(KERN_DEBUG "K %s: " fmt, __func__ , ## args)
#define APRINTK(fmt, args...) printk(fmt, ## args)

#define IMX6PRINTK              APRINTK
#define SPIDEV_MAJOR			153	/* assigned */
#define N_SPI_MINORS			1	/* ... up to 256 */

#define USER_BUFF_SIZE	128

/* Syntactic sugar for pad configuration */
#define IMX6_SETUP_PAD(p) \
        if (cpu_is_mx6q()) \
                mxc_iomux_v3_setup_pad(MX6Q_PAD_##p);\
        else \
                mxc_iomux_v3_setup_pad(MX6DL_PAD_##p)

#include <linux/types.h>

/* User space versions of kernel symbols for SPI clocking modes,
 * matching <linux/spi/spi.h>
 */

#define    DRIVER_NAME "imx6-ecspi2"

#define SPI_CPHA		0x01
#define SPI_CPOL		0x02

#define SPI_MODE_0		(0|0)
#define SPI_MODE_1		(0|SPI_CPHA)
#define SPI_MODE_2		(SPI_CPOL|0)
#define SPI_MODE_3		(SPI_CPOL|SPI_CPHA)

#define SPI_CS_HIGH		0x04
#define SPI_LSB_FIRST	0x08
#define SPI_3WIRE		0x10
#define SPI_LOOP		0x20
#define SPI_NO_CS		0x40
#define SPI_READY		0x80

/*---------------------------------------------------------------------------*/

/* IOCTL commands */
#define ANAREN_READY            0x10
#define ANAREN_GET_IN0          0x20
#define ANAREN_GET_IN1          0x21
#define ANAREN_SET_OUT0         0x28
#define ANAREN_SET_OUT1         0x29

#define SPI_IOC_MAGIC			'k'

/**
 * struct spi_ioc_transfer - describes a single SPI transfer
 * @tx_buf: Holds pointer to userspace buffer with transmit data, or null.
 *	If no data is provided, zeroes are shifted out.
 * @rx_buf: Holds pointer to userspace buffer for receive data, or null.
 * @len: Length of tx and rx buffers, in bytes.
 * @speed_hz: Temporary override of the device's bitrate.
 * @bits_per_word: Temporary override of the device's wordsize.
 * @delay_usecs: If nonzero, how long to delay after the last bit transfer
 *	before optionally deselecting the device before the next transfer.
 * @cs_change: True to deselect device before starting the next transfer.
 *
 * This structure is mapped directly to the kernel spi_transfer structure;
 * the fields have the same meanings, except of course that the pointers
 * are in a different address space (and may be of different sizes in some
 * cases, such as 32-bit i386 userspace over a 64-bit x86_64 kernel).
 * Zero-initialize the structure, including currently unused fields, to
 * accomodate potential future updates.
 *
 * SPI_IOC_MESSAGE gives userspace the equivalent of kernel spi_sync().
 * Pass it an array of related transfers, they'll execute together.
 * Each transfer may be half duplex (either direction) or full duplex.
 *
 *	struct spi_ioc_transfer mesg[4];
 *	...
 *	status = ioctl(fd, SPI_IOC_MESSAGE(4), mesg);
 *
 * So for example one transfer might send a nine bit command (right aligned
 * in a 16-bit word), the next could read a block of 8-bit data before
 * terminating that command by temporarily deselecting the chip; the next
 * could send a different nine bit command (re-selecting the chip), and the
 * last transfer might write some register values.
 */
struct spi_ioc_transfer {
	__u64		tx_buf;
	__u64		rx_buf;

	__u32		len;
	__u32		speed_hz;

	__u16		delay_usecs;
	__u8		bits_per_word;
	__u8		cs_change;
	__u32		pad;

	/* If the contents of 'struct spi_ioc_transfer' ever change
	 * incompatibly, then the ioctl number (currently 0) must change;
	 * ioctls with constant size fields get a bit more in the way of
	 * error checking than ones (like this) where that field varies.
	 *
	 * NOTE: struct layout is the same in 64bit and 32bit userspace.
	 */
};

/* not all platforms use <asm-generic/ioctl.h> or _IOC_TYPECHECK() ... */
#define SPI_MSGSIZE(N) \
	((((N)*(sizeof (struct spi_ioc_transfer))) < (1 << _IOC_SIZEBITS)) \
		? ((N)*(sizeof (struct spi_ioc_transfer))) : 0)
#define SPI_IOC_MESSAGE(N) _IOW(SPI_IOC_MAGIC, 0, char[SPI_MSGSIZE(N)])


/* Read / Write of SPI mode (SPI_MODE_0..SPI_MODE_3) */
#define SPI_IOC_RD_MODE			_IOR(SPI_IOC_MAGIC, 1, __u8)
#define SPI_IOC_WR_MODE			_IOW(SPI_IOC_MAGIC, 1, __u8)

/* Read / Write SPI bit justification */
#define SPI_IOC_RD_LSB_FIRST		_IOR(SPI_IOC_MAGIC, 2, __u8)
#define SPI_IOC_WR_LSB_FIRST		_IOW(SPI_IOC_MAGIC, 2, __u8)

/* Read / Write SPI device word length (1..N) */
#define SPI_IOC_RD_BITS_PER_WORD	_IOR(SPI_IOC_MAGIC, 3, __u8)
#define SPI_IOC_WR_BITS_PER_WORD	_IOW(SPI_IOC_MAGIC, 3, __u8)

/* Read / Write SPI device default max speed hz */
#define SPI_IOC_RD_MAX_SPEED_HZ		_IOR(SPI_IOC_MAGIC, 4, __u32)
#define SPI_IOC_WR_MAX_SPEED_HZ		_IOW(SPI_IOC_MAGIC, 4, __u32)

/* User func */
#define SET_NORDIC_CE       0x5a
#define GET_NORDIC_IRQ      0xa5

#endif /* SPIDEV_H */


/*

FRA

    SPI_IOC_RD_MODE, SPI_IOC_WR_MODE ... pass a pointer to a byte which will
	return (RD) or assign (WR) the SPI transfer mode.  Use the constants
	SPI_MODE_0..SPI_MODE_3; or if you prefer you can combine SPI_CPOL
	(clock polarity, idle high iff this is set) or SPI_CPHA (clock phase,
	sample on trailing edge iff this is set) flags.
	Note that this request is limited to SPI mode flags that fit in a
	single byte.

    SPI_IOC_RD_MODE32, SPI_IOC_WR_MODE32 ... pass a pointer to a uin32_t
	which will return (RD) or assign (WR) the full SPI transfer mode,
	not limited to the bits that fit in one byte.

    SPI_IOC_RD_LSB_FIRST, SPI_IOC_WR_LSB_FIRST ... pass a pointer to a byte
	which will return (RD) or assign (WR) the bit justification used to
	transfer SPI words.  Zero indicates MSB-first; other values indicate
	the less common LSB-first encoding.  In both cases the specified value
	is right-justified in each word, so that unused (TX) or undefined (RX)
	bits are in the MSBs.

    SPI_IOC_RD_BITS_PER_WORD, SPI_IOC_WR_BITS_PER_WORD ... pass a pointer to
	a byte which will return (RD) or assign (WR) the number of bits in
	each SPI transfer word.  The value zero signifies eight bits.

    SPI_IOC_RD_MAX_SPEED_HZ, SPI_IOC_WR_MAX_SPEED_HZ ... pass a pointer to a
	u32 which will return (RD) or assign (WR) the maximum SPI transfer
	speed, in Hz.  The controller can't necessarily assign that specific
	clock speed.


*/






