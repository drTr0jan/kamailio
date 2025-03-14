/*
 *  Crc - 32 + 16 BIT ANSI X3.66 + CCITT CRC checksum files
 *
 * Copyright (C) 2001-2003 FhG Fokus
 *
 * This file is part of Kamailio, a free SIP server.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Kamailio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * Kamailio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
/*!
 * \file
 * \brief Kamailio core ::  Crc - 32 + 16 BIT ANSI X3.66 + CCITT CRC checksum files
 * \ingroup core
 *
 * Module: \ref core
 */


#include <stdio.h>
#include "str.h"
#include "ut.h"
#include "crc.h"

#define OK 0
#define ERROR (-1)
#define UPDC32(ch, crc) (crc_32_tab[((crc) ^ (ch)) & 0xff] ^ ((crc) >> 8))
#define UPDCIT(ch, crc) (ccitt_tab[((crc) ^ (ch)) & 0xff] ^ ((crc) >> 8))
#define UPDC16(ch, crc) (crc_16_tab[((crc) ^ (ch)) & 0xff] ^ ((crc) >> 8))

/*
 * Copyright (C) 1986 Gary S. Brown.  You may use this program, or
 * code or tables extracted from it, as desired without restriction.
 */
/* First, the polynomial itself and its table of feedback terms.  The  */
/* polynomial is                                                       */
/* X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0 */
/* Note that we take it "backwards" and put the highest-order term in  */
/* the lowest-order bit.  The X^32 term is "implied"; the LSB is the   */
/* X^31 term, etc.  The X^0 term (usually shown as "+1") results in    */
/* the MSB being 1.                                                    */

/* added CCITT (X^16+X^12+X^5+X^0) and CRC-16 (X^16+X^15+X^2+X^0)      */
/* polynomials (Heinz Repp, Feb 4 1997)                                */

/* Note that the usual hardware shift register implementation, which   */
/* is what we're using (we're merely optimizing it by doing eight-bit  */
/* chunks at a time) shifts bits into the lowest-order term.  In our   */
/* implementation, that means shifting towards the right.  Why do we   */
/* do it this way?  Because the calculated CRC must be transmitted in  */
/* order from highest-order term to lowest-order term.  UARTs transmit */
/* characters in order from LSB to MSB.  By storing the CRC this way,  */
/* we hand it to the UART in the order low-byte to high-byte; the UART */
/* sends each low-bit to hight-bit; and the result is transmission bit */
/* by bit from highest- to lowest-order term without requiring any bit */
/* shuffling on our part.  Reception works similarly.                  */

/* The feedback terms table consists of 256, 32-bit entries.  Notes:   */
/*                                                                     */
/*  1. The table can be generated at runtime if desired; code to do so */
/*     is shown later.  It might not be obvious, but the feedback      */
/*     terms simply represent the results of eight shift/xor opera-    */
/*     tions for all combinations of data and CRC register values.     */
/*                                                                     */
/*  2. The CRC accumulation logic is the same for all CRC polynomials, */
/*     be they sixteen or thirty-two bits wide.  You simply choose the */
/*     appropriate table.  Alternatively, because the table can be     */
/*     generated at runtime, you can start by generating the table for */
/*     the polynomial in question and use exactly the same "updcrc",   */
/*     if your application needn't simultaneously handle two CRC       */
/*     polynomials.  (Note, however, that XMODEM is strange.)          */
/*                                                                     */
/*  3. For 16-bit CRCs, the table entries need be only 16 bits wide;   */
/*     of course, 32-bit entries work OK if the high 16 bits are zero. */
/*                                                                     */
/*  4. The values must be right-shifted by eight bits by the "updcrc"  */
/*     logic; the shift must be unsigned (bring in zeroes).  On some   */
/*     hardware you could probably optimize the shift in assembler by  */
/*     using byte-swap instructions.                                   */

unsigned long int crc_32_tab[] = {/* CRC polynomial 0xedb88320 */
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
		0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
		0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
		0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
		0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
		0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
		0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
		0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
		0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
		0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
		0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
		0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
		0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
		0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
		0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
		0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
		0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
		0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
		0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
		0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
		0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
		0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
		0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
		0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
		0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
		0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
		0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
		0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
		0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
		0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
		0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
		0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
		0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
		0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
		0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
		0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
		0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
		0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
		0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
		0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
		0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d};

