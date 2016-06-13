#ifndef ColorManager_HH
#define ColorManager_HH

#include <algorithm>
#include <string>
#include <vector>

#include <gdk/gdk.h>

class ColorManager
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Destructor.
   */

  virtual ~ColorManager();

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the instance.
   */

  static ColorManager *getInstance();
  

  // Access methods //

  int getNumColors() const
  {
    return _colorNames.size();
  }
  
  std::string getName(const int index) const
  {
    return _colorNames[index];
  }
  
  std::vector< std::string > getNames() const
  {
    return _colorNames;
  }
  
  std::string getNamesString()
  {
    std::string return_string = "";
  
    std::vector< std::string >::const_iterator name;
    for (name = _colorNames.begin();
	 name != _colorNames.end(); ++name)
    {
      return_string += *name;
      return_string += "\n";
    }
  
    return return_string;
  }

  void addColorName(const std::string &name)
  {
    _colorNames.push_back(name);
  }
  
  GdkColor *getColor(const gchar *color_name) const
  {
//    return (GdkColor *)g_hash_table_lookup(_colorsHash, (gpointer)color_name);
    return (GdkColor *)g_tree_lookup(_colorsTree, (gpointer)color_name);
  }
  
  void addColor(const gchar *color_name, const GdkColor *gcolor)
  {
    g_hash_table_insert(_colorsHash, (gpointer)color_name, (gpointer)gcolor);
    g_tree_insert(_colorsTree, (gpointer)color_name, (gpointer)gcolor);
    _colorNames.push_back(color_name);
    sort(_colorNames.begin(), _colorNames.end());
  }
  
  GdkColor *setColor(const gchar *color_name,
		     const gfloat f_red, const gfloat f_green,
		     const gfloat f_blue);
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static ColorManager *_instance;
  
  /**
   * @brief The color name list.
   */

  std::vector< std::string > _colorNames;
  
  /**
   * @brief The colors tree.
   */

  GTree *_colorsTree;

  /**
   * @brief The colors hash table.  Note that we maintain both the color hash
   *        table and the color tree although only the tree is used.  I think
   *        we can get rid of the hash table, but want to make sure before
   *        doing so.
   */

  GHashTable *_colorsHash;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Constructor.  This is private because this is a singleton object.
   */

  ColorManager();

  static GdkColor *_normalizedRGB2Gdk(const gfloat f_red,
				      const gfloat f_green,
				      const gfloat f_blue);
  
  void _setAllXNamedColors();
  
  void _setDefaultColors();
  
  GdkColor *_toneDownColor(const gchar *name, const gfloat atten);
  
};

#endif
