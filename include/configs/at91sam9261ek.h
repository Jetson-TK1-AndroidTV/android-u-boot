/*
 * (C) Copyright 2005
 * M. Amine SAYA ATMEL Rousset, France.
 *
 * Rick Bronson <rick@efn.org>
 *
 * Configuation settings for the AT91SAM9261EK board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * If we are developing, we might want to start armboot from ram
 * so we MUST NOT initialize critical regs like mem-timing ...
 */

/* ARM asynchronous clock */
#define AT91C_MAIN_CLOCK	179712000	/* from 18.432 MHz crystal (18432000 / 4 * 39) */
#define AT91C_MASTER_CLOCK	99300000	/* peripheral clock (AT91C_MASTER_CLOCK / 3) */
/* #define AT91C_MASTER_CLOCK	48000000 */
/* #define AT91C_MASTER_CLOCK	44928000 */	/* peripheral clock (AT91C_MASTER_CLOCK / 4) */

#define AT91_SLOW_CLOCK		32768	/* slow clock */

#define CFG_HZ 1000

#define CONFIG_AT91SAM9261EK		1	/* on an AT91SAM9261EK Board	 */
#undef  CONFIG_USE_IRQ			        /* we don't need IRQ/FIQ stuff */
#define CONFIG_CMDLINE_TAG	        1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS        1
#define CONFIG_INITRD_TAG	        1

