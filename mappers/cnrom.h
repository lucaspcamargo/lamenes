/*
 * LameNES - Nintendo Entertainment System (NES) emulator
 *
 * Copyright (c) 2005, Joey Loman, <joey@lamenes.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * cnrom.h - NES Mapper 3: CNROM
 */

void
cnrom_switch_chr(int bank)
{
	int prg_size;
	int chr_size;

	int chr_start;

	unsigned int address;

	address = 0x0000;

	prg_size = 16384;
	chr_size = 8192;

	chr_start = prg_size * PRG;

	#ifdef MAPPER_DEBUG
	printf("[%d] chr_switch: reading from %d (offset: %x) into ppu mem address: 0x%x (bank: %d)\n",
		debug_cnt,16 + chr_start + (bank * chr_size),16 + chr_start + (bank * chr_size),address,bank);
	#endif

	memcpy(ppu_memory + address, romcache + 16 + chr_start + (bank * chr_size), chr_size);
}

void
cnrom_access(unsigned int address,unsigned char data)
{
	#ifdef MAPPER_DEBUG
	printf("[%d] cnrom access: 0x%x\n",debug_cnt,address);
	#endif

	if(address > 0x7fff && address < 0x10000) {
		cnrom_switch_chr(data & (CHR - 1));
	}
}
