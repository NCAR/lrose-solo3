/*
 *	$Id$
 *
 *	Module:		 Waveform.h
 *	Original Author: Richard E. K. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date$
 *
 * revision history
 * ----------------
 * $Log$
 * Revision 1.1  2002/07/02 14:57:34  oye
 * Initial revision
 *
 * Revision 1.2  1994/04/05 15:19:23  case
 * moved an ifdef RPC and changed else if to else and if on another line
 * to keep the HP compiler happy.
 *
 * Revision 1.6  1992/04/20  17:18:31  thor
 * Latest Eldora/Asterea revisions included.
 *
 * Revision 1.5  1991/10/16  15:35:16  thor
 * Changed array size and following variable to fit alignment.
 *
 * Revision 1.4  1991/10/15  17:57:29  thor
 * Fixed to meet latest version of tape spec.
 *
 * Revision 1.3  1991/10/09  15:25:28  thor
 * Removed unneeded wavecount items.
 *
 * Revision 1.2  1991/09/11  16:29:24  thor
 * Added wave_counts items.
 *
 * Revision 1.1  1991/08/30  18:39:42  thor
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCWaveformh
#define INCWaveformh

#ifdef OK_RPC

#if defined(UNIX) && defined(sun)
#include <rpc/rpc.h>
#else
#if defined(WRS)
#include "rpc/rpc.h"
#endif
#endif /* UNIX */

#endif /* OK_RPC */

struct waveform_d {
    char  waveform_des[4];	/* Identifier for the waveform */
				/* descriptor (ascii characters "WAVE"). */
    int32_t waveform_des_length;	/* Length of the waveform descriptor */
				/* in bytes. */
    char  ps_file_name[16];	/* Pulsing scheme file name.*/
    short num_chips[6];		/* Number of chips in a repeat. */
				/* sequence for each frequency. */
    char  blank_chip[256];	/* Blanking RAM sequence. */
    float repeat_seq;		/* Number of milliseconds in a repeat */
				/* sequence in ms. */
    short repeat_seq_dwel;	/* Number of repeat sequences in a */
				/* dwell time. */
    short total_pcp;		/* Total Number of PCP in a repeat sequence. */
    short chip_offset[6];	/* Number of 60 Mhz clock cycles to */
				/* wait before starting a particular */
				/* chip in 60 MHz counts. */
    short chip_width[6];	/* Number of 60 Mhz clock cycles in */
				/* each chip in 60 MHz counts. */
    float ur_pcp;		/* Number of PCP that set the */
				/* unambiguous range, after real time */
				/* unfolding. */
    float uv_pcp;		/* Number of PCP that set the */
				/* unambiguous velocity, after real */
				/* time unfolding. */
    short num_gates[6];		/* Total number of gates sampled. */
    short gate_dist1[2];	/* Distance from radar to data cell #1 */
				/* in 60 MHz counts in 0, subsequent */
				/* spacing in 1 for freq 1. */
    short gate_dist2[2];	/* Ditto for freq 2. */
    short gate_dist3[2];	/* Ditto for freq 3. */
    short gate_dist4[2];	/* Ditto for freq 4. */
    short gate_dist5[2];	/* Ditto for freq 5. */
};


typedef struct waveform_d waveform_d;
typedef struct waveform_d WAVEFORM;

#ifdef OK_RPC
#if defined(sun) || defined(WRS)
bool_t xdr_waveform_d(XDR *, WAVEFORM *);
#endif

#endif /* OK_RPC */

#endif /* INCWaveformh */

