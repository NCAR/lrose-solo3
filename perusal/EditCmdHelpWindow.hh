#ifndef EditCmdHelpWindow_HH
#define EditCmdHelpWindow_HH

#include <map>
#include <string>
#include <vector>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/window.h>

class EditCmdHelpWindow : public Gtk::Window
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   *
   * @param[in] default_font    The default font to use for rendering.
   * @param[in] help_text       The help text to display.
   */

  EditCmdHelpWindow(const Pango::FontDescription &default_font,
		    const std::string help_text);
  
  /**
   * @brief Destructor.
   */

  virtual ~EditCmdHelpWindow();


protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief The default font to use for rendering.
   */

  Pango::FontDescription _defaultFont;
  

  // Widgets //

  Gtk::VBox _vbox;
  Gtk::TextView _textView;
  Gtk::ScrolledWindow _textViewWindow;
  Gtk::Button _dismissButton;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  // Callback methods //

  void _dismiss();

};

#endif
