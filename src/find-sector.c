/* find-sector.c.
 This source code follows DVD title set titles and stores the sector ranges used by the title set. The sector ranges then can be queried to determine if a sector is referenced. libdvdvm from libdvdnav is used to simulate execution of pre,post and cell commands for each title in the title set.
The intended use of this is to avoid potentially bad sectors that are perhaps created by disc manufacturers attempting to prevent consumer fair use or otherwise subverting the "limited time" copyright condition of the US constitution.
 * -Don Mahurin
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <glib.h>

#include <stdint.h>

#include <netinet/in.h>

#include <dvdread/dvd_reader.h>
#include <dvdread/ifo_types.h>
#include <dvdread/ifo_read.h>
#include <dvdread/nav_read.h>
#include <dvdread/nav_print.h>

/* include files required by vm.h, but not used here */
//#include <dvdnav/remap.h>
typedef struct remap_s remap_t;
#include <dvdnav/dvd_types.h>
#include <dvdnav/decoder.h>

#include <dvdnav/vm.h>

#include <config.h>

#ifdef HAVE_DVDNAV_DVDDOMAIN_TYPE
#define FP_DOMAIN DVD_DOMAIN_FirstPlay
#define VTS_DOMAIN DVD_DOMAIN_VTSTitle
#define VMGM_DOMAIN DVD_DOMAIN_VMGM
#define VTSM_DOMAIN DVD_DOMAIN_VTSMenu
#endif

//#define DUMP_ALL
//#define TEST_MAIN

#ifdef DUMP_ALL
#define DEBUG_CELL_CHANGE 1
#define DUMP_CMD 1
#define DUMP_CELL_INFO 1
#endif

GSList *title_set_sector_ranges[100] = {0};
int max_title_set = 0;

typedef struct sector_range
{
	int start;
	int end;
} sector_range;

/* This function add a range to a sorted list of ranges. If the range overlaps or is adjacent to existing ranges, the ranges are combined. */

void add_sector_range_list( GSList **range_list, int start, int end)
{
//	fprintf(stderr, "d %d %d\n", start, end);
	if(start > end)
	{
		fprintf(stderr, "end > start\n");
		return;
	}

	sector_range *range;
	GSList *previous_node = NULL;

	GSList *node = *range_list;
	GSList *dummy;

	while(node != NULL)
	{
		// ends before this node
		if(end + 1 < ((sector_range *)(node->data))->start)
		{
			// start overlaps or adjacent with prev node
			if(previous_node != NULL && start  <= ((sector_range *)(previous_node->data))->end + 1)
			{
				((sector_range *)(previous_node->data))->end = end;
//fprintf(stderr, "start\n");
			}
			else // not adjacent range before this one
			{
				range = malloc(sizeof(sector_range));
				range->start = start;
				range->end = end;

				if(previous_node == NULL)
					*range_list = g_slist_prepend(node, range);
				else
				{
					//g_slist_insert_after(previous_node, range);
					dummy = g_slist_insert_before(previous_node, node, range);
					assert(dummy == previous_node);
				}
			}
			break;
		}
		// ends adjadently or overlapping
		else if (end  <= ((sector_range *)(node->data))->end)
		{
			// starts before this node
			if(start < ((sector_range *)(node->data))->start)
			{
				// start is adjacent or overlapping with previous node
				if(previous_node != NULL && start  <= ((sector_range *)(previous_node->data))->end + 1)
				{
					((sector_range *)(previous_node->data))->end = ((sector_range *)(node->data))->end;
					if(node->data)
					{
						free(node->data);
						node->data = NULL;
					}
					dummy = g_slist_delete_link(previous_node, node);
					assert(dummy == previous_node);

					node = previous_node;
				}
				else
					((sector_range *)(node->data))->start = start;
			}
			break;
		}
		previous_node = node;
		node = g_slist_next(node);
	}

	if(node == NULL)
	{
		if(previous_node == NULL)
		{
			range = malloc(sizeof(sector_range));
			range->start = start;
			range->end = end;
			*range_list = g_slist_prepend(NULL, range);
		}
		else
		{
			if(start  <= ((sector_range *)(previous_node->data))->end + 1)
			{
				((sector_range *)(previous_node->data))->end = end;
			}
			else
			{
				range = malloc(sizeof(sector_range));
				range->start = start;
				range->end = end;
				//g_slist_insert_after(previous_node, range);
				dummy = g_slist_insert_before(previous_node, node, range);
				assert(dummy == previous_node);
			}
		}
	}
}


