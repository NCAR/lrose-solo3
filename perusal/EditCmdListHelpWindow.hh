#ifndef EditCmdListHelpWindow_HH
#define EditCmdListHelpWindow_HH

#include <map>
#include <string>
#include <vector>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/listviewtext.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/window.h>

class EditWindow;

class EditCmdListHelpWindow : public Gtk::Window
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

  EditCmdListHelpWindow(EditWindow *parent,
			const Pango::FontDescription &default_font);
  
  /**
   * @brief Destructor.
   */

  virtual ~EditCmdListHelpWindow();


protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief The parent window for this window.
   */

  EditWindow *_parent;
  
  /**
   * @brief The default font to use for rendering.
   */

  Pango::FontDescription _defaultFont;
  
  /**
   * @brief The mapping that associates the command name with the help
   *        text.  The key is the command name, which appears in the command
   *        list.
   */

  std::map< std::string, std::string > _commandList;
  
  // Widgets //

  Gtk::VBox _vbox;
  Gtk::Button _cancelButton;
  Gtk::Label _clickLabel;

  Gtk::ScrolledWindow _cmdListViewWindow;
  Gtk::ListViewText _cmdListView;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Initialize the command list/help text mapping.
   */

  void _initCommandList();

  /**
   * @brief Set the text list in the _cmdListView object to match the
   *        commands in the _commandList mapping.
   */

  void _setCommandList();
  
  
  // Callback methods //

  void _selectCommand(const Gtk::TreeModel::Path &path,
		      Gtk::TreeViewColumn *column);
  

};

#endif
