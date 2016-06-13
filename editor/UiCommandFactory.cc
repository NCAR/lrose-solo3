#include <ASpeckleCmd.hh>
#include <AbsoluteValueCmd.hh>
#include <AddFieldCmd.hh>
#include <AddValueCmd.hh>
#include <AndBadFlagsCmd.hh>
#include <AppendHistogramToFileCmd.hh>
#include <AreaHistogramCmd.hh>
#include <AssertBadFlagsCmd.hh>
#include <AssignValueCmd.hh>
#include <BBGatesAveragedCmd.hh>
#include <BBMaxNegFoldsCmd.hh>
#include <BBMaxPosFoldsCmd.hh>
#include <BBUnfoldingCmd.hh>
#include <BBUseAcWindCmd.hh>
#include <BBUseFirstGoodGateCmd.hh>
#include <BBUseLocalWindCmd.hh>
#include <ClearBadFlagsCmd.hh>
#include <ComplementBadFlagsCmd.hh>
#include <CopyBadFlagsCmd.hh>
#include <CopyCmd.hh>
#include <CountHistogramCmd.hh>
#include <DeglitchMinGatesCmd.hh>
#include <DeglitchRadiusCmd.hh>
#include <DeglitchThresholdCmd.hh>
#include <DespeckleCmd.hh>
#include <DoHistogramCmd.hh>
#include <DontAppendHistogramToFileCmd.hh>
#include <DontUseBoundaryCmd.hh>
#include <DuplicateCmd.hh>
#include <EstablishAndResetCmd.hh>
#include <EWWindCmd.hh>
#include <ExponentiateCmd.hh>
#include <FirstGoodGateCmd.hh>
#include <FirstSweepCmd.hh>
#include <FixVortexVelocitiesCmd.hh>
#include <FlagFrecklesCmd.hh>
#include <FlagGlitchesCmd.hh>
#include <FlaggedAddCmd.hh>
#include <FlaggedAssignCmd.hh>
#include <FlaggedCopyCmd.hh>
#include <FlaggedMultiplyCmd.hh>
#include <ForcedUnfoldingCmd.hh>
#include <FreckleAverageCmd.hh>
#include <FreckleThresholdCmd.hh>
#include <GatesShiftedCmd.hh>
#include <HeaderValueCmd.hh>
#include <HistogramCommentCmd.hh>
#include <HistogramDirectoryCmd.hh>
#include <HistogramFlushCmd.hh>
#include <IgnoreFieldCmd.hh>
#include <IrregularHistogramBinCmd.hh>
#include <LastSweepCmd.hh>
#include <MapBoundaryCmd.hh>
#include <MergeFieldCmd.hh>
#include <MinBadCountCmd.hh>
#include <MinNotchShearCmd.hh>
#include <MultFieldsCmd.hh>
#include <MultiplyCmd.hh>
#include <NewHistogramFileCmd.hh>
#include <NewVersionCmd.hh>
#include <NoNewVersionCmd.hh>
#include <NotchMaxCmd.hh>
#include <NSWindCmd.hh>
#include <NyquistVelocityCmd.hh>
#include <OffsetForRadialShearCmd.hh>
#include <OmitSourceFileInfoCmd.hh>
#include <OptimalBeamwidthCmd.hh>
#include <OrBadFlagsCmd.hh>
#include <RadarNamesCmd.hh>
#include <RadialShearCmd.hh>
#include <RainRateCmd.hh>
#include <RegularHistogramParametersCmd.hh>
#include <RemoveAircraftMotionCmd.hh>
#include <RemoveOnlySecondTripSurfaceCmd.hh>
#include <RemoveOnlySurfaceCmd.hh>
#include <RemoveRingCmd.hh>
#include <RemoveStormMotionCmd.hh>
#include <RemoveSurfaceCmd.hh>
#include <RescaleFieldCmd.hh>
#include <RewriteCmd.hh>
#include <SaveCommandsCmd.hh>
#include <SelectSiteListCmd.hh>
#include <SetBadFlagsCmd.hh>
#include <ShiftFieldCmd.hh>
#include <ShowSiteValuesCmd.hh>
#include <SiteListCmd.hh>
#include <SubtractCmd.hh>
#include <SurfaceGateShiftCmd.hh>
#include <SweepDirectoryCmd.hh>
#include <ThresholdCmd.hh>
#include <UnconditionalDeleteCmd.hh>
#include <UseBoundaryCmd.hh>
#include <VersionCmd.hh>
#include <VertWindCmd.hh>
#include <XorBadFlagsCmd.hh>
#include <XYDirectoryCmd.hh>
#include <XYListingCmd.hh>

