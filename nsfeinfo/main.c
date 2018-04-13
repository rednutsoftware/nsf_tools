#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>

#define S_NSFE_MAGIC "NSFE"

#define S_CHUNK_INFO "INFO"
#define S_CHUNK_DATA "DATA"
#define S_CHUNK_NEND "NEND"
#define S_CHUNK_PLST "plst"
#define S_CHUNK_TIME "time"
#define S_CHUNK_FADE "fade"
#define S_CHUNK_TLBL "tlbl"
#define S_CHUNK_AUTH "auth"
#define S_CHUNK_BANK "BANK"

typedef struct s_chunk_info_body_s {
	uint16_t load_addr;
	uint16_t init_addr;
	uint16_t play_addr;
	uint8_t pal_ntsc_flags;
	uint8_t extra_chip_flags;
	uint8_t nr_songs;
	uint8_t init_song;
} s_chunk_info_body_t;

typedef struct s_chunk_data_body_s {} s_chunk_data_body_t;
typedef struct s_chunk_nend_body_s {} s_chunk_nend_body_t;

typedef struct s_chunk_plst_body_s {
	uint8_t song;
} s_chunk_plst_body_t;

typedef struct s_chunk_time_body_s {} s_chunk_time_body_t;
typedef struct s_chunk_fade_body_s {} s_chunk_fade_body_t;
typedef struct s_chunk_tlbl_body_s {} s_chunk_tlbl_body_t;
typedef struct s_chunk_auth_body_s {} s_chunk_auth_body_t;
typedef struct s_chunk_bank_body_s {} s_chunk_bank_body_t;

typedef struct s_chunk_s {
	/** size of chunk in bytes (does not include this value or the chunk ID) */
	uint32_t size;

	/** chunk identifier */
	uint8_t id[4];

	/** chunk data */
	union {
		s_chunk_info_body_t info; /**< INFO: MUST */
		s_chunk_data_body_t data; /**< DATA: MUST, after INFO */
		s_chunk_nend_body_t nend; /**< NEND: MUST, last */
		s_chunk_plst_body_t plst; /**< plst: optional */
		s_chunk_time_body_t time; /**< time: optional after INFO */
		s_chunk_fade_body_t fade; /**< fade: optional after INFO */
		s_chunk_tlbl_body_t tlbl; /**< tlbl: optional after INFO */
		s_chunk_auth_body_t auth; /**< auth: optional */
		s_chunk_bank_body_t bank; /**< BANK: optional */
	};
} s_chunk_t;

typedef struct s_nsfe_header_s {
	/** "NSFE" */
	uint8_t magic[4];

	/** chunk */
	s_chunk_t chunk[0];
} __attribute__((packed)) s_nsfe_header_t;

static int s_analyze_nsfe( const char *fpath );

static int s_analyze_chunk_info( const s_chunk_info_body_t *info );
static int s_analyze_chunk_plst( const s_chunk_plst_body_t *plst, uint32_t size );

int main(int argc, char *argv[])
{
	int i;
	int ret;

	for ( i = 1; i < argc; i++ )
	{

		ret = s_analyze_nsfe( argv[ i ] );

		printf( "argv-%d[%s]: ret[%d]\n", i, argv[i], ret);
	}

	return 0;
}

static int s_analyze_nsfe( const char *fpath )
{
	int ret;
	size_t fsize;
	size_t ret_sz;
	void *nsfe_data;
	FILE *fp;
	struct stat stat_buf;
	s_nsfe_header_t *nsfe_header;
	s_chunk_t *nsfe_chunk;

	fp = NULL;
	nsfe_data = NULL;

	ret = stat( fpath, &stat_buf );
	if ( ( ret != 0 ) || ( stat_buf.st_size <= 0 ) )
	{
		ret = 1;
		goto ENDING;
	}
	fsize = (size_t)(stat_buf.st_size);

	nsfe_data = malloc( fsize );
	if ( nsfe_data == NULL )
	{
		ret = 2;
		goto ENDING;
	}

	fp = fopen( fpath, "rb" );
	if ( fp == NULL )
	{
		ret = 3;
		goto ENDING;
	}

	ret_sz = fread( nsfe_data, fsize, 1, fp );
	if ( ret_sz != 1 )
	{
		ret = 4;
		goto ENDING;
	}

	nsfe_header = nsfe_data;
	if ( memcmp( nsfe_header->magic, S_NSFE_MAGIC, strlen(S_NSFE_MAGIC) ) != 0 )
	{
		/* not NSFE */
		ret = 5;
		goto ENDING;
	}

	ret = 1;
	nsfe_chunk = nsfe_header->chunk;
	while ( ( (uintptr_t)nsfe_chunk - (uintptr_t)nsfe_data ) <= (fsize - 8) )
	{
		printf("chunk[%c%c%c%c]:size[%u]\n",
			   nsfe_chunk->id[0], nsfe_chunk->id[1],
			   nsfe_chunk->id[2], nsfe_chunk->id[3], nsfe_chunk->size);
		if ( ( nsfe_chunk->size == 0 ) ||
			 ( memcmp( nsfe_chunk->id, S_CHUNK_NEND,
					   strlen( S_CHUNK_NEND ) ) == 0 ) )
		{
			ret = 0;
			break;
		}
		if (memcmp(nsfe_chunk->id, S_CHUNK_INFO, strlen(S_CHUNK_INFO))==0)
		{
			s_analyze_chunk_info( &( nsfe_chunk->info ) );
		}
		if (memcmp(nsfe_chunk->id, S_CHUNK_PLST, strlen(S_CHUNK_PLST))==0)
		{
			s_analyze_chunk_plst( &( nsfe_chunk->plst ), nsfe_chunk->size );
		}
		if (memcmp(nsfe_chunk->id, S_CHUNK_TIME, strlen(S_CHUNK_TIME))==0)
		{
		}
		if (memcmp(nsfe_chunk->id, S_CHUNK_FADE, strlen(S_CHUNK_FADE))==0)
		{
		}
		if (memcmp(nsfe_chunk->id, S_CHUNK_TLBL, strlen(S_CHUNK_TLBL))==0)
		{
		}
		if (memcmp(nsfe_chunk->id, S_CHUNK_TLBL, strlen(S_CHUNK_AUTH))==0)
		{
		}
		if (memcmp(nsfe_chunk->id, S_CHUNK_TLBL, strlen(S_CHUNK_BANK))==0)
		{
		}
		nsfe_chunk = ( s_chunk_t * )( ( uintptr_t )nsfe_chunk + nsfe_chunk->size + 8 );
	}

	ENDING:
	if ( fp != NULL )
	{
		fclose( fp );
	}
	free( nsfe_data );

	return ret;
}

static int s_analyze_chunk_info( const s_chunk_info_body_t *info )
{
#define S_OUT( x, fmt ) printf( "  " #x ": [" fmt "]\n", info->x )
	S_OUT( load_addr, "0x%04x" );
	S_OUT( init_addr, "0x%04x" );
	S_OUT( play_addr, "0x%04x" );
	S_OUT( pal_ntsc_flags, "0x02x" );
	S_OUT( extra_chip_flags, "0x02x" );
	S_OUT( nr_songs, "%u" );
	S_OUT( init_song, "%u" );
#undef S_OUT

	return 0;
}

static int s_analyze_chunk_plst( const s_chunk_plst_body_t *plst, uint32_t size )
{
	uint32_t i;

	for ( i = 0; i < size; i++ )
	{
		printf( "  playlist-%d: %u\n", i+1, plst[i].song );
	}

	return 0;
}