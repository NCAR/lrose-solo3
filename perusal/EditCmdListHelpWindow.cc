#include <iostream>

#include <EditCmdHelpWindow.hh>
#include <EditWindow.hh>

#include "EditCmdListHelpWindow.hh"

/**********************************************************************
 * Constructor
 */

EditCmdListHelpWindow::EditCmdListHelpWindow(EditWindow *parent,
					     const Pango::FontDescription &default_font) :
  Gtk::Window(),
  _parent(parent),
  _defaultFont(default_font),
  _cmdListView(1)
{
  // Initialize the command list

  _initCommandList();
  
  // Create the fonts needed for the widgets

  Pango::FontDescription smaller_italic_font = _defaultFont;
  smaller_italic_font.set_size(_defaultFont.get_size() - (2 * 1024));
  smaller_italic_font.set_style(Pango::STYLE_ITALIC);
  
  // Construct the window title

  char title_string[1024];
  sprintf(title_string, "Frame %d  Edit Commands Help List",
	  _parent->getFrameIndex() + 1);
  
  set_title(title_string);
  set_border_width(2);
  
  // Set up the widgets

  _vbox.set_homogeneous(false);
  _vbox.set_spacing(2);
  add(_vbox);
  
  _cancelButton.set_label("Cancel");
  _cancelButton.modify_font(_defaultFont);
  _vbox.pack_start(_cancelButton, false, false, 0);
  _cancelButton.signal_clicked().connect(sigc::mem_fun(*this,
						       &EditCmdListHelpWindow::hide));
 
  _clickLabel.set_text("(Double-click to select)");
  _clickLabel.set_justify(Gtk::JUSTIFY_CENTER);
  _clickLabel.modify_font(smaller_italic_font);
  _vbox.pack_start(_clickLabel, false, false, 0);
  
  _cmdListView.set_headers_visible(false);
  _cmdListView.modify_font(_defaultFont);

  // Set the padding for the list.  Note that the gtkmm-2.4 installed with
  // the Centos OS has a different API for the CellRenderer class so we can't
  // do this on that OS.
#ifndef OS_IS_CENTOS
  Gtk::CellRenderer *renderer = _cmdListView.get_column_cell_renderer(0);
  renderer->set_padding(0, 0);
#endif

  _setCommandList();
  _cmdListView.signal_row_activated().connect(sigc::mem_fun(*this,
							    &EditCmdListHelpWindow::_selectCommand));

  _cmdListViewWindow.set_policy(Gtk::POLICY_AUTOMATIC,
				Gtk::POLICY_AUTOMATIC);
  _cmdListViewWindow.add(_cmdListView);
  _vbox.pack_start(_cmdListViewWindow, true, true, 0);

  set_size_request(200, 900);
}


/**********************************************************************
 * Destructor
 */