void dump_sector_range_list(GSList *range_list)
{
	GSList *node = range_list;
	while(node != NULL)
	{
		fprintf(stderr, "start, end =  %u, %u\n", ((sector_range *)(node->data))->start, ((sector_range *)(node->data))->end);
		node = g_slist_next(node);
	}
}

void free_sector_range_list(GSList *range_list)
{
	GSList *node = range_list;
	while(node != NULL)
	{
		free(node->data);
		node = g_slist_delete_link(node, node);
	}
}

/* check_vm -vm -
	returns 1 if processing should continue
	returns 0 if command sequence should break
	returns -1 if program sequence should end
*/
static int check_vm(vm_t *vm, int vtsN)
{
#ifdef DEBUG_CELL_CHANGE
	fprintf(stderr, "st %d v %d, d %d c %d p %d %d %d b %d r %d %d %d\n", vm->stopped, vm->state.vtsN, vm->state.domain, vm->state.cellN, vm->state.pgcN, vm->state.pgN, (vm->state).pgc->nr_of_cells, (vm->state).blockN,
  (vm->state).rsm_vtsN,
  (vm->state).rsm_cellN,
  (vm->state).rsm_blockN
);
#endif

	if((vm->state).domain == FP_DOMAIN)
	{
#ifdef DEBUG_CELL_CHANGE
		fprintf(stderr, "fp domain\n");
#endif
		return -1;
	}

	if( (vm->state).pgc->nr_of_programs > 0 && (vm->state).pgN > (vm->state).pgc->nr_of_programs)
	{
#ifdef DEBUG_CELL_CHANGE
		fprintf(stderr, "vm last program %d %d\n", (vm->state).pgN, (vm->state).pgc->nr_of_programs);
#endif
		return -1;
	}

	if(vm->stopped)
	{
#ifdef DEBUG_CELL_CHANGE
		fprintf(stderr, "vm stopped\n");
#endif
		return -1;
	}

	if((vm->state).vtsN != vtsN)
	{
#ifdef DEBUG_CELL_CHANGE
		fprintf(stderr, "vm changed vtsN\n");
#endif
		return -1;
	}

	return 1;
}

