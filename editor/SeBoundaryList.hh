#ifndef SeBoundaryList_HH
#define SeBoundaryList_HH

#include <sed_shared_structs.h>
#include <solo_editor_structs.h>

#include <PointInSpace.hh>

class SeBoundaryList
{

public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    BND_POINT_APPENDED = 0,
    BND_POINT_INSERTED = 1,
    BND_POINT_DELETED = 2,
    BND_POINT_MOVED = 3,
    BND_CLEARED = 4,
    BND_SHIFT = 5,
    BND_OTHER = 6
  } operations_t;
  
  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Destructor.
   */

  virtual ~SeBoundaryList();

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the instance.
   */

  static SeBoundaryList *getInstance();
  
  /**
   * @brief Remove the last point from the current boundary.
   */

  void zapLastPoint();
  

  ////////////////////
  // Public members //
  ////////////////////

  /**
   * @brief Pointer to the first boundary in the list.
   */

  struct one_boundary *firstBoundary;
  
  /**
   * @brief Flag indicating whether to operate inside or outside of the
   *        boundary.  Zero implies inside the boundary, non-zero implies
   *        outside the boundary.
   */

  int operateOutsideBnd;
  
  /**
   * @brief Number of points in the boundary.  Used to indicate the presence
   *        or absence of a boundary(s).
   */

  int numPoints;
  
  /**
   * @brief Line thickness in pixels.
   */

  int lineThickness;
  
  /**
   * @brief Size of the modification list (in bytes?).
   */

  int sizeofModList;
  
  /**
   * @brief Number of modifications.
   */

  int numMods;
  
  /**
   * @brief Last operation.
   */

  int lastOperation;
  
  /**
   * @brief Modification list.
   */

  struct bnd_point_mgmt **modList;
  
  /**
   * @brief Last line.
   */

  struct bnd_point_mgmt *lastLine;
  
  /**
   * @brief Last point.
   */

  struct bnd_point_mgmt *lastPoint;
  
  /**
   * @brief Directory text.
   */

  std::string directoryText;
  
  /**
   * @brief File name text.
   */

  std::string fileNameText;
  
  /**
   * @brief Comment text.
   */

  std::string commentText;
  
  /**
   * @brief Last boundary point text.
   */

  std::string lastBoundaryPointText;
  
  /**
   * @brief Number of boundaries.
   */

  int numBoundaries;
  
  /**
   * @brief Total boundaries.
   */

  int totalBoundaries;
  
  /**
   * @brief Pointer to the current boundary.
   */

  struct one_boundary *currentBoundary;
  
  /**
   * @brief Proximity parameter.  The mouse is considered to be within proximity
   *        of a line if it is within a rectangle centered on the line whose
   *        length is the length of the line and whose width is two times the
   *        PP unless the line width is less than the PP.  Then the PP is
   *        defined as .666 times the length of the line.
   */

  int proximityParameter;
  
  /**
   * @brief ???
   */

  PointInSpace pisp;
  
  /**
   * @brief View bounds.
   */

  int viewBounds;
  
  /**
   * @brief Absorbing boundary.
   */

  int absorbingBoundary;
  

  ////////////////////
  // Public methods //
  ////////////////////

  /////////////////
  // set methods //
  /////////////////

  /**
   * @brief Set the origin.
   *
   * @param[in] origin    The new origin.
   */

  inline void setOrigin(const PointInSpace &origin)
  {
    _origin = origin;
  }
  
  /**
   * @brief Set the origin ID value.
   *
   * @param[in] origin_id   The origin ID.
   */

  inline void setOriginId(const std::string &origin_id)
  {
    _origin.setId(origin_id);
  }
  
  
  /////////////////
  // get methods //
  /////////////////

  /**
   * @brief Get the origin of the boundaries.
   *
   * @return Returns the origin of the boundaries.
   */

  inline PointInSpace getOrigin() const
  {
    return _origin;
  }
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static SeBoundaryList *_instance;
  
  /**
   * @brief Origin for the boundary.
   */

  PointInSpace _origin;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Constructor.  This is private because this is a singleton object.
   */

  SeBoundaryList();

};

#endif