#include <se_utils.hh>
#include <sii_utils.hh>

// NOTE: These are here for the functions associated with the commands.
// After refactoring, these will go away.

#include <se_add_mult.hh>
#include <se_BB_unfold.hh>
#include <se_bnd.hh>
#include <se_catch_all.hh>
#include <se_defrkl.hh>
#include <se_flag_ops.hh>
#include <se_for_each.hh>
#include <se_histog.hh>
#include <se_proc_data.hh>

#include "UiCommandFactory.hh"


// Global variables

UiCommandFactory *UiCommandFactory::_instance = (UiCommandFactory *)0;


/*********************************************************************
 * Constructors
 */

UiCommandFactory::UiCommandFactory()
{
  // Initialize the syntactical sugar vector

  _initSynSugar();

  // Initialize the command list

  _initCommandList();
  
  // Set the instance pointer to point to this singleton instance

  _instance = this;
}


/*********************************************************************
 * Destructor
 */

UiCommandFactory::~UiCommandFactory()
{
}


/*********************************************************************
 * getInstance()
 */

UiCommandFactory *UiCommandFactory::getInstance()
{
  if (_instance == 0)
    new UiCommandFactory();
  
  return _instance;
}


/*********************************************************************
 * createFerCommand()
 */

ForEachRayCmd *UiCommandFactory::createFerCommand(const std::string &line) const
{
  // Parse the tokens in the command line and put them into the command
  // manager.  If we don't have any tokens, don't do anything.
  
  std::vector< UiCommandToken > cmd_tokens = _getCommandTokens(line);

  if (cmd_tokens.size() == 0)
    return 0;

  // Create a UiCommand object for this command

  UiCommand *command = _findFERCommand(cmd_tokens[0].getCommandText(), line);
  
  if (command == 0)
    return 0;

  // Make sure the command has the correct tokens

  command->setCommandTokens(cmd_tokens);
  
  if (!command->setCommandTemplateTokens())
  {
    sii_message(command->getErrorMsg().c_str());
    return 0;
  }

  // If we get here, the command is okay

  return (ForEachRayCmd *)command;
}


/*********************************************************************
 * createOtoCommand()
 */

OneTimeOnlyCmd *UiCommandFactory::createOtoCommand(const std::string &line) const
{
  // Parse the tokens in the command line and put them into the command
  // manager.  If we don't have any tokens, don't do anything.
  
  std::vector< UiCommandToken > cmd_tokens = _getCommandTokens(line);

  if (cmd_tokens.size() == 0)
    return 0;

  // Create a UiCommand object for this command

  UiCommand *command = _findOTOCommand(cmd_tokens[0].getCommandText(), line);
  
  if (command == 0)
    return 0;

  // Make sure the command has the correct tokens

  command->setCommandTokens(cmd_tokens);
  
  if (!command->setCommandTemplateTokens())
  {
    sii_message(command->getErrorMsg().c_str());
    return 0;
  }

  // If we get here, the command is okay

  return (OneTimeOnlyCmd *)command;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _findFERCommand()
 */

UiCommand *UiCommandFactory::_findFERCommand(const std::string &keyword,
					     const std::string &cmd_line) const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Loop through the list of all of the editor commands, looking for a command
  // with the same keyword as this command.

  UiCommand *base_cmd = 0;
  int hits = 0;
  
  for (size_t i = 0; i < _ferCmdList.size(); ++i)
  {
    // If we have an exact match, then this is definitely the command

    if (_ferCmdList[i]->getKeyword() == keyword)
    {
      base_cmd = _ferCmdList[i];
      hits = 1;
      break;
    }
    
    // Now check for a partial match.  If it isn't ambiguous, the user can
    // abbreviate the command

    if (_ferCmdList[i]->getKeyword().compare(0, keyword.size(),
					  keyword) == 0)
    {
      hits++;
      base_cmd = _ferCmdList[i];
    }
  } /* endfor - i */

  // If we didn't find this keyword or found this keyword more than once,
  // record the error

  if (hits != 1)
  {
    char msg[256];
    
    sprintf(msg, "%s\n %s %s", cmd_line.c_str(), keyword.c_str(),
	    (hits < 1) ? "not supported" : "ambiguous");
    sii_message(msg);

    return 0;
  }
  
  return base_cmd->clone();
}