/* for a given dvd reference and title set, create a list that contains ranges of all sectors that are referenced by the title set.
The vm engine from dvdnav is used to follow cells as indicated by cell commands.
*/
void create_titleset_range_list(dvd_reader_t *dvd, int titleset, GSList **range_list)
{
	int titleid;
	int ttn;
	int c;
	int cur_cell;
	int pgc_id;
	int pgn;
	pgc_t *cur_pgc;
	tt_srpt_t *tt_srpt;
	vts_ptt_srpt_t *vts_ptt_srpt;

	vm_t vm;

	memset(&vm, 0, sizeof(vm));
	vm.dvd = (void *)0xffffffff;
	vm_reset(&vm, NULL);
	vm.dvd = 0x0;

	ifo_handle_t *vmg_ifo = ifoOpen( dvd, 0 );

	if( !vmg_ifo )
	{
		fprintf( stderr, "Can't open VMG info.\n" );
		return;
	}

	ifo_handle_t *vts_ifo = ifoOpen( dvd, titleset );

	if( !vts_ifo )
	{
		fprintf( stderr, "Can't open VTS info.\n" );
		return;
	}

	tt_srpt = vmg_ifo->tt_srpt;
	vts_ptt_srpt = vts_ifo->vts_ptt_srpt;

	for (titleid = 0; titleid < tt_srpt->nr_of_srpts; titleid++)
	{
		ttn = tt_srpt->title[ titleid ].vts_ttn;
		if(tt_srpt->title[ titleid ].title_set_nr != titleset)
			continue;

		(vm.state).vtsN = titleset;

		int prev_pgc_id = -1;
		for(c = 0; c < tt_srpt->title[ titleid ].nr_of_ptts; c++ )
		{
			pgn = vts_ptt_srpt->title[ ttn - 1 ].ptt[ c ].pgn;
			pgc_id = vts_ptt_srpt->title[ ttn - 1 ].ptt[ c ].pgcn;
			if(pgc_id == 0)
			{
				fprintf(stderr, "pgcid zero\n");
				continue;
			}
			if(pgc_id > vts_ifo->vts_pgcit->nr_of_pgci_srp)
			{
				fprintf(stderr, "pgcid out of range\n");
				continue;
			}
			if(pgc_id == prev_pgc_id) continue;

			cur_pgc = vts_ifo->vts_pgcit->pgci_srp[ pgc_id - 1 ].pgc;

			(vm.state).pgc = cur_pgc;
			(vm.state).pgcN = pgc_id;
			(vm.state).pgN = pgn;
			(vm.state).domain = VTS_DOMAIN;
			(vm.state).cellN = 0;
			vm.vtsi = vts_ifo;
			vm.vmgi = vmg_ifo;

			int check = 0;
			int i;

			/* execute pre commands */
			if(cur_pgc->command_tbl != NULL)
			for(i = 0; i < cur_pgc->command_tbl->nr_of_pre ; i++)
			{
				vm_cmd_t *cmd = &(cur_pgc->command_tbl->pre_cmds[i]);

				vm_exec_cmd( &vm, cmd);

				/* resume vts domain on menu domain */
				if(((vm.state).domain == VMGM_DOMAIN || (vm.state).domain == VTSM_DOMAIN ) && (vm.state).rsm_vtsN != 0)
				{
#ifdef DEBUG
					fprintf(stderr, "resume domain\n");
#endif
					(vm.state).domain = VTS_DOMAIN;
					(vm.state).pgc = cur_pgc;
					(vm.state).pgcN = pgc_id;
					(vm.state).pgN = pgn;
					(vm.state).vtsN = (vm.state).rsm_vtsN;
					(vm.state).cellN = (vm.state).rsm_cellN;
					(vm.state).blockN = (vm.state).rsm_blockN;
				}

				if(0 >= (check = check_vm(&vm, titleset))) break;
			}
			if(check < 0) break;

			int next_cell;
			for( cur_cell = 0; cur_cell < cur_pgc->nr_of_cells; cur_cell = next_cell )
			{
				next_cell = cur_cell+1;

				add_sector_range_list(range_list, cur_pgc->cell_playback[ cur_cell ].first_sector, cur_pgc->cell_playback[ cur_cell ].last_sector);
#ifdef DUMP_CELL_INFO
			fprintf(stderr, "ts, ttn, titleid,  c, cell, first, last =  %u, %u, %u, %u,  %u, %u %u \n",   tt_srpt->title[ titleid ].title_set_nr, ttn, titleid, c, cur_cell+1, cur_pgc->cell_playback[ cur_cell ].first_sector, cur_pgc->cell_playback[ cur_cell ].last_sector);
#endif

				int cmd_nr = cur_pgc->cell_playback[ cur_cell ].cell_cmd_nr;
				if(cmd_nr > 0)
				{
					vm_cmd_t *cmd = &(cur_pgc->command_tbl->cell_cmds[cmd_nr - 1]);

					(vm.state).cellN = cur_cell +1;
//					fprintf(stderr, "cur %d\n", cur_cell+1);
//					fprintf(stderr, "pre exec %d\n", (vm.state).cellN);
					vm_exec_cmd( &vm, cmd);

					/* resume vts domain on menu domain */
					if(((vm.state).domain >= VMGM_DOMAIN || (vm.state).domain >= VTSM_DOMAIN ) && (vm.state).rsm_vtsN != 0)
					{
						(vm.state).domain = VTS_DOMAIN;
						(vm.state).pgc = cur_pgc;
						(vm.state).pgcN = pgc_id;
						(vm.state).pgN = pgn;
						(vm.state).vtsN = (vm.state).rsm_vtsN;
						(vm.state).cellN = (vm.state).rsm_cellN;
						(vm.state).blockN = (vm.state).rsm_blockN;
					}
					if(0 >= (check = check_vm(&vm, titleset))) break;

					if(vm.state.cellN > 0 && vm.state.cellN-1 != cur_cell)
					{
						next_cell = vm.state.cellN-1;
#ifdef DEBUG_CELL_CHANGE
						fprintf(stderr, "cell %d -> cell %d\n", cur_cell+1, vm.state.cellN);
#endif
					}
				}
				if(check < 0) break;
			}
			if(check < 0) break;

			/* execute each post command */
			if(cur_pgc->command_tbl != NULL)
			for(i = 0; i < cur_pgc->command_tbl->nr_of_post ; i++)
			{
				int check;
				vm_cmd_t *cmd = &(cur_pgc->command_tbl->post_cmds[i]);

				vm_exec_cmd( &vm, cmd);

				/* resume vts domain on menu domain */
				if(((vm.state).domain >= VMGM_DOMAIN || (vm.state).domain >= VTSM_DOMAIN ) && (vm.state).rsm_vtsN != 0)
				{
					(vm.state).domain = VTS_DOMAIN;
					(vm.state).pgc = cur_pgc;
					(vm.state).pgcN = pgc_id;
					(vm.state).pgN = pgn;
					(vm.state).vtsN = (vm.state).rsm_vtsN;
					(vm.state).cellN = (vm.state).rsm_cellN;
					(vm.state).blockN = (vm.state).rsm_blockN;
				}
				if(0 >= (check = check_vm(&vm, titleset))) break;
			}
			prev_pgc_id = pgc_id;

			if(check < 0) break;
		}
	}
	ifoClose( vts_ifo );
	ifoClose( vmg_ifo );
}

