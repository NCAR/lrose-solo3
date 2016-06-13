/* 	$Id$	 */
# ifndef SEDS_H
# define SEDS_H


# define        SE_PROCESS_SERIES 300
#
# define           DMC_COLORTABLE 301
# define             DMC_ENDTABLE 302
# define               DMC_XCOLOR 303
# define            SO_SAVE_WINFO 304
# define         SO_START_PERUSAL 305
#
# /* commands for the for-each-ray procedure */
#
# define          SE_BB_UNFOLD_AC 306
# define          SE_BB_UNFOLD_GB 307
# define             SE_ADD_FIELD 308
# define             SE_ADD_VALUE 309
# define            SE_COPY_FIELD 310
# define               SE_DENOTCH 311
# define             SE_DESPECKLE 312
# define             SE_DEFRECKLE 313
# define     SE_DONT_USE_BOUNDARY 314
# define         SE_FORCED_UNFOLD 315
# define             SE_HISTOGRAM 316
# define              SE_MULTIPLY 317
# define          SE_RADIAL_SHEAR 318
# define       SE_ZAP_AC_SIDELOBE 319
# define         SE_REMOVE_AC_MOT 320
# define          SE_REMOVE_FIELD 321
# define      SE_REMOVE_STORM_MOT 322
# define              SE_SUBTRACT 323
# define             SE_THRESHOLD 324
# define        SE_UC_DELETE_DATA 325
# define          SE_USE_BOUNDARY 326
#
# define         SE_AND_BAD_FLAGS 327
# define        SE_AND_COND_FLAGS 328
# define      SE_ASSERT_BAD_FLAGS 329
# define     SE_ASSERT_COND_FLAGS 330
# define        SE_BSET_BAD_FLAGS 331
# define       SE_BSET_COND_FLAGS 332
# define         SE_CLR_BAD_FLAGS 333
# define        SE_CLR_COND_FLAGS 334
# define         SE_FLAG_FRECKLES 335
# define          SE_OR_BAD_FLAGS 336
# define           SE_OR_COND_BAD 337
# define         SE_OR_COND_FLAGS 338
# define       SE_RESET_BAD_FLAGS 339
# define      SE_RESET_COND_FLAGS 340
# define         SE_SET_BAD_FLAGS 341
# define        SE_SET_COND_FLAGS 342
# define         SE_XOR_BAD_FLAGS 343
# define          SE_XOR_COND_BAD 344
# define        SE_COPY_BAD_FLAGS 345
#
# /* state  and control parameter commands */
#
# define             SE_DIRECTORY 346
# define             SE_BND_SETUP 347
# define           SE_NEW_VERSION 348
# define          SE_PROCESS_DATA 349
# define       SE_SRC_VERSION_NUM 350
# define            SE_START_TIME 351
# define             SE_STOP_TIME 352
# define          SE_HISTOG_SETUP 353
#
# define               SE_EW_WIND 354
# define               SE_NS_WIND 355
# define           SE_READIN_CMDS 356
# define             SE_ONLY_ONCE 357
#
# define              SE_BB_SETUP 358
# define       SE_ESTABLISH_FIELD 359
# define        SE_REMOVE_SURFACE 360
# define             SE_CLEAR_BND 361
# define             SE_SAVE_CMDS 362

# ifndef EX_ID_NUMBERS
# define            EX_ID_NUMBERS 363
/* mode selected */
# define            EX_RADAR_DATA 364
# define        EX_BEAM_INVENTORY 365
# define             EX_EDIT_HIST 366
# define           EX_DESCRIPTORS 367
# define          EX_REFRESH_LIST 368

/* buttons clicked */
# define           EX_SCROLL_LEFT 369
# define          EX_SCROLL_RIGHT 370
# define             EX_SCROLL_UP 371
# define           EX_SCROLL_DOWN 372
# define            EX_MINUS_FOLD 373
# define             EX_PLUS_FOLD 374
# define                EX_DELETE 375
# define               EX_REPLACE 376
/* for replace use value beside "change to" */
# define                  EX_UNDO 377
# define         EX_CLEAR_CHANGES 378
# define                EX_COMMIT 379
# define         EX_CLICK_IN_LIST 380
# define         EX_CLICK_IN_DATA 381
# define  EX_CENTER_ON_LAST_CLICK 382

# define           EX_VIEW_RANGES 383
# define        EX_VIEW_CELL_NUMS 384
# define         EX_VIEW_ROT_ANGS 385
# define         EX_VIEW_RAY_NUMS 386

# define     EX_TOGGLE_ANNOTATION 387
# define       EX_CHANGE_LOCATION 388
# define         EX_PICK_LOCATION 389
# define          EX_WHICH_FIELDS 390
# define            EX_CELL_COUNT 391
# define            EX_SCROLL_INC 392
# define           EX_DISPLAY_FMT 393
# define             EX_FRAME_NUM 394
# define             EX_RAY_COUNT 395
# define           EX_DISPLAY_LOG 396

/* ui debug commands */
# define             XX_ZAP_FILES 397
# define   XX_LIST_SELECTED_FILES 398
# define         XX_PRESERVE_FILE 399
# endif
# define       TOGGLE_NEXRAD_MODE 400
# define         SE_NEXT_BOUNDARY 401
# define      SE_MANIFEST_EDITING 402

# define        SE_ABSOLUTE_VALUE 403
# define          SE_ASSIGN_VALUE 404
# define          SE_FOR_EACH_RAY 405

# define           TS_TIME_SERIES 406
# define               TS_UP_DOWN 407
# define            TS_CONTIGUOUS 408
# define              TS_RELATIVE 409
# define            TS_START_TIME 410
# define             TS_STOP_TIME 411

# define           SE_REMOVE_RING 412
# define       SE_FIX_VORTEX_VELS 413
# define           SE_FLAGGED_ADD 414
# define      SE_FLAGGED_MULTIPLY 415
# define     EX_REMOVE_AIR_MOTION 416
# define            EX_RAY_IGNORE 417
# define         EX_RAY_PLUS_FOLD 418
# define        EX_RAY_MINUS_FOLD 419
# define  SE_FORCE_INSIDE_OUTSIDE 420
# define          SE_FLAGGED_COPY 421
# define          SE_MERGE_FIELDS 422
# define   SE_CHANGE_HEADER_VALUE 423
# define         SE_RESCALE_FIELD 424
# define       SE_COMPL_BAD_FLAGS 425
# define           SE_Z_LINEARIZE 426
# define             SE_RAIN_RATE 427
# define              SE_XY_STUFF 428
# define         EX_SEG_PLUS_FOLD 429
# define        EX_SEG_MINUS_FOLD 430
# define          EX_GT_PLUS_FOLD 431
# define         EX_GT_MINUS_FOLD 432

# define       SE_MAX_PROCESS_NUM 432

/*
 * adding a ui command entails:
 *
 * setting up a definition in <seds.h>
 *
 * if this is an editor command,
 * adding a line to the command list in "se_utils.c"
 *
 * setting up the case and the call in this file "se_shared_uics.c"
 *
 * defining the actual UI command in "seds-initial.state"
 *
 * and writing the routine to service this command see "se_once_only()"
 * in "se_for_each.c"
 *
 */

#endif
