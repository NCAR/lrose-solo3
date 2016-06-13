#ifndef ColorNamesWindow_HH
#define ColorNamesWindow_HH

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/table.h>
#include <gtkmm/window.h>

class ParameterWindow;

class ColorNamesWindow : public Gtk::Window
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   *
   * @param[in] parent          The parent window for this window.
   * @param[in] default_font    The default font to use for rendering.
   */

  ColorNamesWindow(ParameterWindow *parent,
		   const Pango::FontDescription &default_font);
  
  /**
   * @brief Destructor.
   */

  virtual ~ColorNamesWindow();


  /**
   * @brief Set the list of color names
   *
   * @param[in] color_names   The list of color names.
   */

  inline void setColorNamesList(const std::vector< std::string > &color_names)
  {
    _colorNames.clear_items();
    for (std::vector< std::string>::const_iterator color_name =
           color_names.begin();
         color_name != color_names.end(); ++color_name)
      _colorNames.append_text(*color_name);
  }


protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief The parent window for this window.
   */

  ParameterWindow *_parent;
  
  /**
   * @brief The default font to use for rendering.
   */

  Pango::FontDescription _defaultFont;
  

  // Widgets //

  Gtk::VBox _vbox;
  Gtk::Button _cancelButton;
  Gtk::Label _clickLabel;
  Gtk::Table _table;
  Gtk::ScrolledWindow _colorNamesScrolledWindow;
  Gtk::ListViewText _colorNames;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  // Callback methods //

  void _selectColorName(const Gtk::TreeModel::Path &path,
			Gtk::TreeViewColumn *column);
  

};

#endif