unsigned short int ccitt_tab[] = {
		/* CRC polynomial 0x8408 */
		0x0000,
		0x1189,
		0x2312,
		0x329B,
		0x4624,
		0x57AD,
		0x6536,
		0x74BF,
		0x8C48,
		0x9DC1,
		0xAF5A,
		0xBED3,
		0xCA6C,
		0xDBE5,
		0xE97E,
		0xF8F7,
		0x1081,
		0x0108,
		0x3393,
		0x221A,
		0x56A5,
		0x472C,
		0x75B7,
		0x643E,
		0x9CC9,
		0x8D40,
		0xBFDB,
		0xAE52,
		0xDAED,
		0xCB64,
		0xF9FF,
		0xE876,
		0x2102,
		0x308B,
		0x0210,
		0x1399,
		0x6726,
		0x76AF,
		0x4434,
		0x55BD,
		0xAD4A,
		0xBCC3,
		0x8E58,
		0x9FD1,
		0xEB6E,
		0xFAE7,
		0xC87C,
		0xD9F5,
		0x3183,
		0x200A,
		0x1291,
		0x0318,
		0x77A7,
		0x662E,
		0x54B5,
		0x453C,
		0xBDCB,
		0xAC42,
		0x9ED9,
		0x8F50,
		0xFBEF,
		0xEA66,
		0xD8FD,
		0xC974,
		0x4204,
		0x538D,
		0x6116,
		0x709F,
		0x0420,
		0x15A9,
		0x2732,
		0x36BB,
		0xCE4C,
		0xDFC5,
		0xED5E,
		0xFCD7,
		0x8868,
		0x99E1,
		0xAB7A,
		0xBAF3,
		0x5285,
		0x430C,
		0x7197,
		0x601E,
		0x14A1,
		0x0528,
		0x37B3,
		0x263A,
		0xDECD,
		0xCF44,
		0xFDDF,
		0xEC56,
		0x98E9,
		0x8960,
		0xBBFB,
		0xAA72,
		0x6306,
		0x728F,
		0x4014,
		0x519D,
		0x2522,
		0x34AB,
		0x0630,
		0x17B9,
		0xEF4E,
		0xFEC7,
		0xCC5C,
		0xDDD5,
		0xA96A,
		0xB8E3,
		0x8A78,
		0x9BF1,
		0x7387,
		0x620E,
		0x5095,
		0x411C,
		0x35A3,
		0x242A,
		0x16B1,
		0x0738,
		0xFFCF,
		0xEE46,
		0xDCDD,
		0xCD54,
		0xB9EB,
		0xA862,
		0x9AF9,
		0x8B70,
		0x8408,
		0x9581,
		0xA71A,
		0xB693,
		0xC22C,
		0xD3A5,
		0xE13E,
		0xF0B7,
		0x0840,
		0x19C9,
		0x2B52,
		0x3ADB,
		0x4E64,
		0x5FED,
		0x6D76,
		0x7CFF,
		0x9489,
		0x8500,
		0xB79B,
		0xA612,
		0xD2AD,
		0xC324,
		0xF1BF,
		0xE036,
		0x18C1,
		0x0948,
		0x3BD3,
		0x2A5A,
		0x5EE5,
		0x4F6C,
		0x7DF7,
		0x6C7E,
		0xA50A,
		0xB483,
		0x8618,
		0x9791,
		0xE32E,
		0xF2A7,
		0xC03C,
		0xD1B5,
		0x2942,
		0x38CB,
		0x0A50,
		0x1BD9,
		0x6F66,
		0x7EEF,
		0x4C74,
		0x5DFD,
		0xB58B,
		0xA402,
		0x9699,
		0x8710,
		0xF3AF,
		0xE226,
		0xD0BD,
		0xC134,
		0x39C3,
		0x284A,
		0x1AD1,
		0x0B58,
		0x7FE7,
		0x6E6E,
		0x5CF5,
		0x4D7C,
		0xC60C,
		0xD785,
		0xE51E,
		0xF497,
		0x8028,
		0x91A1,
		0xA33A,
		0xB2B3,
		0x4A44,
		0x5BCD,
		0x6956,
		0x78DF,
		0x0C60,
		0x1DE9,
		0x2F72,
		0x3EFB,
		0xD68D,
		0xC704,
		0xF59F,
		0xE416,
		0x90A9,
		0x8120,
		0xB3BB,
		0xA232,
		0x5AC5,
		0x4B4C,
		0x79D7,
		0x685E,
		0x1CE1,
		0x0D68,
		0x3FF3,
		0x2E7A,
		0xE70E,
		0xF687,
		0xC41C,
		0xD595,
		0xA12A,
		0xB0A3,
		0x8238,
		0x93B1,
		0x6B46,
		0x7ACF,
		0x4854,
		0x59DD,
		0x2D62,
		0x3CEB,
		0x0E70,
		0x1FF9,
		0xF78F,
		0xE606,
		0xD49D,
		0xC514,
		0xB1AB,
		0xA022,
		0x92B9,
		0x8330,
		0x7BC7,
		0x6A4E,
		0x58D5,
		0x495C,
		0x3DE3,
		0x2C6A,
		0x1EF1,
		0x0F78,
};

