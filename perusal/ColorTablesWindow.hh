#ifndef ColorTablesWindow_HH
#define ColorTablesWindow_HH

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/table.h>
#include <gtkmm/window.h>

#include <ColorTableManager.hh>

class ParameterWindow;

class ColorTablesWindow : public Gtk::Window
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

  ColorTablesWindow(ParameterWindow *parent,
		      const Pango::FontDescription &default_font);
  
  /**
   * @brief Destructor.
   */

  virtual ~ColorTablesWindow();


  /**
   * @brief Set the list of table names
   *
   * @param[in] table_names   The list of table names.
   */

  inline void setTableNamesList(const std::vector< std::string > &table_names)
  {
    _tableNames.clear_items();
    for (std::vector< std::string>::const_iterator table_name =
           table_names.begin();
         table_name != table_names.end(); ++table_name)
      _tableNames.append_text(*table_name);
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
  Gtk::ScrolledWindow _tableNamesScrolledWindow;
  Gtk::ListViewText _tableNames;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  // Callback methods //

  void _selectTableName(const Gtk::TreeModel::Path &path,
			Gtk::TreeViewColumn *column);
  

};

#endif
