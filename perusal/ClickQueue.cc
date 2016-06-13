#include "ClickQueue.hh"

// Global variables

ClickQueue *ClickQueue::_instance = (ClickQueue *)0;


/*********************************************************************
 * Constructor
 */

ClickQueue::ClickQueue() :
  _clickQueueSize(123)
{
  // Set the instance pointer to point to this singleton instance

  _instance = this;
}


/*********************************************************************
 * Destructor
 */

ClickQueue::~ClickQueue()
{
}


/*********************************************************************
 * getInstance()
 */

ClickQueue *ClickQueue::getInstance()
{
  if (_instance == 0)
    new ClickQueue();
  
  return _instance;
}


/*********************************************************************
 * getClicksInFrame()
 */

std::vector< SiiPoint > ClickQueue::getClicksInFrame(const int frame_num,
						     const size_t num_clicks) const
{
  std::vector< SiiPoint > point_list;
  
  // First, see if there are enough total clicks in the queue.  If there
  // aren't, return a blank list

  if (_clickQueue.size() < num_clicks)
    return point_list;
  
  // Return the requested number of points

  std::deque< SiiPoint >::const_iterator pt_iter = _clickQueue.begin();
  
  for (size_t i = 0; i < num_clicks; ++i, ++pt_iter)
  {
    // All of the clicks have to be in the specified frame.

    if ((int)pt_iter->frame_num != frame_num)
    {
      point_list.clear();
      return point_list;
    }

    point_list.push_back(*pt_iter);
  }

  return point_list;
}


/*********************************************************************
 * insert()
 */

void ClickQueue::insert(const SiiPoint &click)
{
  // If this click is the same as our latest click, don't do anything.

  if (!_clickQueue.empty())
  {
    SiiPoint prev_pt = _clickQueue.front();

    if (memcmp(&prev_pt, &click, sizeof(prev_pt)) == 0)
      return;
  }

  // If we get here, this is a new click.  Insert the click in the queue.

  _clickQueue.push_front(click);
  
  // Make sure our queue doesn't get too big

  if (_clickQueue.size() > _clickQueueSize)
    _clickQueue.pop_back();
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