unsigned short int crc_16_tab[] = {
		/* CRC polynomial 0xA001 */
		0x0000,
		0xC0C1,
		0xC181,
		0x0140,
		0xC301,
		0x03C0,
		0x0280,
		0xC241,
		0xC601,
		0x06C0,
		0x0780,
		0xC741,
		0x0500,
		0xC5C1,
		0xC481,
		0x0440,
		0xCC01,
		0x0CC0,
		0x0D80,
		0xCD41,
		0x0F00,
		0xCFC1,
		0xCE81,
		0x0E40,
		0x0A00,
		0xCAC1,
		0xCB81,
		0x0B40,
		0xC901,
		0x09C0,
		0x0880,
		0xC841,
		0xD801,
		0x18C0,
		0x1980,
		0xD941,
		0x1B00,
		0xDBC1,
		0xDA81,
		0x1A40,
		0x1E00,
		0xDEC1,
		0xDF81,
		0x1F40,
		0xDD01,
		0x1DC0,
		0x1C80,
		0xDC41,
		0x1400,
		0xD4C1,
		0xD581,
		0x1540,
		0xD701,
		0x17C0,
		0x1680,
		0xD641,
		0xD201,
		0x12C0,
		0x1380,
		0xD341,
		0x1100,
		0xD1C1,
		0xD081,
		0x1040,
		0xF001,
		0x30C0,
		0x3180,
		0xF141,
		0x3300,
		0xF3C1,
		0xF281,
		0x3240,
		0x3600,
		0xF6C1,
		0xF781,
		0x3740,
		0xF501,
		0x35C0,
		0x3480,
		0xF441,
		0x3C00,
		0xFCC1,
		0xFD81,
		0x3D40,
		0xFF01,
		0x3FC0,
		0x3E80,
		0xFE41,
		0xFA01,
		0x3AC0,
		0x3B80,
		0xFB41,
		0x3900,
		0xF9C1,
		0xF881,
		0x3840,
		0x2800,
		0xE8C1,
		0xE981,
		0x2940,
		0xEB01,
		0x2BC0,
		0x2A80,
		0xEA41,
		0xEE01,
		0x2EC0,
		0x2F80,
		0xEF41,
		0x2D00,
		0xEDC1,
		0xEC81,
		0x2C40,
		0xE401,
		0x24C0,
		0x2580,
		0xE541,
		0x2700,
		0xE7C1,
		0xE681,
		0x2640,
		0x2200,
		0xE2C1,
		0xE381,
		0x2340,
		0xE101,
		0x21C0,
		0x2080,
		0xE041,
		0xA001,
		0x60C0,
		0x6180,
		0xA141,
		0x6300,
		0xA3C1,
		0xA281,
		0x6240,
		0x6600,
		0xA6C1,
		0xA781,
		0x6740,
		0xA501,
		0x65C0,
		0x6480,
		0xA441,
		0x6C00,
		0xACC1,
		0xAD81,
		0x6D40,
		0xAF01,
		0x6FC0,
		0x6E80,
		0xAE41,
		0xAA01,
		0x6AC0,
		0x6B80,
		0xAB41,
		0x6900,
		0xA9C1,
		0xA881,
		0x6840,
		0x7800,
		0xB8C1,
		0xB981,
		0x7940,
		0xBB01,
		0x7BC0,
		0x7A80,
		0xBA41,
		0xBE01,
		0x7EC0,
		0x7F80,
		0xBF41,
		0x7D00,
		0xBDC1,
		0xBC81,
		0x7C40,
		0xB401,
		0x74C0,
		0x7580,
		0xB541,
		0x7700,
		0xB7C1,
		0xB681,
		0x7640,
		0x7200,
		0xB2C1,
		0xB381,
		0x7340,
		0xB101,
		0x71C0,
		0x7080,
		0xB041,
		0x5000,
		0x90C1,
		0x9181,
		0x5140,
		0x9301,
		0x53C0,
		0x5280,
		0x9241,
		0x9601,
		0x56C0,
		0x5780,
		0x9741,
		0x5500,
		0x95C1,
		0x9481,
		0x5440,
		0x9C01,
		0x5CC0,
		0x5D80,
		0x9D41,
		0x5F00,
		0x9FC1,
		0x9E81,
		0x5E40,
		0x5A00,
		0x9AC1,
		0x9B81,
		0x5B40,
		0x9901,
		0x59C0,
		0x5880,
		0x9841,
		0x8801,
		0x48C0,
		0x4980,
		0x8941,
		0x4B00,
		0x8BC1,
		0x8A81,
		0x4A40,
		0x4E00,
		0x8EC1,
		0x8F81,
		0x4F40,
		0x8D01,
		0x4DC0,
		0x4C80,
		0x8C41,
		0x4400,
		0x84C1,
		0x8581,
		0x4540,
		0x8701,
		0x47C0,
		0x4680,
		0x8641,
		0x8201,
		0x42C0,
		0x4380,
		0x8341,
		0x4100,
		0x81C1,
		0x8081,
		0x4040,
};