/*********************************************************************
 * _findOTOCommand()
 */

UiCommand *UiCommandFactory::_findOTOCommand(const std::string &keyword,
					     const std::string &cmd_line) const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Loop through the list of all of the editor commands, looking for a command
  // with the same keyword as this command.

  UiCommand *base_cmd = 0;
  int hits = 0;
  
  for (size_t i = 0; i < _otoCmdList.size(); ++i)
  {
    // If we have an exact match, then this is definitely the command

    if (_otoCmdList[i]->getKeyword() == keyword)
    {
      base_cmd = _otoCmdList[i];
      hits = 1;
      break;
    }
    
    // Now check for a partial match.  If it isn't ambiguous, the user can
    // abbreviate the command

    if (_otoCmdList[i]->getKeyword().compare(0, keyword.size(),
					  keyword) == 0)
    {
      hits++;
      base_cmd = _otoCmdList[i];
    }
  } /* endfor - i */

  // If we didn't find this keyword or found this keyword more than once,
  // record the error

  if (hits != 1)
  {
    char msg[256];
    
    sprintf(msg, "%s\n %s %s", cmd_line.c_str(), keyword.c_str(),
	    (hits < 1) ? "not supported" : "ambiguous");
    sii_message(msg);

    return 0;
  }
  
  return base_cmd->clone();
}


/*********************************************************************
 * _getCommandTokens()
 */

std::vector< UiCommandToken > UiCommandFactory::_getCommandTokens(const std::string &line) const
{
  std::vector< UiCommandToken > cmd_tokens;
  
  char *str = new char[line.size()+1];
  strcpy(str, line.c_str());
  char *str_ptr = str;
  char *end_str = str + strlen(str);

  while (str_ptr < end_str)
  {
    char *dq = strstr(str_ptr, "\"");	/* beware of double quoted strings */

    if (dq != 0)
      *dq = '\0';

    std::vector< std::string > tokens;
    tokenize(str_ptr, tokens);

    for (size_t ii = 0; ii < tokens.size(); ii++)
    {
      UiCommandToken cmd_token;
      
      // If this token is syntactic sugar, skip it.
      
      if (std::find(_synSugar.begin(), _synSugar.end(),
		    tokens[ii]) != _synSugar.end())
	continue;

      cmd_token.setTokenType(UiCommandToken::UTT_KW);
      cmd_token.setCommandText(tokens[ii]);
      cmd_tokens.push_back(cmd_token);
      
      // Tokens will be checked and interpreted later
    }

    if (dq != 0)
    {
      // Look for ending quotes

      char *dq2 = strstr(dq + 1, "\"");
      
      if (dq2 != 0)
      {
	// Found ending quotes

	*dq2 = '\0';
      }
      else
      {
	// Didn't find ending quotes so suck up the rest of the line

	dq2 = end_str;
      }

      UiCommandToken cmd_token;
      
      cmd_token.setTokenType(UiCommandToken::UTT_KW);
      cmd_token.setCommandText(dq + 1);
      cmd_tokens.push_back(cmd_token);
      str_ptr = dq2 +1; 
    }
    else
    {
      // Should have processed the rest of the tokens if no double quotes
      // found

      break;
    }
  }

  UiCommandToken cmd_token;
  
  cmd_token.setTokenType(UiCommandToken::UTT_END);
  cmd_tokens.push_back(cmd_token);
  
  delete [] str;
  
  return cmd_tokens;
}


/*********************************************************************
 * _initCommandList()
 */

