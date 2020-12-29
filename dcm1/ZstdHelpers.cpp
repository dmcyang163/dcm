#include "stdafx.h"
#include "ZstdHelpers.h"

#include <stdio.h>     // printf
#include <stdlib.h>    // free
#include <string.h>    // strlen, strcat, memset
#include <zstd.h>      // presumes zstd library is installed
#include "zstdcommon.h"    // Helper functions, CHECK(), and CHECK_ZSTD()

#include "TickCounter.h"

CZstdHelpers::CZstdHelpers()
{
}


CZstdHelpers::~CZstdHelpers()
{
}



void CZstdHelpers::compress_orDie(const char* fname, const char* oname)
{
	CTickCounter tc(__FUNCTION__);
	size_t fSize;
	void* const fBuff = mallocAndLoadFile_orDie(fname, &fSize);
	size_t const cBuffSize = ZSTD_compressBound(fSize);
	void* const cBuff = malloc_orDie(cBuffSize);

	/* Compress.
	* If you are doing many compressions, you may want to reuse the context.
	* See the multiple_simple_compression.c example.
	*/
	size_t const cSize = ZSTD_compress(cBuff, cBuffSize, fBuff, fSize, 3);
	CHECK_ZSTD(cSize);

	saveFile_orDie(oname, cBuff, cSize);

	/* success */
	printf("%25s : %6u -> %7u - %s \n", fname, (unsigned)fSize, (unsigned)cSize, oname);

	free(fBuff);
	free(cBuff);
}

char* CZstdHelpers::createOutFilename_orDie(const char* filename)
{
	size_t inL = strlen(filename);
	size_t outL = inL + 5;
	char* outSpace = (char*)malloc_orDie(outL);
	memset(outSpace, 0, outL);
	strcat(outSpace, filename);
	strcat(outSpace, ".zst");
	return (char*)outSpace;
}