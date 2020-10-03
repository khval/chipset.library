
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define __USE_INLINE__

#include <proto/exec.h>
#include <proto/dos.h>

uint32_t	ahi_mode_id = 0;
uint32_t	ahi_frequency = 0;

struct IFF_head 
{
	char id[4];
	uint32_t size;
};

struct __attribute__((__packed__)) IFF_PRHD_group
{
	uint32_t	items;
	uint32_t	stuff1;
	uint16_t	stuff2;
};

struct  IFF_PRHD_group prhd;

struct __attribute__((__packed__)) IFF_AHIG
{
	uint16_t	data[13];
};

struct IFF_AHIG ahig;

struct __attribute__((__packed__)) IFF_AHIU
{
	uint32_t	data[9];	
};

struct IFF_AHIU ahiu;

struct IFF_PRHD_group prhd;

struct item
{
	const char *id;
	uint32_t size;
	void *ptr;
	void (*get) (struct item *);
};

void get_pref (struct item *i);
void get_prhd (struct item *i);
void get_ahig (struct item *i);
void get_ahiu (struct item *i);

struct item items[] = 
{
	{"PREF",  0, NULL, NULL },
	{"PRHD", sizeof(struct IFF_PRHD_group), &prhd, NULL },
	{"AHIG", sizeof(struct IFF_AHIG), &ahig, get_ahig },
	{"AHIU", sizeof(struct IFF_AHIU), &ahiu, get_ahiu },
	{NULL, 0, NULL }
};


int ahig_count = 0;
int ahiu_count = 0;

void get_ahig (struct item *item)
{
}

void get_ahiu (struct item *item)
{
	uint32_t *ptr = (uint32_t *) item -> ptr;

	if (ahiu_count == 4)		// music unit.
	{
		ahi_mode_id = ptr[2];
		ahi_frequency = ptr[3];
	}

	ahiu_count ++;
}

void read_ahi_prefs()
{
	struct IFF_head head;
	char id[4];
	struct item *i;
	bool found;

	BPTR fd;
	fd = Open( "env:sys/ahi.prefs", MODE_OLDFILE );
	if (fd == 0)  return ;

	Read( fd, &head , sizeof(head));

	for (;Read( fd, id , 4);)
	{
		found = false;
		for (i = items; i -> id != NULL ; i++ )
		{
			if (strncmp(i -> id, id, 4) != 0 ) continue;
			if (i -> size) 
			{ 
				Read( fd, i -> ptr , i -> size );
				if (i -> get) i -> get(i);
			}
			found = true;
			break;
		}

		if (!found) break;
	}

	Close(fd);
}