/* Starting at offset, find, return count of consecutive known sectors.
  Or return count until known sectors as a negative number.
 Or return -INT_MAX if no known sectors remain. */
int find_next_sectors(GSList *range_list, int offset)
{
	GSList *node;
	if(range_list == NULL) return -INT_MAX;

	for(node = range_list; node != NULL; node = g_slist_next(node))
	{
		if(offset < ((sector_range *)(node->data))->start) return offset - ((sector_range *)(node->data))->start;
		if(offset <= ((sector_range *)(node->data))->end) return  ((sector_range *)(node->data))->end - offset + 1;
	}
	return -INT_MAX;
}

#ifdef TEST_MAIN
int main(int argc, char *argv[])
{
	GSList *range_list = NULL;
	dvd_reader_t *dvd;
	int i;
	ifo_handle_t *vmg_ifo;
	int vmg_nr_of_title_sets;

	dvd = DVDOpen( argv[1] );

        vmg_ifo = ifoOpen( dvd, 0 );
	vmg_nr_of_title_sets = vmg_ifo->vmgi_mat->vmg_nr_of_title_sets;
	ifoClose(vmg_ifo);

        if( !vmg_ifo ) {
                fprintf( stderr, "Can't open VMG info.\n" );
                return 1;
        }

	for( i = 0; i < vmg_nr_of_title_sets; i++)
	{
		fprintf(stderr, "ts = %d\n", i+1);
		create_titleset_range_list(dvd, i+1, &range_list);
		dump_sector_range_list(range_list);
		free_sector_range_list(range_list);
		range_list = NULL;
	}
	DVDClose( dvd );
	return 0;
}
#endif


