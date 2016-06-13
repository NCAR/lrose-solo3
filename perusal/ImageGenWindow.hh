#ifndef ImageGenWindow_HH
#define ImageGenWindow_HH

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/table.h>
#include <gtkmm/window.h>

class ImageGenWindow : public Gtk::Window
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  ImageGenWindow();
  
  /**
   * @brief Destructor.
   */

  virtual ~ImageGenWindow();

  // Access methods //

  inline void setImageDir(const Glib::ustring &image_dir)
  {
    _imageDirEntry.set_text(image_dir);
  }
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  // Widgets //

  Gtk::VBox _vbox;
  Gtk::HBox _hbox;
  Gtk::Button _makeImageButton;
  Gtk::Button _cancelButton;
  Gtk::Table _table;
  Gtk::Label _imageDirLabel;
  Gtk::Entry _imageDirEntry;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  // Callback methods //

  void _makeImage();
  

};

#endif