unsigned short crcitt_string_ex(char *s, int len, register unsigned short ccitt)
{
	ccitt = ~ccitt;
	while(len) {
		ccitt = UPDCIT(*s, ccitt);
		s++;
		len--;
	}
	return ~ccitt;
}

unsigned short crcitt_string(char *s, int len)
{
	return crcitt_string_ex(s, len, 0);
}

void crcitt_string_array(char *dst, str src[], int size)
{
	register int i;
	register unsigned short ccitt;
	register char *c;
	register int len;
	int str_len;

	ccitt = 0xFFFF;
	str_len = CRC16_LEN;
	for(i = 0; i < size; i++) {
		/* invalid str with positive length and null char pointer */
		if(unlikely(src[i].s == NULL))
			break;
		c = src[i].s;
		len = src[i].len;
		while(len) {
			ccitt = UPDCIT(*c, ccitt);
			c++;
			len--;
		}
	}
	ccitt = ~ccitt;
	if(int2reverse_hex(&dst, &str_len, ccitt) == -1) {
		/* bug ... printed ccitt value longer than CRC32_LEN */
		LM_CRIT("string conversion incomplete\n");
	}
	/* padding */
	while(str_len) {
		*dst = '0';
		dst++;
		str_len--;
	}
}


int crc32file(char *name)
{
	register FILE *fin;
	register unsigned long crc32;
	register unsigned short ccitt, crc16;
	register int c;
	register long charcnt;

	crc32 = 0xFFFFFFFF;
	ccitt = 0xFFFF;
	crc16 = charcnt = 0;
	if((fin = fopen(name, "rb")) == NULL) {
		perror(name);
		return ERROR;
	}
	while((c = getc(fin)) != EOF) {
		++charcnt;
		crc32 = UPDC32(c, crc32);
		ccitt = UPDCIT(c, ccitt);
		crc16 = UPDC16(c, crc16);
	}

	if(ferror(fin)) {
		perror(name);
		fclose(fin);
		return ERROR;
	} else {
		crc32 = ~crc32;
		ccitt = ~ccitt;

		printf(" %08lX  %04X  %04X%10ld   %s\n", crc32, ccitt, crc16, charcnt,
				name);
	}

	fclose(fin);
	return OK;
}

/*!
 * \brief CRC32 value from source string
 * \param source_string source string
 * \param hash_ret calulated CRC32
 */
void crc32_uint(str *source_string, unsigned int *hash_ret)
{
	unsigned int hash;
	unsigned int len;
	const char *data;

	hash = 0xffffffff;
	data = source_string->s;

	for(len = source_string->len / 4; len--; data += 4) {
		hash = crc_32_tab[((unsigned char)hash) ^ data[0]] ^ (hash >> 8);
		hash = crc_32_tab[((unsigned char)hash) ^ data[1]] ^ (hash >> 8);
		hash = crc_32_tab[((unsigned char)hash) ^ data[2]] ^ (hash >> 8);
		hash = crc_32_tab[((unsigned char)hash) ^ data[3]] ^ (hash >> 8);
	}

	for(len = source_string->len % 4; len--; data++) {
		hash = crc_32_tab[((unsigned char)hash) ^ *data] ^ (hash >> 8);
	}

	*hash_ret = ~hash;
}

/*

int main(int argc, char **argv)
{
	register int errors = 0;

	puts("  CRC-32  CCITT  CRC-16    size   name\n"
			 "-----------------------------------------------");

	while( --argc > 0)
		errors |= crc32file( *++argv);
	exit(errors != 0);
}
*/

/* End of crc.c */
