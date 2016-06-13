#ifndef ColorPalettesWindow_HH
#define ColorPalettesWindow_HH

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/table.h>
#include <gtkmm/window.h>

class ParameterWindow;

class ColorPalettesWindow : public Gtk::Window
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

  ColorPalettesWindow(ParameterWindow *parent,
		      const Pango::FontDescription &default_font);
  
  /**
   * @brief Destructor.
   */

  virtual ~ColorPalettesWindow();


  /**
   * @brief Update the window to display the latest palette list.
   */

  void updatePaletteList();
  

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
  Gtk::ScrolledWindow _paletteNamesScrolledWindow;
  Gtk::ListViewText _paletteNames;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  // Callback methods //

  void _selectPaletteName(const Gtk::TreeModel::Path &path,
			  Gtk::TreeViewColumn *column);
  

};

#endif
