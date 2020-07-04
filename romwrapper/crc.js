
function ROL(i, b) {
    return (((i) << (b)) | ((i) >> (32 - (b))));
}

function BYTES2LONG(b, offset) {
    return b[offset + 0] << 24 |
        b[offset + 1] << 16 |
        b[offset + 2] <<  8 |
        b[offset + 3];
}

const N64_HEADER_SIZE = 0x40;
const N64_BC_SIZE = (0x1000 - N64_HEADER_SIZE);

const N64_CRC1 = 0x10;
const N64_CRC2 = 0x14;

const CHECKSUM_START = 0x00001000;
const CHECKSUM_LENGTH = 0x00100000;
const CHECKSUM_CIC6102 = 0xF8CA4DDC;
const CHECKSUM_CIC6103 = 0xA3886759;
const CHECKSUM_CIC6105 = 0xDF26F436;
const CHECKSUM_CIC6106 = 0x1FEA617A;

function Write32(Buffer, Offset, Value) {
	Buffer[Offset] = (Value & 0xFF000000) >> 24;
	Buffer[Offset + 1] = (Value & 0x00FF0000) >> 16;
	Buffer[Offset + 2] = (Value & 0x0000FF00) >> 8;
	Buffer[Offset + 3] = (Value & 0x000000FF);
}

const crc_table = [
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
];

function gen_table() {
    let crc = 0;
    let poly = 0;

	poly = 0xEDB88320;
	for (let i = 0; i < 256; i++) {
		crc = i;
		for (let j = 8; j > 0; j--) {
			if (crc & 1) crc = (crc >> 1) ^ poly;
			else crc >>= 1;
		}
		crc_table[i] = crc;
	}
}

function crc32(data, offset, len) {
	let crc = ~0;

	for (let i = 0; i < len; i++) {
		crc = (crc >> 8) ^ crc_table[(crc ^ data[i + offset]) & 0xFF];
	}

	return ~crc;
}


function N64GetCIC(data) {
	switch (crc32(data, N64_HEADER_SIZE, N64_BC_SIZE)) {
		case 0x6170A4A1: return 6101;
		case 0x90BB6CB5: return 6102;
		case 0x0B050EE0: return 6103;
		case 0x98BC2C86: return 6105;
		case 0xACC8580A: return 6106;
	}

	return 6105;
}

function N64CalcCRC(crc, data) {
	let bootcode;
	let seed;

	let t1, t2, t3;
	let t4, t5, t6;
	let r, d;


	switch ((bootcode = N64GetCIC(data))) {
		case 6101:
		case 6102:
			seed = CHECKSUM_CIC6102;
			break;
		case 6103:
			seed = CHECKSUM_CIC6103;
			break;
		case 6105:
			seed = CHECKSUM_CIC6105;
			break;
		case 6106:
			seed = CHECKSUM_CIC6106;
			break;
		default:
			return 1;
	}

	t1 = t2 = t3 = t4 = t5 = t6 = seed;

	i = CHECKSUM_START;
	while (i < (CHECKSUM_START + CHECKSUM_LENGTH)) {
		d = BYTES2LONG(data, i);
		if ((t6 + d) < t6) t4++;
		t6 += d;
		t3 ^= d;
		r = ROL(d, (d & 0x1F));
		t5 += r;
		if (t2 > d) t2 ^= r;
		else t2 ^= t6 ^ d;

		if (bootcode == 6105) t1 += BYTES2LONG(data, N64_HEADER_SIZE + 0x0710 + (i & 0xFF)) ^ d;
		else t1 += t5 ^ d;

		i += 4;
	}
	if (bootcode == 6103) {
		crc[0] = (t6 ^ t4) + t3;
		crc[1] = (t5 ^ t2) + t1;
	}
	else if (bootcode == 6106) {
		crc[0] = (t6 * t4) + t3;
		crc[1] = (t5 * t2) + t1;
	}
	else {
		crc[0] = t6 ^ t4 ^ t3;
		crc[1] = t5 ^ t2 ^ t1;
	}

	return 0;
}

function recalcCRC(buffer) {
	let cic = 0;
	let crc = [0, 0];

	//Init CRC algorithm
	gen_table();

	//Check CIC BootChip
	cic = N64GetCIC(buffer);
	console.log("BootChip: ");
	console.log((cic ? "CIC-NUS-%d\n" : "Unknown\n"), cic);

	//Calculate CRC
	if (N64CalcCRC(crc, buffer)) {
		console.log("Unable to calculate CRC\n");
	}
	else {
		console.log("CRC 1: 0x%08X  ", BYTES2LONG(buffer, N64_CRC1));
		console.log("Calculated: 0x%08X ", crc[0]);
		if (crc[0] == BYTES2LONG(buffer, N64_CRC1))
			console.log("(Good)\n");
		else{
			Write32(buffer, N64_CRC1, crc[0]);
			console.log("(Bad, fixed)\n");
		}

		console.log("CRC 2: 0x%08X  ", BYTES2LONG(buffer, N64_CRC2));
		console.log("Calculated: 0x%08X ", crc[1]);
		if (crc[1] == BYTES2LONG(buffer, N64_CRC2))
			console.log("(Good)\n");
		else{
			Write32(buffer, N64_CRC2, crc[1]);
			console.log("(Bad, fixed)\n");
		}
	}

	return 0;
}