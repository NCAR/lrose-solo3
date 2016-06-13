/*
 *	$Id$
 *
 *	Module:		 CellVector.h
 *	Original Author: Richard E. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date$
 *
 * revision history
 * ----------------
 * $Log$
 * Revision 1.1  2002/07/02 14:57:33  oye
 * Initial revision
 *
 * Revision 1.3  1993/11/18 16:54:59  oye
 * *** empty log message ***
 *
 * Revision 1.2  1993/11/18  16:53:31  oye
 * Add MAXGATES definition to be compatable with Oye's code.
 *
 * Revision 1.1  1993/10/04  22:45:45  nettletn
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCCellVector_h
#define INCCellVector_h

#define MAXCVGATES 8192

# ifndef MAXGATES
# define MAXGATES MAXCVGATES
# endif

struct cell_d {
    char cell_spacing_des[4];	/* Cell descriptor identifier: ASCII */
				/* characters "CELV" stand for cell*/
				/* vector. */
    int32_t  cell_des_len   ;	/* Comment descriptor length in bytes*/
    int32_t  number_cells   ;	/*Number of sample volumes*/
    float dist_cells[MAXCVGATES]; /*Distance from the radar to cell*/
				/* n in meters*/

}; /* End of Structure */

typedef struct cell_d cell_d;
typedef struct cell_d CELLVECTOR;

#endif