void UiCommandFactory::_initCommandList()
{
  /*
   * exhaustive list of commands with the
   * subroutine that will process each command
   * 
   * routines and files to be modified when adding a new command
   * 
   * edit this routine and  "se_all_fer_cmds()" or
   #    "se_all_other_commands()" in this file.
   * generate code to perform this command
   * add help for this command in ../perusal/help_edit.h
   * add an entry in the routine "edit_cmds_help_widget ()"
   *    in the file ../perusal/sii_edit_widget.c
   * 
   */

  // For-each-ray commands.  Put these in the order in which you want them
  // to appear in the edit window list.

  AbsoluteValueCmd *absolute_value_cmd = new AbsoluteValueCmd;
  _ferCmdList.push_back(absolute_value_cmd);
  _ferTemplates.push_back(absolute_value_cmd->getTemplate());
  
  BBUnfoldingCmd *bb_unfolding_cmd = new BBUnfoldingCmd;
  _ferCmdList.push_back(bb_unfolding_cmd);
  _ferTemplates.push_back(bb_unfolding_cmd->getTemplate());
  
  AddFieldCmd *add_field_cmd = new AddFieldCmd;
  _ferCmdList.push_back(add_field_cmd);
  _ferTemplates.push_back(add_field_cmd->getTemplate());
  
  AddValueCmd *add_value_cmd = new AddValueCmd;
  _ferCmdList.push_back(add_value_cmd);
  _ferTemplates.push_back(add_value_cmd->getTemplate());
  
  AndBadFlagsCmd *and_bad_flags_cmd = new AndBadFlagsCmd;
  _ferCmdList.push_back(and_bad_flags_cmd);
  _ferTemplates.push_back(and_bad_flags_cmd->getTemplate());
  
  AssertBadFlagsCmd *assert_bad_flags_cmd = new AssertBadFlagsCmd;
  _ferCmdList.push_back(assert_bad_flags_cmd);
  _ferTemplates.push_back(assert_bad_flags_cmd->getTemplate());
  
  AssignValueCmd *assign_value_cmd = new AssignValueCmd;
  _ferCmdList.push_back(assign_value_cmd);
  _ferTemplates.push_back(assign_value_cmd->getTemplate());
  
  ClearBadFlagsCmd *clear_bad_flags_cmd = new ClearBadFlagsCmd;
  _ferCmdList.push_back(clear_bad_flags_cmd);
  _ferTemplates.push_back(clear_bad_flags_cmd->getTemplate());
  
  ComplementBadFlagsCmd *complement_bad_flags_cmd = new ComplementBadFlagsCmd;
  _ferCmdList.push_back(complement_bad_flags_cmd);
  _ferTemplates.push_back(complement_bad_flags_cmd->getTemplate());
  
  CopyCmd *copy_cmd = new CopyCmd;
  _ferCmdList.push_back(copy_cmd);
  _ferTemplates.push_back(copy_cmd->getTemplate());
  
  CopyBadFlagsCmd *copy_bad_flags_cmd = new CopyBadFlagsCmd;
  _ferCmdList.push_back(copy_bad_flags_cmd);
  _ferTemplates.push_back(copy_bad_flags_cmd->getTemplate());
  
  DespeckleCmd *despeckle_cmd = new DespeckleCmd;
  _ferCmdList.push_back(despeckle_cmd);
  _ferTemplates.push_back(despeckle_cmd->getTemplate());
  
  DoHistogramCmd *do_histogram_cmd = new DoHistogramCmd;
  _ferCmdList.push_back(do_histogram_cmd);
  _ferTemplates.push_back(do_histogram_cmd->getTemplate());
  
  DontUseBoundaryCmd *dont_use_boundary_cmd = new DontUseBoundaryCmd;
  _ferCmdList.push_back(dont_use_boundary_cmd);
  _ferTemplates.push_back(dont_use_boundary_cmd->getTemplate());
  
  DuplicateCmd *duplicate_cmd = new DuplicateCmd;
  _ferCmdList.push_back(duplicate_cmd);
  _ferTemplates.push_back(duplicate_cmd->getTemplate());
  
  EstablishAndResetCmd *establish_and_reset_cmd = new EstablishAndResetCmd;
  _ferCmdList.push_back(establish_and_reset_cmd);
  _ferTemplates.push_back(establish_and_reset_cmd->getTemplate());
  
  ExponentiateCmd *exponentiate_cmd = new ExponentiateCmd;
  _ferCmdList.push_back(exponentiate_cmd);
  _ferTemplates.push_back(exponentiate_cmd->getTemplate());
  
  FixVortexVelocitiesCmd *fix_vortex_velocities_cmd =
    new FixVortexVelocitiesCmd;
  _ferCmdList.push_back(fix_vortex_velocities_cmd);
  _ferTemplates.push_back(fix_vortex_velocities_cmd->getTemplate());
  
  FlagFrecklesCmd *flag_freckles_cmd = new FlagFrecklesCmd;
  _ferCmdList.push_back(flag_freckles_cmd);
  _ferTemplates.push_back(flag_freckles_cmd->getTemplate());
  
  FlagGlitchesCmd *flag_glitches_cmd = new FlagGlitchesCmd;
  _ferCmdList.push_back(flag_glitches_cmd);
  _ferTemplates.push_back(flag_glitches_cmd->getTemplate());
  
  FlaggedAddCmd *flagged_add_cmd = new FlaggedAddCmd;
  _ferCmdList.push_back(flagged_add_cmd);
  _ferTemplates.push_back(flagged_add_cmd->getTemplate());
  
  FlaggedAssignCmd *flagged_assign_cmd = new FlaggedAssignCmd;
  _ferCmdList.push_back(flagged_assign_cmd);
  _ferTemplates.push_back(flagged_assign_cmd->getTemplate());
  
  FlaggedCopyCmd *flagged_copy_cmd = new FlaggedCopyCmd;
  _ferCmdList.push_back(flagged_copy_cmd);
  _ferTemplates.push_back(flagged_copy_cmd->getTemplate());
  
  FlaggedMultiplyCmd *flagged_multiply_cmd = new FlaggedMultiplyCmd;
  _ferCmdList.push_back(flagged_multiply_cmd);
  _ferTemplates.push_back(flagged_multiply_cmd->getTemplate());
  
  ForcedUnfoldingCmd *forced_unfolding_cmd = new ForcedUnfoldingCmd;
  _ferCmdList.push_back(forced_unfolding_cmd);
  _ferTemplates.push_back(forced_unfolding_cmd->getTemplate());
  
  HeaderValueCmd *header_value_cmd = new HeaderValueCmd;
  _ferCmdList.push_back(header_value_cmd);
  _ferTemplates.push_back(header_value_cmd->getTemplate());
  
  IgnoreFieldCmd *ignore_field_cmd = new IgnoreFieldCmd;
  _ferCmdList.push_back(ignore_field_cmd);
  _ferTemplates.push_back(ignore_field_cmd->getTemplate());
  
  MergeFieldCmd *merge_field_cmd = new MergeFieldCmd;
  _ferCmdList.push_back(merge_field_cmd);
  _ferTemplates.push_back(merge_field_cmd->getTemplate());
  
  MultFieldsCmd *mult_fields_cmd = new MultFieldsCmd;
  _ferCmdList.push_back(mult_fields_cmd);
  _ferTemplates.push_back(mult_fields_cmd->getTemplate());
  
  MultiplyCmd *multiply_cmd = new MultiplyCmd;
  _ferCmdList.push_back(multiply_cmd);
  _ferTemplates.push_back(multiply_cmd->getTemplate());
  
  OrBadFlagsCmd *or_bad_flags_cmd = new OrBadFlagsCmd;
  _ferCmdList.push_back(or_bad_flags_cmd);
  _ferTemplates.push_back(or_bad_flags_cmd->getTemplate());
  
  RadialShearCmd *radial_shear_cmd = new RadialShearCmd;
  _ferCmdList.push_back(radial_shear_cmd);
  _ferTemplates.push_back(radial_shear_cmd->getTemplate());
  
  RainRateCmd *rain_rate_cmd = new RainRateCmd;
  _ferCmdList.push_back(rain_rate_cmd);
  _ferTemplates.push_back(rain_rate_cmd->getTemplate());
  
  RemoveAircraftMotionCmd *remove_aircraft_motion_cmd =
    new RemoveAircraftMotionCmd;
  _ferCmdList.push_back(remove_aircraft_motion_cmd);
  _ferTemplates.push_back(remove_aircraft_motion_cmd->getTemplate());
  
  RemoveOnlySurfaceCmd *remove_only_surface_cmd = new RemoveOnlySurfaceCmd;
  _ferCmdList.push_back(remove_only_surface_cmd);
  _ferTemplates.push_back(remove_only_surface_cmd->getTemplate());
  
  RemoveOnlySecondTripSurfaceCmd *remove_only_second_trip_surface_cmd =
    new RemoveOnlySecondTripSurfaceCmd;
  _ferCmdList.push_back(remove_only_second_trip_surface_cmd);
  _ferTemplates.push_back(remove_only_second_trip_surface_cmd->getTemplate());
  
  RemoveRingCmd *remove_ring_cmd = new RemoveRingCmd;
  _ferCmdList.push_back(remove_ring_cmd);
  _ferTemplates.push_back(remove_ring_cmd->getTemplate());
  
  RemoveStormMotionCmd *remove_storm_motion_cmd = new RemoveStormMotionCmd;
  _ferCmdList.push_back(remove_storm_motion_cmd);
  _ferTemplates.push_back(remove_storm_motion_cmd->getTemplate());
  
  RemoveSurfaceCmd *remove_surface_cmd = new RemoveSurfaceCmd;
  _ferCmdList.push_back(remove_surface_cmd);
  _ferTemplates.push_back(remove_surface_cmd->getTemplate());
  
  RescaleFieldCmd *rescale_field_cmd = new RescaleFieldCmd;
  _ferCmdList.push_back(rescale_field_cmd);
  _ferTemplates.push_back(rescale_field_cmd->getTemplate());
  
  RewriteCmd *rewrite_cmd = new RewriteCmd;
  _ferCmdList.push_back(rewrite_cmd);
  _ferTemplates.push_back(rewrite_cmd->getTemplate());
  
  SetBadFlagsCmd *set_bad_flags_cmd = new SetBadFlagsCmd;
  _ferCmdList.push_back(set_bad_flags_cmd);
  _ferTemplates.push_back(set_bad_flags_cmd->getTemplate());
  
  ShiftFieldCmd *shift_field_cmd = new ShiftFieldCmd;
  _ferCmdList.push_back(shift_field_cmd);
  _ferTemplates.push_back(shift_field_cmd->getTemplate());
  
  SubtractCmd *subtract_cmd = new SubtractCmd;
  _ferCmdList.push_back(subtract_cmd);
  _ferTemplates.push_back(subtract_cmd->getTemplate());
  
  ThresholdCmd *threshold_cmd = new ThresholdCmd;
  _ferCmdList.push_back(threshold_cmd);
  _ferTemplates.push_back(threshold_cmd->getTemplate());
  
  UnconditionalDeleteCmd *unconditional_delete_cmd = new UnconditionalDeleteCmd;
  _ferCmdList.push_back(unconditional_delete_cmd);
  _ferTemplates.push_back(unconditional_delete_cmd->getTemplate());
  
  UseBoundaryCmd *use_boundary_cmd = new UseBoundaryCmd;
  _ferCmdList.push_back(use_boundary_cmd);
  _ferTemplates.push_back(use_boundary_cmd->getTemplate());
  
  XorBadFlagsCmd *xor_bad_flags_cmd = new XorBadFlagsCmd;
  _ferCmdList.push_back(xor_bad_flags_cmd);
  _ferTemplates.push_back(xor_bad_flags_cmd->getTemplate());
  
  XYListingCmd *xy_listing_cmd = new XYListingCmd;
  _ferCmdList.push_back(xy_listing_cmd);
  _ferTemplates.push_back(xy_listing_cmd->getTemplate());
  
  // One time only commands

  BBGatesAveragedCmd *bb_gates_averaged_cmd = new BBGatesAveragedCmd;
  _otoCmdList.push_back(bb_gates_averaged_cmd);
  _otoTemplates.push_back(bb_gates_averaged_cmd->getTemplate());
  
  BBMaxNegFoldsCmd *bb_max_neg_folds_cmd = new BBMaxNegFoldsCmd;
  _otoCmdList.push_back(bb_max_neg_folds_cmd);
  _otoTemplates.push_back(bb_max_neg_folds_cmd->getTemplate());
  
  BBMaxPosFoldsCmd *bb_max_pos_folds_cmd = new BBMaxPosFoldsCmd;
  _otoCmdList.push_back(bb_max_pos_folds_cmd);
  _otoTemplates.push_back(bb_max_pos_folds_cmd->getTemplate());
  
  BBUseAcWindCmd *bb_use_ac_wind_cmd = new BBUseAcWindCmd;
  _otoCmdList.push_back(bb_use_ac_wind_cmd);
  _otoTemplates.push_back(bb_use_ac_wind_cmd->getTemplate());
  
  BBUseFirstGoodGateCmd *bb_use_first_good_gate_cmd = new BBUseFirstGoodGateCmd;
  _otoCmdList.push_back(bb_use_first_good_gate_cmd);
  _otoTemplates.push_back(bb_use_first_good_gate_cmd->getTemplate());
  
  BBUseLocalWindCmd *bb_use_local_wind_cmd = new BBUseLocalWindCmd;
  _otoCmdList.push_back(bb_use_local_wind_cmd);
  _otoTemplates.push_back(bb_use_local_wind_cmd->getTemplate());
  
  ASpeckleCmd *a_speckle_cmd = new ASpeckleCmd;
  _otoCmdList.push_back(a_speckle_cmd);
  _otoTemplates.push_back(a_speckle_cmd->getTemplate());
  
  AppendHistogramToFileCmd *append_histogram_to_file_cmd =
    new AppendHistogramToFileCmd;
  _otoCmdList.push_back(append_histogram_to_file_cmd);
  _otoTemplates.push_back(append_histogram_to_file_cmd->getTemplate());

  AreaHistogramCmd *area_histogram_cmd = new AreaHistogramCmd;
  _otoCmdList.push_back(area_histogram_cmd);
  _otoTemplates.push_back(area_histogram_cmd->getTemplate());
  
  CountHistogramCmd *count_histogram_cmd = new CountHistogramCmd;
  _otoCmdList.push_back(count_histogram_cmd);
  _otoTemplates.push_back(count_histogram_cmd->getTemplate());
  
  DeglitchMinGatesCmd *deglitch_min_gates_cmd = new DeglitchMinGatesCmd;
  _otoCmdList.push_back(deglitch_min_gates_cmd);
  _otoTemplates.push_back(deglitch_min_gates_cmd->getTemplate());
  
  DeglitchRadiusCmd *deglitch_radius_cmd = new DeglitchRadiusCmd;
  _otoCmdList.push_back(deglitch_radius_cmd);
  _otoTemplates.push_back(deglitch_radius_cmd->getTemplate());
  
  DeglitchThresholdCmd *deglitch_threshold_cmd = new DeglitchThresholdCmd;
  _otoCmdList.push_back(deglitch_threshold_cmd);
  _otoTemplates.push_back(deglitch_threshold_cmd->getTemplate());
  
  DontAppendHistogramToFileCmd *dont_append_histogram_to_file_cmd =
    new DontAppendHistogramToFileCmd;
  _otoCmdList.push_back(dont_append_histogram_to_file_cmd);
  _otoTemplates.push_back(dont_append_histogram_to_file_cmd->getTemplate());
  
  EWWindCmd *ew_wind_cmd = new EWWindCmd;
  _otoCmdList.push_back(ew_wind_cmd);
  _otoTemplates.push_back(ew_wind_cmd->getTemplate());
  
  FirstGoodGateCmd *first_good_gate_cmd = new FirstGoodGateCmd;
  _otoCmdList.push_back(first_good_gate_cmd);
  _otoTemplates.push_back(first_good_gate_cmd->getTemplate());
  
  FreckleAverageCmd *freckle_average_cmd = new FreckleAverageCmd;
  _otoCmdList.push_back(freckle_average_cmd);
  _otoTemplates.push_back(freckle_average_cmd->getTemplate());
  
  FreckleThresholdCmd *freckle_threshold_cmd = new FreckleThresholdCmd;
  _otoCmdList.push_back(freckle_threshold_cmd);
  _otoTemplates.push_back(freckle_threshold_cmd->getTemplate());
  
  GatesShiftedCmd *gates_shifted_cmd = new GatesShiftedCmd;
  _otoCmdList.push_back(gates_shifted_cmd);
  _otoTemplates.push_back(gates_shifted_cmd->getTemplate());
  
  HistogramCommentCmd *histogram_comment_cmd = new HistogramCommentCmd;
  _otoCmdList.push_back(histogram_comment_cmd);
  _otoTemplates.push_back(histogram_comment_cmd->getTemplate());
  
  HistogramDirectoryCmd *histogram_directory_cmd = new HistogramDirectoryCmd;
  _otoCmdList.push_back(histogram_directory_cmd);
  _otoTemplates.push_back(histogram_directory_cmd->getTemplate());
  
  HistogramFlushCmd *histogram_flush_cmd = new HistogramFlushCmd;
  _otoCmdList.push_back(histogram_flush_cmd);
  _otoTemplates.push_back(histogram_flush_cmd->getTemplate());
  
  IrregularHistogramBinCmd *irregular_histogram_bin_cmd =
    new IrregularHistogramBinCmd;
  _otoCmdList.push_back(irregular_histogram_bin_cmd);
  _otoTemplates.push_back(irregular_histogram_bin_cmd->getTemplate());
  
  MinBadCountCmd *min_bad_count_cmd = new MinBadCountCmd;
  _otoCmdList.push_back(min_bad_count_cmd);
  _otoTemplates.push_back(min_bad_count_cmd->getTemplate());
  
  MinNotchShearCmd *min_notch_shear_cmd = new MinNotchShearCmd;
  _otoCmdList.push_back(min_notch_shear_cmd);
  _otoTemplates.push_back(min_notch_shear_cmd->getTemplate());
  
  NotchMaxCmd *notch_max_cmd = new NotchMaxCmd;
  _otoCmdList.push_back(notch_max_cmd);
  _otoTemplates.push_back(notch_max_cmd->getTemplate());
  
  NewHistogramFileCmd *new_histogram_file_cmd = new NewHistogramFileCmd;
  _otoCmdList.push_back(new_histogram_file_cmd);
  _otoTemplates.push_back(new_histogram_file_cmd->getTemplate());
  
  NSWindCmd *ns_wind_cmd = new NSWindCmd;
  _otoCmdList.push_back(ns_wind_cmd);
  _otoTemplates.push_back(ns_wind_cmd->getTemplate());
  
  NyquistVelocityCmd *nyquist_velocity_cmd = new NyquistVelocityCmd;
  _otoCmdList.push_back(nyquist_velocity_cmd);
  _otoTemplates.push_back(nyquist_velocity_cmd->getTemplate());
  
  OptimalBeamwidthCmd *optimal_beamwidth_cmd = new OptimalBeamwidthCmd;
  _otoCmdList.push_back(optimal_beamwidth_cmd);
  _otoTemplates.push_back(optimal_beamwidth_cmd->getTemplate());
  
  OffsetForRadialShearCmd *offset_for_radial_shear_cmd =
    new OffsetForRadialShearCmd;
  _otoCmdList.push_back(offset_for_radial_shear_cmd);
  _otoTemplates.push_back(offset_for_radial_shear_cmd->getTemplate());
  
  RegularHistogramParametersCmd *regular_histogram_parameters_cmd =
    new RegularHistogramParametersCmd;
  _otoCmdList.push_back(regular_histogram_parameters_cmd);
  _otoTemplates.push_back(regular_histogram_parameters_cmd->getTemplate());
  
  SurfaceGateShiftCmd *surface_gate_shift_cmd = new SurfaceGateShiftCmd;
  _otoCmdList.push_back(surface_gate_shift_cmd);
  _otoTemplates.push_back(surface_gate_shift_cmd->getTemplate());
  
  VertWindCmd *vert_wind_cmd = new VertWindCmd;
  _otoCmdList.push_back(vert_wind_cmd);
  _otoTemplates.push_back(vert_wind_cmd->getTemplate());

  XYDirectoryCmd *xy_directory_cmd = new XYDirectoryCmd;
  _otoCmdList.push_back(xy_directory_cmd);
  _otoTemplates.push_back(xy_directory_cmd->getTemplate());
  
  // These commands don't appear in a command list

  _otoCmdList.push_back(new FirstSweepCmd);
  _otoCmdList.push_back(new LastSweepCmd);
  _otoCmdList.push_back(new MapBoundaryCmd);
  _otoCmdList.push_back(new NewVersionCmd);
  _otoCmdList.push_back(new NoNewVersionCmd);
  _otoCmdList.push_back(new OmitSourceFileInfoCmd);
  _otoCmdList.push_back(new RadarNamesCmd);
  _otoCmdList.push_back(new SaveCommandsCmd);
  _otoCmdList.push_back(new SelectSiteListCmd);
  _otoCmdList.push_back(new ShowSiteValuesCmd);
  _otoCmdList.push_back(new SiteListCmd);
  _otoCmdList.push_back(new SweepDirectoryCmd);
  _otoCmdList.push_back(new VersionCmd);
  
}


/*********************************************************************
 * _initSynSugar()
 */

void UiCommandFactory::_initSynSugar()
{
  _synSugar.push_back("to");
  _synSugar.push_back("of");
  _synSugar.push_back("in");
  _synSugar.push_back("with");
  _synSugar.push_back("from");
  _synSugar.push_back("by");
  _synSugar.push_back("put-in");
  _synSugar.push_back("is");
  _synSugar.push_back("scale");
  _synSugar.push_back("bias");
  _synSugar.push_back("when");
  _synSugar.push_back("and");
  _synSugar.push_back("gates");
  _synSugar.push_back("on");
  _synSugar.push_back("km.");
  _synSugar.push_back("degrees");
  _synSugar.push_back("deg.");
  _synSugar.push_back("deg");
  _synSugar.push_back("mps");
  _synSugar.push_back("m.");
  _synSugar.push_back("low");
  _synSugar.push_back("high");
  _synSugar.push_back("increment");
  _synSugar.push_back("around");
  _synSugar.push_back("milliseconds");
  _synSugar.push_back("meters-per-second");
}
