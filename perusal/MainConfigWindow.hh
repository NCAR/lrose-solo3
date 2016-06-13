#ifndef MainConfigWindow_HH
#define MainConfigWindow_HH

#include <iostream>

#include <gdk/gdk.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/fileselection.h>
#include <gtkmm/label.h>
#include <gtkmm/listviewtext.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/table.h>
#include <gtkmm/treepath.h>
#include <gtkmm/window.h>

#include <solo2.hh>

class MainConfigWindow : public Gtk::Window
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   *
   * @param[in] default_font   The default font to use for the widgets.
   */

  MainConfigWindow(const Pango::FontDescription &default_font);
  
  /**
   * @brief Destructor.
   */

  virtual ~MainConfigWindow();


  // Access methods //

  /**
   * @brief Set the main configuration file directory.
   *
   * @param[in] config_dir  The new configuration file directory.
   */

  void setConfigDir(const Glib::ustring &config_dir);
  
  /**
   * @brief Set the sweep file directory.
   *
   * @param[in] sweep_file_dir  The new sweep file directory.
   */

  inline void setSweepFileDir(const Glib::ustring &sweep_file_dir)
  {
    // Set the directory

    _swpfiDirEntry.set_text(sweep_file_dir);
  }
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  // NOTE: See if we need this.  We might just need the Widget that contains
  // this information.

  Glib::ustring _gsConfigFileNames;
  
  Pango::FontDescription _defaultFont;
  
  Gtk::VBox _vbox;
  Gtk::HBox _hbox;

  Gtk::Table _widgetTable;

  Gtk::Button _saveConfigButton;
  Gtk::Button _listConfigsButton;
  Gtk::Button _cancelButton;
  Gtk::Label _helpLabel;

  Gtk::Label _swpfiDirLabel;
  Gtk::Entry _swpfiDirEntry;
  
  Gtk::Label _blankLabel;
  Gtk::Button _swpfiFileSelectButton;
  
  Gtk::Label _configDirLabel;
  Gtk::Entry _configDirEntry;

  Gtk::Label _commentLabel;
  Gtk::Entry _commentEntry;

  Gtk::CheckButton _ignoreButton;
  Gtk::Button _configFileSelectButton;
  
  Gtk::Label _selectLabel;
  Gtk::ScrolledWindow _mainConfigScrolledWindow;
  Gtk::ListViewText _mainConfigText;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief See if we are ignoring the sweep file information.
   *
   * @return Returns true if we are ignoring the sweep file information,
   *         false otherwise.
   */

  inline bool _isIgnoreSweepFileInfo() const
  {
    return _ignoreButton.get_active();
  }
  

  /**
   * @brief Compile a list of config files in the indicated directory.
   *
   * @param[in] dir    Directory path.
   *
   * @return Returns the list of configuration files.
   */

  static std::vector< std::string > _listConfigFiles(const std::string &dir);
  

  /**
   * @brief Set the text in the configuration file list.
   *
   * @param[in] file_list   The new list of configuration files.
   */

  inline void _setMainConfigText(const std::string &dir)
  {
    std::vector< std::string > file_list = _listConfigFiles(dir);
    
    _mainConfigText.clear_items();
    
    for (std::vector< std::string >::const_iterator file = file_list.begin();
	 file != file_list.end(); ++file)
      _mainConfigText.append_text(*file);
  }
  

  // Callback methods //

  void _configFileSelected(Gtk::FileSelection *file_selector);
  void _configFileSelectedFromList(const Gtk::TreeModel::Path &path,
				   Gtk::TreeViewColumn *column);
  bool _configTextMouseButtonPressed(GdkEventButton *event_button);
  void _deleteFileSelectionPointer(Gtk::FileSelection *file_selector);
  void _loadConfigFileList();
  void _selectConfigFile();
  void _selectSweepFile();
  void _setSweepFileDir();
  void _setWindowInfo();
  void _sweepFileSelected(Gtk::FileSelection *file_selector);
  
};

#endif
