"""

	A script to output the contents of a regdump into a human-readable format.

	struct RegDumpEntry
	{
		uint32_t	clkDelta;	// Delta of previous CLK counter (CPU Core clock cycles) value at the time of accessing to the register
		uint8_t 	reg; 		// Register index + Flag (msb - 0: write, 1: read)
		uint8_t 	value;		// Written value. Not used for reading.
		uint16_t	padding;	// Not used (yet?)
	};

"""

import os
import sys
import struct

regbase = 0x2000
entry_format = '=IBBH' 		# uint32_t + uint8_t + uint8_t + uint16_t

def DumpEntry (entry, clk_counter):
	print (f"{entry[0]}, ", end="")
	if entry[1] & 0x80:
		print (f"read {hex(regbase+(entry[1]&~0x80))}")
	else:
		print (f"write {hex(regbase+entry[1])}={hex(entry[2])}")

def Main (regdump_file):
	f = open(regdump_file, 'rb')
	binarycontent = f.read(-1)
	f.close()
	regdump_size = len(binarycontent) 
	entry_size = struct.calcsize(entry_format)
	num_entries = int(len(binarycontent) / entry_size)
	print (f"regdump size: {regdump_size}, entry size: {entry_size}, entries count: {num_entries}")
	offset = 0
	clk_counter = 0
	for n in range(num_entries):
		entry = struct.unpack_from(entry_format, binarycontent, offset)
		DumpEntry (entry, clk_counter)
		clk_counter += entry[0]
		offset = offset + entry_size

if __name__ == '__main__':
	if (len(sys.argv) < 1):
		print ("Use: python DumpRegdump.py <file.regdump>")
	else:
		Main(sys.argv[1])