EditCmdListHelpWindow::~EditCmdListHelpWindow()
{
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _initCommandList()
 */

void EditCmdListHelpWindow::_initCommandList()
{
  _commandList["BB-ac-unfolding"] =
    std::string("!  Help file for the \"BB-ac-unfolding\" command which has the form:\n") +
    "\n" +
    "    BB-ac-unfolding in <field-name>\n" +
    "\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "\n" +
    "!  This operation does a Bargen-Brown unfold of a velocity field\n" +
    "!  based on the difference between a given point and a running\n" +
    "!  average of a certain number of gates being greater than the\n" +
    "!  Nyquist Velocity. This command is meant for aircraft data.\n" +
    "\n" +
    "!  Other \"one-time\" commands that affect BB-ac-unfolding are:\n" +
    "\n" +
    "    BB-gates-averaged is <integer> gates\n" +
    "!  The number of gates in the running average\n" +
    "    BB-max-neg-folds is <integer>\n" +
    "    BB-max-pos-folds is <integer>\n" +
    "!  Stops the number of unfolds from exceeding this value\n" +
    "    BB-use-first-good-gate\n" +
    "!  This is the default!\n" +
    "!  Causes the running average to initialize on the first good gate\n" +
    "!  encountered in the first ray in the sweep. Subsequent rays\n" +
    "!  initialize on the first good gate in the previous ray.\n" +
    "    BB-use-ac-wind\n" +
    "!  Is for aircraft data and means to use the wind information from the\n" +
    "!  ac data present.\n" +
    "    BB-use-local-wind\n" +
    "!  Users can define the local wind in m/s. with\n" +
    "    ew-wind <float>\n" +
    "    ns-wind <float>\n";

  _commandList["absolute-value"] =
    std::string("!  Help file for the \"absolute-value\" command which has the form:\n") +
    "\n" +
    "    absolute-value of <field>\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  Computes and assigns the absolute value to all gates in each ray\n" +
    "!    or just gates inside or outside boundaries.\n" +
    "!  e.g.\n" +
    "    absolute-value of AP\n";

  _commandList["and-bad-flags"] =
    std::string("!  Help file for the \"and-bad-flags\" command which has the form:\n") +
    "\n" +
    "    and-bad-flags when <field> <below>|<above>|<between> <real> [and <real>]\n" +
    "\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "\n" +
    "!  ANDs the result of the condition with the bad flags mask.\n" +
    "!  If the test field is flagged bad, the flag in the bad flags mask\n" +
    "!  is turned off otherwise the condition is ANDed with the\n" +
    "!  corresponding bad flag.\n" +
    "\n" +
    "!  e.g.\n" +
    "    and-bad-flags when SW above 4.4\n" +
    "    and-bad-flags when VE between -1. and 1.\n" +
    "\n" +
    "!  You might also use the following sequence as one way to\n" +
    "!  remove clutter from both the VE and DZ fields:\n" +
    "\n" +
    "    set-bad-flags when VE between -1. and 1.\n" +
    "    and-bad-flags when DZ above 35.\n" +
    "    assert-bad-flags in VE\n" +
    "    assert-bad-flags in DZ\n" +
    "!  Turning off the flag when the test field contains a delete flag\n" +
    "!  for AND only guarentees that the absence of a reflectivity will\n" +
    "!  not cause a velocity value between -1. and 1. to be deleted in\n" +
    "!  the above example.  Click on the next line for more information on the\n" +
    "    bad-flags-mask\n";

  _commandList["assert-bad-flags"] =
    std::string("!  Help file for the \"assert-bad-flags\" command which has the form:\n") +
    "\n" +
    "    assert-bad-flags in <field>\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  Set delete flags in the field indicated based on the bad flags mask\n" +
    "!  e.g.\n" +
    "    assert-bad-flags in VT\n";

  _commandList["assign-value"] =
    std::string("!  Help file for the \"assign-value\" command which has the form:\n") +
    "\n" +
    "    assign-value <real> to <field>\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  Assigns the <real> value to all gates in each ray or just gates\n" +
    "    inside or outside boundaries.\n" +
    "!  e.g.\n" +
    "    assign-value 2 to AP\n";

  _commandList["bad-flags-mask"] =
    std::string("!  The bad flags mask which can also be thought of a just a flags mask\n") +
    "!  is an array of flags that can be manipulated with\n" +
    "!  logical operations on fields and then asserted in various fields.\n" +
    "!  For example you might use the following sequence as one way to\n" +
    "!  remove clutter from both the VE and DZ fields:\n" +
    "\n" +
    "    clear-bad-flags\n" +
    "    set-bad-flags when VE between -1. and 1.\n" +
    "    and-bad-flags when DZ above 35.\n" +
    "    assert-bad-flags in VE\n" +
    "    assert-bad-flags in DZ\n"
    "\n" +
    "!  \"set-bad-flags\" automatically clears the bad flags mask\n" +
    "!  but other operations that set flags DO NOT automatically clear the\n" +
    "!  bad flags mask so you will want to insert a \"clear-bad-flags\"\n" +
    "!  command at least once in each ray.\n" +
    "\n" +
    "!  Bad flagged data do not cause the flag to be set, only the\n" +
    "!  condition being true causes the bad flag to be set.\n" +
    "!  You can also create a flag mask based on the bad data flags\n" +
    "!  in a field with the \"copy-bad-flags\" command which also effectively\n" +
    "!  clears the flags mask.\n" +
    "\n" +
    "!  The AND operation also has a special feature that turns off\n" +
    "!  a flag when the test field contains a delete flag. This\n" +
    "!  guarentees that the absence of a reflectivity in this case will\n" +
    "!  not cause a velocity value between -1. and 1. to be deleted.\n" +
    "\n" +
    "!  Other flag operations:\n" +
    "    or-bad-flags\n" +
    "    xor-bad-flags\n" +
    "    copy-bad-flags\n" +
    "    flagged-add\n" +
    "    flagged-multiply\n" +
    "    flagged-copy\n" +
    "    flag-freckles\n" +
    "    flag-glitches\n";

  _commandList["clear-bad-flags"] =
    std::string("!  Help file for the \"clear-bad-flags\" command which has the form:\n") +
    "\n" +
    "    clear-bad-flags\n" +
    "!  Clears all flags in the bad flags mask.\n" +
    "!  At the very least you should clear the bad flags at the beginning of\n" +
    "!    a set of operations for each ray so that flags set for the\n" +
    "!    previous ray do not affect operation for the current ray. This is\n" +
    "!    not done automatically because it would be unnecessary overhead\n" +
    "!    if you are not using flag operations.\n";

  _commandList["copy"] =
    std::string("!  Help file for the \"copy\" command which has the form:\n") +
    "\n" +
    "    copy from <field> to <field>\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  Copies gates from one field to another USING THE BOUNDARY.\n" +
    "!  e.g.\n" +
    "    copy from VE to VG\n" +
    "\n" +
    "!  See\n" +
    "    duplicate VE in VG\n" +
    "\n" +
    "!  for an alternative that ignores the boundary\n";

  _commandList["copy-bad-flags"] =
    std::string("!  Help file for the \"copy-bad-flags\" command which has the form:\n") +
    "\n" +
    "    copy-bad-flags from <field>\n"
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  Sets or clears flags in the bad flags mask based on delete flags\n" +
    "!  in the <field>\n" +
    "!  e.g.\n" +
    "    copy-bad-flags from DZ\n";

  _commandList["despeckle"] =
    std::string("!  Help file for the \"despeckle\" command which has the form:\n") +
    "\n" +
    "    despeckle in <field-name>\n" +
    "\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  Other \"one-time\" commands that affect defreckling are:\n" +
    "    a-speckle is <integer> gates\n" +
    "!  defines a speckle and <integer> or less gates surrounded\n" +
    "!  by delete flags.\n" +
    "!  This operation removes speckles\n";

  _commandList["do-defreckling"] =
    std::string("!  Help file for the \"do-defreckling\" command which has the form:\n") +
    "\n" +
    "    do-defreckling in <field>\n" +
    "\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  Other \"one-time\" commands that affect defreckling are:\n" +
    "\n" +
    "    freckle-average is <integer> gates\n" +
    "!  number of gates in the running average\n" +
    "    freckle-threshold is <real>\n" +
    "!  maximum acceptable shear for determining a freckle\n" +
    "\n" +
    "!  A point is deleted if the difference between it and the running\n" +
    "!  average is greater than the max shear. We start out averaging\n" +
    "!  n gates ahead but once we have encountered n non-shear points\n" +
    "!  we go to a trailing average. See also\n" +
    "    flag-freckles\n";

  _commandList["do-histogram"] =
    std::string("!  Help file for the \"do-histogram\" command which has the form:\n") +
    "\n" +
    "    do-histogram\n" +
    "\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  You can only do one histogram on one field on each pass through the data.\n" +
    "\n" +
    "!  Other \"one-time\" commands that affect histogram calculation are:\n" +
    "\n" +
    "    regular-histogram-parameters low <real> high <real> increment <real>\n" +
    "!  \"low\", \"high\", and \"increment\" are optional, but order of the arguments\n" +
    "!  is critical!\n" +
    "\n" +
    "    area-histogram on <field>\n" +
    "!  does areas instead of counts\n" +
    "    count-histogram on <field>\n" +
    "!  the two commands above would also cause a reset to accept a new\n" +
    "!  set of irregular bins.\n" +
    "\n" +
    "    irregular-histogram-bin from <real> to <real>\n" +
    "\n" +
    "!  one line for each irregular bin. Irregular bins can overlap or underlap.\n" +
    "!  The algorithm imposes a resolution of 100 bins between the min and max\n" +
    "!  values and fills an irregular bin based on the set of regular bins\n" +
    "!  that fall inside the region defined by the irregular bin.\n" +
    "\n" +
    "    new-histogram-file\n" +
    "!  starts a new file of histogram output information\n" +
    "    append-histogram-to-file\n" +
    "!  causes histogram output to be appended to the current histogram file\n" +
    "    dont-append-histogram-to-file\n" +
    "\n" +
    "    histogram-comment \"appends_this_comment_to_the_file_name\"\n" +
    "!  use double quotes for the comment unless you use underscores\n" +
    "\n" +
    "    histogram-directory /your_path_to/histograms\n" +
    "    histogram-flush   ! makes the output file contents visible\n" +
    "\n" +
    "!  e.g. set of commands to do a histogram\n" +
    "\n" +
    "!  commands that need to be executed only once\n" +
    "\n" +
    "    count-histogram on VU\n" +
    "    regular-histogram-parameters low -12. high 12. increment 1.\n" +
    "\n" +
    "!  operations executed for-each-ray\n" +
    "\n" +
    "    do-histogram\n";

  _commandList["duplicate"] =
    std::string("!  Help file for the \"duplicate\" command which has the form:\n") +
    "\n" +
    "    duplicate <field> in <field>\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  Makes a duplicate of one field in another and does NOT use the\n" +
    "!  boundary.\n" +
    "!  e.g.\n" +
    "    duplicate VE in VG\n" +
    "!\n" +
    "    duplicate VE in VG\n" +
    "    ignore VE\n" +
    "!  Is effectively a rename of field VE.\n";

  _commandList["establish-and-reset"] =
    std::string("!  Help file for the \"establish-and-reset\" command which has the form:\n") +
    "\n" +
    "    establish-and-reset from <field> to <field>\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  Copies header information from one field to another and\n" +
    "!  initializes the field with bad data flags.\n" +
    "!  e.g.\n" +
    "    establish-and-reset from VE to RS\n";

  _commandList["fix-vortex-velocities"] =
    std::string("!  Help file for the \"fix-vortex-velocities\" command which has the form:\n") +
    "\n" +
    "    fix-vortex-velocities in <field>\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  Works only if the fields VS and VL are present. Operation replaces the\n" +
    "!    original vield VR by recalculating the final dual prt velocity\n" +
    "!    and eliminating some noisy gates.\n" +
    "\n" +
    "!  e.g.\n" +
    "    fix-vortex-velocities in VG\n";

  _commandList["flag-freckles"] =
    std::string("!  Help file for the \"flag-freckles\" command which has the form:\n") +
    "\n" +
    "    flag-freckles in <field>\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  Other \"one-time\" commands that affect defreckling are:\n" +
    "\n" +
    "    freckle-average is <integer> gates\n" +
    "!  number of gates in the running average\n" +
    "    freckle_threshold is <real>\n" +
    "!  maximum acceptable shear for determining a freckle\n" +
    "\n" +
    "!  Set a flag if the difference between point and the running average\n" +
    "!  is greater than the max shear. We start out averaging n gates ahead\n" +
    "!  but once we have encountered enought non-shear points we go to a\n" +
    "!  trailing average\n" +
    "!  e.g.\n" +
    "    clear-bad-flags\n" +
    "    flag-freckles in DZ\n" +
    "    assert-bad-flags in DZ\n" +
    "    assert-bad-flags in VT\n";

  _commandList["flag-glitches"] =
    std::string("!  Help file for the \"flag-glitches\" command which has the form:\n") +
    "\n" +
    "    flag-glitches in <field>\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  Other \"one-time\" commands that affect defreckling are:\n" +
    "\n" +
    "    deglitch-radius is <integer> gates\n" +
    "!  defines the number of gates in the running average (navg = radius * 2 +1)\n" +
    "    deglitch-threshold is <real>\n" +
    "!  maximum acceptable shear for determining a glitch\n" +
    "    deglitch-min-gates is <integer>\n" +
    "!  minimum number of good gates before any glitch test is attempted    \n" +
    "\n" +
    "!  Set a flag if the difference between a valid center point\n" +
    "!  and the running average without the center point is greater than\n" +
    "!  the max shear.\n" +
    "!  e.g.\n" +
    "    deglitch-radius is 3 gates\n" +
    "!  running average is for 7 gates\n" +
    "    deglitch-threshold is 15\n" +
    "    deglitch-min-gates is 6\n" +
    "!  need only 6 valid points to conduct a glitch test\n" +
    "!  for-each-ray\n" +
    "    clear-bad-flags\n" +
    "    flag-glitches in DZ\n" +
    "    assert-bad-flags in DZ\n" +
    "    assert-bad-flags in VT\n";
  
  _commandList["flagged-add"] =
    std::string("!  Help file for the \"flagged-add\" command which has the form:\n") +
    "\n" +
    "    flagged-add of <real> in <field>\n" +
    "\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  This operation will obey boundaries and will add the appropriate value\n" +
    "!    to the gate value if the corresponding \"bad-flag\" is set.\n" +
    "\n" +
    "!  e.g.\n" +
    "    clear-bad-flags\n" +
    "    set-bad-flags when VQ below 0\n" +
    "    flagged-add of 50.72 in VQ\n";
  
  _commandList["flagged-multiply"] =
    std::string("!  Help file for the \"flagged-multiply\" command which has the form:\n") +
    "\n" +
    "    flagged-multiply of <real> in <field>\n" +
    "\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  This operation will obey boundaries and will multiply the gate value\n" +
    "!    by the appropriate value if the corresponding \"bad-flag\" is set.\n" +
    "\n" +
    "!  e.g.\n" +
    "    clear-bad-flags\n" +
    "    set-bad-flags when VQ below 0\n" +
    "    flagged-multiply by -1. in VQ\n";

  _commandList["forced-unfolding"] =
    std::string("!  Help file for the \"forced-unfolding\" command which has the form:\n") +
    "\n" +
    "    forced-unfolding in <field> around <real>\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "\n" +
    "!  Forces all data points to fall within plus or minus the Nyquist\n" +
    "!  Velocity of the <real> value.\n";

  _commandList["header-value"] =
    std::string("!  Help file for the \"header-value\" command which has the form:\n") +
    "\n" +
    "    header-value <name> is <real>\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "\n" +
    "!  Changes the value of the named variable in one of the headers\n" +
    "\n" +
    "!  Exhaustive list of examples:\n" +
    "\n" +
    "    header-value nyquist-velocity is 23.45 meters-per-second\n" +
    "    header-value fixed-angle is 12.3456 degrees\n" +
    "    header-value range-to-first-cell is 123.456 meters\n" +
    "    header-value cell-spacing is 156.7 meters\n" +
    "    header-value rotation-angle is 180.0 degrees\n" +
    "    header-value tilt-angle 0.0 degrees\n" +
    "    header-value elevation-angle 90.0 degrees\n" +
    "    header-value latitude 39.7867 degrees\n" +
    "    header-value longitude -104.5458 degrees\n" +
    "    header-value altitude 1.7102 km.\n" +
    "    header-value agl-altitude 1.7102 km.\n" +
    "    header-value msl-into-agl-corr 0.0 km.\n" +
    "    header-value corr-elevation by 5. degrees\n" +
    "    header-value corr-azimuth by 5. degrees\n" +
    "    header-value corr-rot-angle by 5. degrees\n" +
    "    header-value ipp1 is 1.1 milliseconds\n" +
    "    header-value ipp2 is 1.0 milliseconds\n" +
    "\n" +
    "!  Tokens following the <real> are ignored but can be included to\n" +
    "!  clarify the units of the header value\n" +
    "!  \"rotation-angle\" and \"tilt-angle\" modify the platform header for\n" +
    "!  every ray and modifying the \"rotation-angle\" is only useful for\n" +
    "!  non-scanning instruments on moving platforms.\n" +
    "!  \"elevation-angle\" modifies the ray information header\n" +
    "!  for each ray.\n";

  _commandList["ignore-field"] =
    std::string("!  Help file for the \"ignore-field\" command which has the form:\n") +
    "\n" +
    "    ignore-field <field>\n" +
    "\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  Causes the field to be ignored when writing the ray back out to the disk.\n" +
    "\n" +
    "!  e.g.\n" +
    "    ignore-field VX\n" +
    "!\n" +
    "!  Which is effectively a delete or removal of this field.\n";

  _commandList["merge-field"] =
    std::string("!  Help file for the \"merge-field\" command which has the form:\n") +
    "\n" +
    "    merge-field <field> with <field> put-in <field>\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "\n" +
    "!  If the first field value in not bad it is placed in the third field\n" +
    "!  otherwise the second field value is placed in the third field\n" +
    "!  e.g.\n" +
    "    merge-field VX with VE put-in VY\n";

  _commandList["one-time"] =
    std::string("Replace angle brackets and argument types with appropriate arguments.\n") +
    "\n" +
    "    a-speckle is <integer> gates\n" +
    "\n" +
    "!  used by \"despeckle\". If the number of gates specified is n, then we\n" +
    "!  will remove any set of n or less gates that are preceeded and\n" +
    "!  followed by delete flags.\n" +
    "\n" +
    "    first-good-gate is <integer>\n" +
    "\n" +
    "!  for an operation on a ray, this parameter setting indicates\n" +
    "!  where to begin the operation. If the first good gate is n, then\n" +
    "!  we will ignore the first n-1 gates. Thresholding operations \n" +
    "!  will set the delete flag in these gates.\n" +
    "\n" +
    "    freckle-average is <integer> gates\n" +
    "\n" +
    "!  used by \"do-defreckling\" to indicate the number of gates to average\n" +
    "\n" +
    "    freckle-threshold is <real>\n" +
    "\n" +
    "!  used by \"do-defreckling\" to indicate the maximum acceptable shear\n" +
    "!  between a gate and the running average\n" +
    "\n" +
    "    offset-for-radial-shear is <integer> gates\n" +
    "!  used by \"radial-shear\" command to create a field of the\n" +
    "!  differences between the current gate and the gate n gates ahead\n" +
    "\n" +
    "    ew-wind is <real>\n" +
    "!  enters the east-west wind where the value is positive if the\n" +
    "!  wind is from the west. Dimensions are m/s.\n" +
    "\n" +
    "    ns-wind is <real>\n" +
    "!  enters the north-south wind where the value is positive if the\n" +
    "!  wind is from the south. Dimensions are m/s.\n" +
    "\n" +
    "    vert-wind is <real>\n" +
    "!  enters the vertical wind where the value is positive if there\n" +
    "!  is an updraft. Dimensions are m/s.\n";

  _commandList["or-bad-flags"] =
    std::string("!  Help file for the \"or-bad-flags\" command which has the form:\n") +
    "\n" +
    "    or-bad-flags when <field> <below>|<above>|<between> <real> [and <real>]\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  ORs the result of the condition with the bad flag. The bad flags mask\n" +
    "!  is not modified if the test field contains a bad flag.\n" +
    "!  e.g.\n" +
    "    or-bad-flags when DZ below 0.0\n" +
    "    or-bad-flags when VE between -1. and 1.\n" +
    "!  Click on the next line for more information on the\n" +
    "    bad-flags-mask\n";

  _commandList["radial-shear"] =
    std::string("!  Help file for the \"radial-shear\" command which has the form:\n") +
    "\n" +
    "    radial-shear in <field> put-in <field>\n" +
    "\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "\n" +
    "!  If the \"put-in\" field does not exist it will be added using\n" +
    "!  the header information from the source field.\n" +
    "\n" +
    "!  Other \"one-time\" commands that affect radial-shear are:\n" +
    "\n" +
    "    offset-for-radial-shear is <integer> gates\n" +
    "\n" +
    "!  if n is the number of gates, then the shear value is computed by\n" +
    "!  shear = val[i+n] - val[i];\n";

  _commandList["remove-aircraft-motion"] =
    std::string("!  Help file for the \"remove-aircraft-motion\" command which has the form:\n") +
    "\n" +
    "    remove-aircraft-motion in <field>\n" +
    "\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  This operation subtract the component of velocity contributed by\n" +
    "!  the aircraft's motion based on the following calculation:\n" +
    "\n" +
    "!     vert =  asib->vert_velocity != -999 ? asib->vert_velocity : 0;\n" +
    "!     d = sqrt((double)(SQ(asib->ew_velocity) + SQ(asib->ns_velocity)));\n" +
    "!     d += dds->cfac->ew_gndspd_corr; /* klooge to correct ground speed */\n" +
    "!     ac_vel = d*sin(ra->tilt) + vert*sin(ra->elevation);\n" +
    "\n" +
    "!  These are the calculated track relative tilt and elevations.\n" +
    "!  e.g.\n" +
    "    remove-aircraft-motion in VG\n";

  _commandList["remove-ring"] =
    std::string("!  Help file for the \"remove-ring\" command which has the form:\n") +
    "\n" +
    "    remove-ring in <field> from <real> to <real> km.\n" +
    "\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  Inserts a bad data flag between the two indicated ranges.\n" +
    "\n" +
    "!  e.g.\n" +
    "    remove-ring in VQ from 75.15 to 75.8 km.\n";

  _commandList["remove-storm-motion"] =
    std::string("!  Help file for the \"remove-storm-motion\" command which has the form:\n") +
    "\n" +
    "    remove-storm-motion in <field> of <real> deg <real> mps\n" +
    "\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  This operation subtracts the component of velocity contributed by\n" +
    "!  the storm's motion where the angle in degrees is the direction\n" +
    "!  from which the wind is blowing. \n" +
    "\n" +
    "!  e.g.\n" +
    "    remove-storm-motion in VT of 234 deg 11 mps\n";

  _commandList["remove-surface"] =
    std::string("!  Help file for the \"remove-surface\" command which has the form:\n") +
    "\n" +
    "    remove-surface in <field>\n" +
    "\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  Other \"one-time\" commands that affect removing the surface are:\n" +
    "\n" +
    "    optimal-beamwidth is <real> degrees\n" +
    "!  As a default the algorithm uses the beamwidth in the headers but\n" +
    "!    it is sometime helpful to widen this beamwidth to eliminate\n" +
    "!    more contaminated gates.\n" +
    "\n" +
    "!  e.g.\n" +
    "!    optimal-beamwidth is 3.3 degrees\n" +
    "!    for-each-ray (put one-time cmds before this line)\n" +
    "!    remove-surface in VQ\n";

  _commandList["rewrite"] =
    std::string("!  Help file for the \"rewrite\" command which has the form:\n") +
    " \n" +
    "    rewrite\n" +
    " \n" +
    "!  This command triggers a rewrite of a sweepfile in the absence\n" +
    "!  of any other edit commands that modify the data.\n";

  _commandList["set-bad-flags"] =
    std::string("!  Help file for the \"set-bad-flags\" command which has the form:\n") +
    "\n" +
    "    set-bad-flags when <field> <below>|<above>|<between> <real> [and <real>]\n" +
    "\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "\n" +
    "!  ALWAYS clears the bad flags and then sets them based\n" +
    "!  on the condition being satified. The bad flag will not be set\n" +
    "!  if the test field contains a delete flag.\n" +
    "!  e.g.\n" +
    "    set-bad-flags when VE between -1. and 1.\n" +
    "\n" +
    "!  You can then use a command like\n" +
    "    and-bad-flags when DZ above 35.\n" +
    "!  or\n" +
    "    or-bad-flags when DZ below 0.0\n" +
    "\n" +
    "!  to create a more complex mask and then use\n" +
    "    assert-bad-flags\n" +
    "\n" +
    "!  in various fields. See\n" +
    "    bad-flags-mask\n";

  _commandList["threshold"] =
    std::string("!  Help file for the \"threshold\" command which has the form:\n") +
    "\n" +
    "    threshold <field> on <field> <below>|<above> <real>\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "\n" +
    "!  Two example commands are:\n" +
    "    threshold VT on NCP below 0.333\n" +
    "    threshold VT on SW above 5.0\n";

  _commandList["unconditional-delete"] =
    std::string("!  Help file for the \"unconditional-delete\" command which has the form:\n") +
    "\n" +
    "    unconditional-delete in <field> \n" +
    "\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  Should be use ONLY with a boundary present. Deletes gates that\n" +
    "!  satisfy boundary conditions.\n" +
    "!  e.g.\n" +
    "    unconditional-delete in VT\n";

  _commandList["xor-bad-flags"] =
    std::string("!  Help file for the \"xor-bad-flags\" command which has the form:\n") +
    "\n" +
    "    xor-bad-flags when <field> <below>|<above>|<between> <real> [and <real>]\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "!  Does and EXCLUSIVE OR of the result of the condition with\n" +
    "!  the bad flag. The bad flag field is not modified\n" +
    "!  if the test field contains a bad flag.\n" +
    "!  e.g.\n" +
    "    xor-bad-flags when DZ below 0.0\n" +
    "    xor-bad-flags when VE between -1. and 1.\n";

  _commandList["xy-listing"] =
    std::string("!  Help file for the \"xy-listing\" command which has the form:\n") +
    "\n" +
    "    xy-listing of <field> and <field>\n" +
    "\n" +
    "!  Replace angle brackets and argument types with appropriate arguments.\n" +
    "\n" +
    "!  This operation creates an ASCII file with two columns one for each \n" +
    "!  field. This file is closed at the end of each pass and a new one\n" +
    "!  opened for the next pass.\n" +
    "\n" +
    "!  \"xy-directory <directory>\" allows the user to choose a destination\n" +
    "!  directory for this file. This directory is synonymous with the\n" +
    "!  \"histogram-directory\". The file name begins with the prefix \"xyp\".\n" +
    "\n" +
    "!  The \"histogram-comment\" is appended to the file name plus the two\n" +
    "!  field names.\n" +
    "\n" +
    "!  The time stamp in the file name is the start of the data; therefore,\n" +
    "!  THE ONLY WAY to distinguish two passes through the data for the same\n" +
    "!  time and the same two variables is with the \"histogram-comment\" \n";
}


/**********************************************************************
 * _selectCommand()
 */

void EditCmdListHelpWindow::_selectCommand(const Gtk::TreeModel::Path &path,
					   Gtk::TreeViewColumn *column)
{
  int row_number = atoi(path.to_string().c_str());
  std::string selected_command = _cmdListView.get_text(row_number);

  std::string help_text = _commandList[selected_command];
  
  new EditCmdHelpWindow(_defaultFont, help_text);
}


/**********************************************************************
 * _setCommandList()
 */

void EditCmdListHelpWindow::_setCommandList()
{
  _cmdListView.clear_items();
  for (std::map< std::string, std::string >::const_iterator command =
	 _commandList.begin();
       command != _commandList.end(); ++command)
    _cmdListView.append_text(command->first);
}
