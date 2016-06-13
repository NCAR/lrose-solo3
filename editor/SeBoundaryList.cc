#include <iostream>
#include <string.h>

#include <se_bnd.hh>
#include "SeBoundaryList.hh"

// Global variables

SeBoundaryList *SeBoundaryList::_instance = (SeBoundaryList *)0;


/*********************************************************************
 * Constructor
 */

SeBoundaryList::SeBoundaryList() :
  firstBoundary(0),
  operateOutsideBnd(0),
  numPoints(0),
  lineThickness(0),
  sizeofModList(0),
  numMods(0),
  lastOperation(0),
  modList(0),
  lastLine(0),
  lastPoint(0),
  directoryText(""),
  fileNameText(""),
  commentText("no_comment"),
  lastBoundaryPointText(""),
  numBoundaries(0),
  totalBoundaries(0),
  currentBoundary(0),
  proximityParameter(0),
  viewBounds(0),
  absorbingBoundary(0)
{
  firstBoundary = se_malloc_one_bnd();
  currentBoundary = firstBoundary;
  firstBoundary->last = firstBoundary;

  // Set the singleton instance pointer

  _instance = this;
}


/*********************************************************************
 * Destructor
 */

SeBoundaryList::~SeBoundaryList()
{
}


/*********************************************************************
 * Inst()
 */

SeBoundaryList *SeBoundaryList::getInstance()
{
  if (_instance == 0)
    new SeBoundaryList();
  
  return _instance;
}


/*********************************************************************
 * zapLastPoint()
 */

void SeBoundaryList::zapLastPoint()
{
  struct one_boundary *ob = currentBoundary;
  if (ob->num_points > 1)
    se_draw_bnd(ob->top_bpm->last, 2, YES); /* erase it first */
  se_zap_last_bpm(&ob->top_bpm);
  if (ob->num_points > 0)
    ob->num_points--;
  lastOperation = BND_POINT_DELETED;
}
