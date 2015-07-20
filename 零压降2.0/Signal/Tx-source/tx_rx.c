
unsigned char nbits_mask[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f};
/*
 * 5bits_0 -- byte[0]:bits[7:3]
 * 5bits_1 -- byte[0]:bits[2:0] + byte[1]:bits[7:6]
 * 5bits_2 -- byte[1]:bits[5:1]
 *
 * 返回5-bits个数
 */
int split_bytes_to_5bits(const unsigned char *bytes, int byten, unsigned char *bits_5, int bitn)
{
	int i, ret;
	int byte_ind, shift_bits;
	int bits5_ind, had_get_bits, need_bits;
	unsigned char *pch;
	
	bits5_ind = byten * 8;
	if (bits5_ind % 5)
		bits5_ind = bits5_ind/5 + 1;
	else
		bits5_ind /= 5;

	if (bitn < bits5_ind)
		return -1;

	ret = bits5_ind;

	for (i=0, pch=bits_5; i<bits5_ind; ++i)
		*pch++ = 0;

	byte_ind 	= 0;
	bits5_ind 	= 0;
	had_get_bits	= 0;
	while (byte_ind < byten) {
		need_bits  = 5 - had_get_bits;
		shift_bits = 8 - need_bits;
		bits_5[bits5_ind++] |= (bytes[byte_ind] & (nbits_mask[need_bits]<<shift_bits)) >> shift_bits;
		if (shift_bits >= 5) {
			shift_bits -= 5;
			bits_5[bits5_ind++] = (bytes[byte_ind] & (nbits_mask[5]<<shift_bits)) >> shift_bits;
		}
		
		if (0 != shift_bits) {
			bits_5[bits5_ind] = (bytes[byte_ind] & (nbits_mask[shift_bits])) << (5-shift_bits);
			had_get_bits    = shift_bits;
		} else {
			had_get_bits    = 0;
		}
		
		++byte_ind;
	}
	
	return ret;	
}

/*
 * 返回组合的字节个数
 */
int merge_5bits_to_bytes(unsigned char *bytes, int byten, const unsigned char *bits_5, int bitn)
{
	int i, ret;
	unsigned char *pch;

	int bits5_ind;
	int byte_ind, free_bits;
	
	byte_ind = bitn*5;
	if (byte_ind % 8)
		byte_ind = byte_ind/8 + 1;
	else
		byte_ind /= 8;

	if (byten < byte_ind)
		return -1;

	ret = byte_ind;

	for (i=0, pch=bytes; i<byte_ind; ++i)
		*pch++ = 0;

	bits5_ind 	= 0;
	byte_ind 	= 0;
	free_bits	= 8;
	while (bits5_ind < bitn) {
		if (free_bits > 5) {
			free_bits -= 5;
			bytes[byte_ind] |= bits_5[bits5_ind] << free_bits;
		} else if  (5 == free_bits) {
			bytes[byte_ind++] |= bits_5[bits5_ind];
			free_bits = 8;
		} else {
			bytes[byte_ind++] |= (bits_5[bits5_ind] & (nbits_mask[free_bits]<<(5-free_bits))) >> (5-free_bits);
			free_bits = 5 - free_bits;
			bytes[byte_ind] = (bits_5[bits5_ind] & nbits_mask[free_bits]) << (8-free_bits);
			free_bits = 8 - free_bits;
		}
		
		++bits5_ind;
	}

	return ret;
}

