/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#include "sdcard.h"		

#define SD_CARD		0
#define SPI_FLASH	1
#define INTER_FLASH	2

#define FLASH_SECTOR_COUNT	1024
#define FLASH_SECTOR_SIZE	4096
#define FLASH_BLOCK_SZIE	1

extern sd_card_info_struct sd_cardinfo;



/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;

	switch (pdrv) {
	case SD_CARD :
		return 0;

	case SPI_FLASH :
		
		return stat;

	case INTER_FLASH :
		
		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;

	switch (pdrv) {
	case SD_CARD :
		stat &=~STA_NOINIT;
		return 0;
	case SPI_FLASH :
		return stat;

	case INTER_FLASH :
		
		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
//	int result;
	sd_error_enum SD_stat = SD_OK;
	switch (pdrv) {
	case SD_CARD :
		if(count>1)
		{
			SD_stat = sd_multiblocks_read((uint32_t*)buff,sector * sd_cardinfo.card_blocksize,sd_cardinfo.card_blocksize,count);
		}
		else
		{
			SD_stat = sd_block_read((uint32_t*)buff,sector * sd_cardinfo.card_blocksize,sd_cardinfo.card_blocksize);
		}
		if(SD_stat == SD_OK)
		{
			res = RES_OK;
		}
		else
		{
			res = RES_ERROR;
		}
		return res;

	case SPI_FLASH :
		return res;

	case INTER_FLASH :
		return res;
	}
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
//	int result;
	sd_error_enum SD_stat = SD_OK;
	switch (pdrv) {
	case SD_CARD :
		if(count>1)
		{
			SD_stat = sd_multiblocks_write((uint32_t*)buff,sector * sd_cardinfo.card_blocksize,sd_cardinfo.card_blocksize,count);
		}
		else
		{
			SD_stat = sd_block_write((uint32_t*)buff,sector * sd_cardinfo.card_blocksize,sd_cardinfo.card_blocksize);
		}
		if(SD_stat == SD_OK)
		{
			res = RES_OK;
		}
		else
		{
			res = RES_ERROR;
		}
		return res;
	case SPI_FLASH :
		return res;
	case INTER_FLASH :
		return res;
	}
	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
//	int result;

	switch (pdrv) {
	case SD_CARD :
		switch (cmd) 
		{
			case  GET_SECTOR_COUNT:
				*(DWORD*)buff = sd_cardinfo.card_capacity / (sd_cardinfo.card_blocksize);
				break;
			case GET_SECTOR_SIZE:
				*(WORD*)buff = sd_cardinfo.card_blocksize;
				break;
			case GET_BLOCK_SIZE:
				*(DWORD *)buff = sd_cardinfo.card_blocksize;
				break;
		}
		res = RES_OK;
		return res;
	case SPI_FLASH :
		return res;
	case INTER_FLASH :
		return res;
	}

	return RES_PARERR;
}

DWORD get_fattime (void)
{
	return 0;
}

