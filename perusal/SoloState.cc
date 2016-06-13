#include <iostream>

#include "SoloState.hh"


// Global variables

SoloState *SoloState::_instance = (SoloState *)0;

/*********************************************************************
 * Constructor
 */

SoloState::SoloState() :
  _haltFlag(false)
{
  // Set the instance pointer to point to this singleton instance

  _instance = this;
}


/*********************************************************************
 * Destructor
 */

SoloState::~SoloState()
{
}


/*********************************************************************
 * getInstance()
 */

SoloState *SoloState::getInstance()
{
  if (_instance == 0)
    new SoloState();
  
  return _instance;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
