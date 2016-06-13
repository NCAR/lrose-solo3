/* $Id$ */

#ifndef solo_list_widget_ids_hh
#define solo_list_widget_ids_hh

# define	  LEFT_MOUSE_BUTTON -1
# define	 RIGHT_MOUSE_BUTTON  1
# define	MIDDLE_MOUSE_BUTTON  2
     
/* list widget ids
 */
# define      NON_FER_CMDS_LIST  1 /* invoked from the main edit widget */
# define	  EDIT_LOG_LIST  3 /* invoked from the data examine widget */
# define      CURRENT_CMDS_LIST  4 /* invoked from the main edit widget */
# define       EXAMPLE_OPS_LIST  5 /* invoked from the main edit widget */
# define              HELP_LIST  6 /* invoked from the main edit widget */
# define           MESSAGE_LIST  7 /* invoked from the main edit widget */
# define      RELATED_CMDS_LIST  8 /* invoked from the main edit widget */

# define     SED_CMD_FILES_LIST 21 /* invoked from the SED files widget */

# define	BND_POINTS_LIST 22 /* invoked from the boundary widget */
# define	 BND_FILES_LIST 23 /* invoked from the boundary widget */

# define      FRAME_STATES_LIST 24 /* invoked from the save frame states
				    * widget*/
# define	    RADARS_LIST 25 /* invoked from either the sweeps widget
				    * or the source data widget */

# define	    SWEEPS_LIST 26 /* invoked from either the sweeps widget
				    * or the source data widget */

# define        RADAR_DATA_LIST 33 /* invoked from the data examine widget
				    * which is popped up from the MENU
				    * button */
  
/*
 * widget buttons
 */
# define            NEW_COMMAND_SEQ 21
# define       START_PASS_THRU_DATA 22
# define	 FIRST_SWEEP_BUTTON 23
# define	  LAST_SWEEP_BUTTON 24
# define       REWRITE_SAME_VERSION 25
# define         CREATE_NEW_VERSION 26
# define         USE_LAST_SWEEP_SET 27     
# define             LIST_HELP_INFO 28

# define          SAVE_FRAME_STATES 29
# define          IGNORE_SWEEP_INFO 30
# define         SAVE_SED_CMD_FILES 31
# define              SAVE_BOUNDARY 32
# define             CLEAR_BOUNDARY 33
# define             CLOSE_BOUNDARY 34
# define          DELETE_LAST_POINT 35
# define             BND_DATA_POINT 36
# define            INSIDE_BOUNDARY 37
# define           OUTSIDE_BOUNDARY 38

# define       CLEAR_ALL_BOUNDARIES 39
# define     CLEAR_CURRENT_BOUNDARY 40
# define      MOVE_TO_NEXT_BOUNDARY 42
# define          REDRAW_BOUNDARIES 43
# define    IGNORE_SOURCE_FILE_INFO 44
# define               PREV_BND_SET 45

# define               OK_TO_DELETE 50
# define         TOGGLE_QUICK_PLOTS 51
# define   ADD_TO_PRESERVATION_LIST 52
# define       LIST_ALL_SWEEP_FILES 53
# define          CLEAR_SELECT_LIST 54
# define       LIST_PRESERVED_FILES 55
# define      MARK_FOR_PRESERVATION 56
# define        REMOVE_PRESERVATION 57
# define         NOT_SELECTED_FILES 58
# define  CLICKED_IN_ALL_FILES_LIST 59
# define     CLICKED_IN_SELECT_LIST 60
# define      PETERS_QUICKIE_DELETE 61

#endif