/* define this to include the functionality of boot.bin in u-boot */
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SKIP_RELOCATE_UBOOT
/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN	(CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

#define CONFIG_BAUDRATE         115200

/*
 * Hardware drivers
 */

/* define one of these to choose the DBGU, USART0  or USART1 as console */
#define CONFIG_DBGU       1
#undef CONFIG_USART0
#undef CONFIG_USART1
#undef CONFIG_USART2

#undef	CONFIG_HWFLOW			/* don't include RTS/CTS flow control support	*/

#undef	CONFIG_MODEM_SUPPORT		/* disable modem initialization stuff */

#define CONFIG_BOOTDELAY      3

/* #define CONFIG_ENV_OVERWRITE  1 */
#define BOARD_LATE_INIT		1

#define CONFIG_COMMANDS		\
		       ((CONFIG_CMD_DFL	| \
                        CFG_CMD_NET | \
			CFG_CMD_PING | \
                        CFG_CMD_ENV | \
                        CFG_CMD_USB | \
                        CFG_CMD_FLASH | \
			CFG_CMD_NAND | \
			CFG_CMD_AUTOSCRIPT | \
                        CFG_CMD_FAT ) & \
		      ~(CFG_CMD_BDI | \
			CFG_CMD_IMLS | \
			CFG_CMD_IMI | \
			CFG_CMD_FPGA | \
			CFG_CMD_MISC | \
			CFG_CMD_LOADS))

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define NAND_MAX_CHIPS		       1	/* Max number of NAND devices	*/
#define CFG_MAX_NAND_DEVICE            1	/* Max number of NAND devices	*/
#define CFG_NAND_BASE           0x40000000
#define CONFIG_NEW_NAND_CODE

#define ADDR_COLUMN                    1
#define ADDR_PAGE                      2
#define ADDR_COLUMN_PAGE               3

#define NAND_ChipID_UNKNOWN	       0
#define NAND_MAX_FLOORS                1
#undef  CFG_NAND_WP

/* These timings are specific to MT29F2G16AAB 256Mb (Micron) 
 * at MCK = 100 MHZ
 */
#define AT91C_SM_NWE_SETUP	(1 << 0)
#define AT91C_SM_NCS_WR_SETUP	(0 << 8)
#define AT91C_SM_NRD_SETUP	(1 << 16)
#define AT91C_SM_NCS_RD_SETUP	(0 << 24)
  
#define AT91C_SM_NWE_PULSE 	(3 << 0)
#define AT91C_SM_NCS_WR_PULSE	(3 << 8)
#define AT91C_SM_NRD_PULSE	(3 << 16)
#define AT91C_SM_NCS_RD_PULSE	(3 << 24)
  
#define AT91C_SM_NWE_CYCLE 	(5 << 0)
#define AT91C_SM_NRD_CYCLE	(5 << 16)

#define AT91C_SM_TDF	        (2 << 16)		



#define CONFIG_NR_DRAM_BANKS            1
#define PHYS_SDRAM                      0x20000000
#define PHYS_SDRAM_SIZE                 0x4000000  /* 64 megs */

#define CFG_MEMTEST_START		PHYS_SDRAM
#define CFG_MEMTEST_END			CFG_MEMTEST_START + PHYS_SDRAM_SIZE - 262144

#define CONFIG_DRIVER_ETHER             1
#define CONFIG_DRIVER_DM9000            1
#define CONFIG_NET_RETRY_COUNT		5000
#define CONFIG_TFTP_TIMEOUT             2500
#define CONFIG_AT91C_USE_RMII
#define CONFIG_DM9000_BASE              0x30000000
#define DM9000_IO			CONFIG_DM9000_BASE
#define DM9000_DATA			(CONFIG_DM9000_BASE+4)
#undef  CONFIG_DM9000_USE_8BIT
#define CONFIG_DM9000_USE_16BIT         1
#undef  CONFIG_DM9000_USE_32BIT
#undef  CONFIG_DM9000_DEBUG
/* #define CONFIG_DM9000_DEBUG             1 */

#ifdef  CONFIG_DRIVER_DM9000
/* 
 * SMC Chip Select 2 Timings for DM9000 (davicom).
 * These timings were calculated for MASTER_CLOCK = 48000000 
 * according to DM9000 timings. Refer to SMC user interface 
 * in AT91SAM9261 datasheet to learn how to regenerate these
 * values in case MASTER_CLOCK changes.
 */
/*
#define AT91C_DM9000_NWE_SETUP	        (1 << 0)		
#define AT91C_DM9000_NCS_WR_SETUP	(0 << 8)		
#define AT91C_DM9000_NRD_SETUP	        (1 << 16)		
#define AT91C_DM9000_NCS_RD_SETUP	(0 << 24)		
  
#define AT91C_DM9000_NWE_PULSE 	        (2 << 0)		
#define AT91C_DM9000_NCS_WR_PULSE	(4 << 8)		
#define AT91C_DM9000_NRD_PULSE	        (2 << 16)		
#define AT91C_DM9000_NCS_RD_PULSE	(4 << 24)		
  
#define AT91C_DM9000_NWE_CYCLE 	        (8 << 0)		
#define AT91C_DM9000_NRD_CYCLE	        (8 << 16)		
  
#define AT91C_DM9000_TDF	        (1 << 16)		
*/

/*
 * SMC Chip Select 2 Timings for DM9000 (davicom).
 * These timings were calculated for MASTER_CLOCK = 100000000 
 * according to DM9000 timings. Refer to SMC user interface 
 * in AT91SAM9261 datasheet to learn how to regenerate these
 * values in case MASTER_CLOCK changes.
 */
#define AT91C_DM9000_NWE_SETUP	        (2 << 0)		
#define AT91C_DM9000_NCS_WR_SETUP	(0 << 8)		
#define AT91C_DM9000_NRD_SETUP	        (2 << 16)		
#define AT91C_DM9000_NCS_RD_SETUP	(0 << 24)		
  
#define AT91C_DM9000_NWE_PULSE 	        (4 << 0)		
#define AT91C_DM9000_NCS_WR_PULSE	(8 << 8)		
#define AT91C_DM9000_NRD_PULSE	        (4 << 16)		
#define AT91C_DM9000_NCS_RD_PULSE	(8 << 24)		
  
#define AT91C_DM9000_NWE_CYCLE 	        (16 << 0)		
#define AT91C_DM9000_NRD_CYCLE	        (16 << 16)		
  
#define AT91C_DM9000_TDF	        (1 << 16)		

#endif /* CONFIG_DRIVER_DM9000 */

#define CONFIG_HAS_DATAFLASH		1
#define CFG_SPI_WRITE_TOUT		(50*CFG_HZ)

/* AC Characteristics */
/* DLYBS = tCSS = 250ns min and DLYBCT = tCSH = 250ns */
#define DATAFLASH_TCSS	(0x1a << 16)
#define DATAFLASH_TCHS	(0x1 << 24)

#define CFG_MAX_DATAFLASH_BANKS 	2
#define CFG_MAX_DATAFLASH_PAGES 	16384
#define CFG_DATAFLASH_LOGIC_ADDR_CS0	0xC0000000	/* Logical adress for CS0 */
#define CFG_DATAFLASH_LOGIC_ADDR_CS3	0xD0000000	/* Logical adress for CS3 */

#define CFG_NO_FLASH			1
#define PHYS_FLASH_1			0x10000000
#define PHYS_FLASH_SIZE			0x800000  /* 2 megs main flash */
#define CFG_FLASH_BASE			PHYS_FLASH_1
#define CFG_MAX_FLASH_BANKS		1
#define CFG_MAX_FLASH_SECT		256
#define CFG_FLASH_ERASE_TOUT		(2*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT		(2*CFG_HZ) /* Timeout for Flash Write */

#ifdef CFG_ENV_IS_IN_DATAFLASH
#define CFG_ENV_OFFSET			0x4000
#define CFG_ENV_ADDR			(CFG_DATAFLASH_LOGIC_ADDR_CS0 + CFG_ENV_OFFSET)
#define CFG_ENV_SIZE			0x4000  /* 0x8000 */
#endif

#ifdef CFG_ENV_IS_IN_NAND
#define CFG_ENV_OFFSET		0x60000		/* environment starts here  */
#define	CFG_ENV_OFFSET_REDUND	0x80000		/* redundant environment starts here */
#define CFG_ENV_SIZE    	0x20000 	/* 1 sector = 128kB */
#endif

#ifdef CFG_ENV_IS_IN_FLASH
#ifdef CONFIG_BOOTBINFUNC
#define CFG_ENV_ADDR			(PHYS_FLASH_1 + 0x60000)  /* after u-boot.bin */
#define CFG_ENV_SIZE			0x10000 /* sectors are 64K here */
#else
#define CFG_ENV_ADDR			(PHYS_FLASH_1 + 0xe000)  /* between boot.bin and u-boot.bin.gz */
#define CFG_ENV_SIZE			0x2000  /* 0x8000 */
#endif
#endif

/* Add LCD stuff */
#define  CONFIG_LCD
/* #undef  CONFIG_LCD_LOGO */
#define CONFIG_LCD_LOGO
#undef LCD_TEST_PATTERN
#define CONFIG_LCD_INFO
#define CONFIG_LCD_INFO_BELOW_LOGO
/* #define CFG_INVERT_COLORS */
#define CFG_WHITE_ON_BLACK

/* Add USB stuff */
#define CONFIG_USB_OHCI			1
#define CONFIG_USB_STORAGE		1
#define CONFIG_DOS_PARTITION	        1
#define LITTLEENDIAN 			1

#define CFG_LOAD_ADDR		0x23f00000  /* default load address */

#ifdef CONFIG_BOOTBINFUNC
#define CFG_BOOT_SIZE		0x00 /* 0 KBytes */
#define CFG_U_BOOT_BASE		PHYS_FLASH_1
#define CFG_U_BOOT_SIZE		0x60000 /* 384 KBytes */
#else
#define CFG_BOOT_SIZE		0x6000 /* 24 KBytes */
#define CFG_U_BOOT_BASE		(PHYS_FLASH_1 + 0x10000)
#define CFG_U_BOOT_SIZE		0x10000 /* 64 KBytes */
#endif

#define CFG_BAUDRATE_TABLE	{115200 , 19200, 38400, 57600, 9600 }
#define CFG_CONSOLE_IS_SERIAL
#undef  CFG_CONSOLE_IS_LCD   
#define CFG_PROMPT		"U-Boot> "	/* Monitor Command Prompt */
#define CFG_CBSIZE		256		/* Console I/O Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args */
#define CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */


#ifndef __ASSEMBLY__
/*-----------------------------------------------------------------------
 * Board specific extension for bd_info
 *
 * This structure is embedded in the global bd_info (bd_t) structure
 * and can be used by the board specific code (eg board/...)
 */

struct bd_info_ext {
	/* helper variable for board environment handling
	 *
	 * env_crc_valid == 0    =>   uninitialised
	 * env_crc_valid  > 0    =>   environment crc in flash is valid
	 * env_crc_valid  < 0    =>   environment crc in flash is invalid
	 */
	int env_crc_valid;
};
#endif


#define CONFIG_STACKSIZE	(32*1024)	/* regular stack */

#ifdef CONFIG_USE_IRQ
#error CONFIG_USE_IRQ not supported
#endif

#endif
