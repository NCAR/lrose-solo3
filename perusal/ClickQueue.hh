#ifndef ClickQueue_HH
#define ClickQueue_HH

#include <deque>

#include <soloii.h>

class ClickQueue
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Destructor.
   */

  virtual ~ClickQueue();


  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the instance.
   */

  static ClickQueue *getInstance();
  
  bool empty() const
  {
    return _clickQueue.empty();
  }
  

  SiiPoint getLatestClick()
  {
    return _clickQueue.front();
  }
  

  size_t getClickCount() const
  {
    return _clickQueue.size();
  }
  

  std::vector< SiiPoint > getClicksInFrame(const int frame_num,
					   const size_t num_clicks) const;
  

  void insert(const SiiPoint &click);
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static ClickQueue *_instance;
  

  size_t _clickQueueSize;
  std::deque< SiiPoint > _clickQueue;


  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Constructor -- Protected since this is a singleton object.
   */

  ClickQueue();


};

#endif
