#include <stdio.h>
#include <stdint.h>

static int s_analyze_nsf( const char *fpath );

int main(int argc, char *argv[])
{
	int i;
	int ret;

	for ( i = 1; i < argc; i++ )
	{

		ret = s_analyze_nsf( argv[i] );

		printf( "argv-%d[%s]: ret[%d]\n", i, argv[i], ret);
	}

	return 0;
}

typedef struct s_nsf_header_s {
	/** "NESM",01Ah  ; denotes an NES sound format file */
	uint8_t id[5];

	/** Version number (currently 01h) */
	uint8_t ver;

	/** Total songs   (1=1 song, 2=2 songs, etc) */
	uint8_t nr_songs;

	/** Starting song (1= 1st song, 2=2nd song, etc) */
	uint8_t init_song;

	/** (lo/hi) load address of data (8000-FFFF) */
	uint16_t load_addr;

	/** (lo/hi) init address of data (8000-FFFF) */
	uint16_t init_addr;

	/** (lo/hi) play address of data (8000-FFFF) */
	uint16_t play_addr;

	/** The name of the song, null terminated */
	uint8_t title[32];

	/** The artist, if known, null terminated */
	uint8_t artist[32];

	/** The Copyright holder, null terminated */
	uint8_t copyright[32];

	/** (lo/hi) speed, in 1/1000000th sec ticks, NTSC (see text) */
	uint16_t ntsc_tick_usec;

	/** Bankswitch Init Values (see text, and FDS section) */
	uint8_t init_bank[8];

	/** (lo/hi) speed, in 1/1000000th sec ticks, PAL (see text) */
	uint16_t pal_tick_usec;

	/** PAL/NTSC bits */
	uint8_t pal_ntsc_flags;

	/** Extra Sound Chip Support */
	uint8_t extra_chip_flags;

	/** 4 extra bytes for expansion (must be 00h) */
	uint8_t expansion[4];

	/** The music program/data follows */
	uint8_t data[ 0 ];
} __attribute__((packed)) s_nsf_header_t;

#define S_NES_ID "NESM\x1A"

static int s_analyze_nsf( const char *fpath )
{
	size_t ret_sz;
	FILE *fp;
	s_nsf_header_t nsf_header;

	fp = fopen( fpath, "rb" );
	if ( fp == NULL )
	{
		return 1;
	}

	ret_sz = fread( &nsf_header, sizeof( nsf_header ), 1, fp );
	if ( ret_sz != 1 )
	{
		fclose( fp );
		return 2;
	}

	if ( memcmp( nsf_header.id, S_NES_ID, 5 ) != 0 )
	{
		/* not NSF */
		fclose( fp );
		return 3;
	}
#define S_TMP_PR( x, fmt ) printf( #x ": [" fmt "]\n", nsf_header.x )
	S_TMP_PR( id[0], "%c" );
	S_TMP_PR( id[1], "%c" );
	S_TMP_PR( id[2], "%c" );
	S_TMP_PR( id[3], "%c" );
	S_TMP_PR( id[4], "%u" );
	S_TMP_PR( ver, "%u" );
	S_TMP_PR( nr_songs, "%u" );
	S_TMP_PR( init_song, "%u" );
	S_TMP_PR( load_addr, "0x%04x" );
	S_TMP_PR( init_addr, "0x%04x" );
	S_TMP_PR( play_addr, "0x%04x" );
	S_TMP_PR( title, "%s" );
	S_TMP_PR( artist, "%s" );
	S_TMP_PR( copyright, "%s" );
	S_TMP_PR( ntsc_tick_usec, "%u" );
	S_TMP_PR( init_bank[0], "%x" );
	S_TMP_PR( init_bank[1], "%x" );
	S_TMP_PR( init_bank[2], "%x" );
	S_TMP_PR( init_bank[3], "%x" );
	S_TMP_PR( init_bank[4], "%x" );
	S_TMP_PR( init_bank[5], "%x" );
	S_TMP_PR( init_bank[6], "%x" );
	S_TMP_PR( init_bank[7], "%x" );
	S_TMP_PR( pal_tick_usec, "%u" );
	S_TMP_PR( pal_ntsc_flags, "0x%02x" );
	S_TMP_PR( extra_chip_flags, "0x%02x" );
	S_TMP_PR( expansion[0], "%u" );
	S_TMP_PR( expansion[1], "%u" );
	S_TMP_PR( expansion[2], "%u" );
	S_TMP_PR( expansion[3], "%u" );
#undef S_TMP_PR

	fclose( fp );
	return 0;
}
