#ifndef RemoveSurfaceCmd_HH
#define RemoveSurfaceCmd_HH

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#endif

#include <ForEachRayCmd.hh>

class RemoveSurfaceCmd : public ForEachRayCmd
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  RemoveSurfaceCmd();

  /**
   * @brief Constructor.
   */

  RemoveSurfaceCmd(const std::string &keyword,
		   const std::string &cmd_template);

  /**
   * @brief Destructor.
   */

  virtual ~RemoveSurfaceCmd();

#ifdef USE_RADX
  
  virtual bool doIt(const int frame_num, RadxRay &ray) const;

#else

  virtual bool doIt() const;

#endif  
  
  /**
   * @brief Clone this object.
   *
   * @return Returns a clone of this object.
   */

  virtual UiCommand *clone() const
  {
    return new RemoveSurfaceCmd(*this);
  }
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  ///////////////////////
  // Protected methods //
  ///////////////////////

#ifdef USE_RADX
  /**
   * @brief Remove the surface from the given ray.
   *
   * @param[in] frame_num     The frame number.
   * @param[in,out] ray       The ray to process.
   * @param[in] param_name    The parameter name to process.
   * @param[in] surface_only  Flag indicating whether to do the surface only.
   * @param[in] only_2_trip   Flag indicating whether to do second trip only.
   *
   * @return Returns true on success, false on failure.
   */

  bool _removeSurface(const int frame_num, RadxRay &ray,
		      const std::string &param_name,
		      const bool surface_only,
		      const bool only_2_trip) const;
#endif
  
};

#endif
