#ifndef ImportColorTableWindow_HH
#define ImportColorTableWindow_HH

#include <iostream>

#include <gdk/gdk.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/fileselection.h>
#include <gtkmm/label.h>
#include <gtkmm/window.h>

class ImportColorTableWindow : public Gtk::Window
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   *
   * @param[in] parent          The parent window for this window.
   * @param[in] default_font   The default font to use for the widgets.
   */

  ImportColorTableWindow(ParameterWindow *parent,
			 const Pango::FontDescription &default_font);
  
  /**
   * @brief Destructor.
   */

  virtual ~ImportColorTableWindow();


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
  Gtk::HBox _hbox;

  Gtk::Label _promptLabel;
  Gtk::Entry _colorTablePathEntry;
  Gtk::Button _okButton;
  Gtk::Button _fileSelButton;
  Gtk::Button _cancelButton;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  // Callback methods //

  void _colorTableFileSelected(Gtk::FileSelection *file_selector);
  void _deleteFileSelectionPointer(Gtk::FileSelection *file_selector);
  void _selectColorTablePath();
  void _setColorTablePath();
  
};

#endif
