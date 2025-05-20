//--------------------------------------------------------------------------------------------------
// SAP Source Code Decompressor
//--------------------------------------------------------------------------------------------------
// Written by Daniel Berlin <mail@daniel-berlin.de> - https://www.daniel-berlin.de/
// Inspired by code from Dennis Yurichev <dennis@conus.info> - http://www.yurichev.com/
// This program uses portions of the MaxDB 7.6 source code, which are licensed under
// the GNU General Public License v2 (see file headers in "lib/" folder).
// Comments, suggestions and questions are welcome!
//--------------------------------------------------------------------------------------------------
// The source code stored in REPOSRC-DATA is basically compressed using the LZH algorithm
// (Lempel-Ziv-Huffman), for details see "lib/vpa108csulzh.cpp".
// Additionally, I had to add some tweaks to get it to work; see comments below.
// The decompressed source code has a fixed line length of 255 characters (blank-padded).
//--------------------------------------------------------------------------------------------------
// To extract the compressed source code from SAP, use the included report "ZS_REPOSRC_DOWNLOAD".
//--------------------------------------------------------------------------------------------------
// Changes:
//   2012-06-23 - 1.0.0 - Initial version
//   2012-06-24 - 1.0.1 - Revert modifications to MaxDB library
//   2015-01-05 - 1.1.0 - Major UTF-16 enhancement by Uwe Lindemann
//   2015-07-24 - 1.1.1 - Add option for non-Unicode SAP systems by Bastian Preißler
//   2017-02-09 - 1.1.2 - Tune output buffer factor; various minor adjustments
//--------------------------------------------------------------------------------------------------

#define VERSION "1.1.2"

// Silence MS Visual C++ ("... consider using fopen_s instead ...")
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/hpa101saptype.h"
#include "lib/hpa106cslzc.h"
#include "lib/hpa107cslzh.h"
#include "lib/hpa104CsObject.h"
#include "lib/hpa105CsObjInt.h"

#ifdef _WIN32
#define DLL_EXPORT extern "C" __declspec(dllexport)
#else
#define DLL_EXPORT extern "C"
#endif

DLL_EXPORT int decompress_sap_source(const char* input_path, const char* output_path) {
	int ret;
	FILE *fin, *fout;
	long fin_size;
	SAP_BYTE *bin, *bout;
	short factor = 5;
	class CsObjectInt o;
	SAP_INT byte_read, byte_decomp;
	long i;
	long nextpos;
	int funicode = 0;
	int fnuc = 0;
	unsigned char cbom1 = (unsigned char) 0xFE;
	unsigned char cbom2 = (unsigned char) 0xFF;
	
	// Open input file
	fin = fopen(input_path, "rb");
	if(fin == NULL) {
		return 1;
	}
	ret = fseek(fin, 0, SEEK_END);
	if (ret != 0) { fclose(fin); return 1; }
	fin_size = ftell(fin);
	if (fin_size == -1L) { fclose(fin); return 1; }
	ret = fseek(fin, 0, SEEK_SET);
	if (ret != 0) { fclose(fin); return 1; }
	bin = (SAP_BYTE*) malloc(fin_size);
	ret = fread(bin, 1, fin_size, fin);
	fclose(fin);
	if (ret != fin_size) { free(bin); return 1; }
	fout = fopen(output_path, "wb");
	if(fout == NULL) {
		free(bin);
		return 2;
	}
	ret = o.CsGetAlgorithm(bin + 1);
	for(;;) {
		bout = (SAP_BYTE*) malloc(fin_size * factor);
		ret = o.CsDecompr(
			bin + 1,
			fin_size - 1,
			bout,
			fin_size * factor,
			CS_INIT_DECOMPRESS,
			&byte_read,
			&byte_decomp
		);
		if(ret == CS_END_OUTBUFFER || ret == CS_E_OUT_BUFFER_LEN) {
			factor += 5;
			free(bout);
			continue;
		}
		free(bin);
		switch(ret) {
			case CS_END_OF_STREAM      :
			case CS_END_INBUFFER       : break;
			case CS_E_IN_BUFFER_LEN    : return  3;
			case CS_E_MAXBITS_TOO_BIG  : return  4;
			case CS_E_FILENOTCOMPRESSED: return  6;
			case CS_E_IN_EQU_OUT       : return  7;
			case CS_E_INVALID_ADDR     : return  8;
			case CS_E_FATAL            : return  9;
			default                    : return 10;
		}
		break;
	}
	// No Unicode/non-Unicode/BOM/linefeed handling for DLL version (can be added if needed)
	for(i = 1; i < byte_decomp; i++) {
		int ret2 = fwrite(bout + i, 1, 1, fout);
		if(ret2 != 1) {
			fclose(fout);
			free(bout);
			return 11;
		}
	}
	fwrite("\n", 1, 1, fout);
	fclose(fout);
	free(bout);
	return 0;
}

int main(int argc, char *argv[]) {
	if(argc < 3) {
		printf("Usage:\n  %s <infile> <outfile>\n", argv[0]);
		return 0;
	}
	int result = decompress_sap_source(argv[1], argv[2]);
	if(result == 0) {
		printf("Decompression successful.\n");
	} else {
		printf("Decompression failed with code %d.\n", result);
	}
	return result;
}
