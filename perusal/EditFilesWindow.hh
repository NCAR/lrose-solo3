#ifndef EditFilesWindow_HH
#define EditFilesWindow_HH

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

class EditFilesWindow : public Gtk::Window
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

  EditFilesWindow(EditWindow *parent,
		   const Pango::FontDescription &default_font);
  
  /**
   * @brief Destructor.
   */

  virtual ~EditFilesWindow();


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
  
  // Widgets //

  Gtk::VBox _vbox0;
  Gtk::HBox _hbox;
  
  Gtk::Button _saveCmdsButton;
  Gtk::Button _saveBdryButton;
  Gtk::Button _cancelButton;
  
  Gtk::Label _cmdFilesLabel;
  Gtk::Table _table;
  Gtk::Label _cmdDirectoryLabel;
  Gtk::Entry _cmdDirectoryEntry;
  Gtk::Label _cmdCommentLabel;
  Gtk::Entry _cmdCommentEntry;
  Gtk::Label _cmdFilesHelpLabel;
  Gtk::ListViewText _cmdFilesList;
  Gtk::ScrolledWindow _cmdFilesScrolledWindow;
  
  Gtk::Label _boundaryFilesLabel;
  Gtk::Table _table3;
  Gtk::Label _bdryDirectoryLabel;
  Gtk::Entry _bdryDirectoryEntry;
  Gtk::Label _bdryCommentLabel;
  Gtk::Entry _bdryCommentEntry;
  Gtk::Label _bdryFilesHelpLabel;
  Gtk::ListViewText _bdryFilesList;
  Gtk::ScrolledWindow _bdryFilesScrolledWindow;
  
  Gtk::Label _latlonFilesLabel;
  Gtk::Table _table5;
  Gtk::Label _latlonDirectoryLabel;
  Gtk::Entry _latlonDirectoryEntry;
  Gtk::Label _latlonFilesHelpLabel;
  Gtk::ListViewText _latlonFilesList;
  Gtk::ScrolledWindow _latlonFilesScrolledWindow;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  std::vector< std::string > _getBdryFileList(const std::string &dir) const;
  std::vector< std::string > _getCmdFileList(const std::string &dir) const;
  std::vector< std::string > _getFileList(const std::string &dir,
					  const std::string &type) const;
  std::vector< std::string > _getLatlonFileList(const std::string &dir) const;


  // Callback methods //

  void _bdryDirectoryActivated();
  void _cmdCommentActivated();
  void _cmdDirectoryActivated();
  void _latlonDirectoryActivated();
  void _saveBoundary();
  void _saveCommands();
  void _selectBoundaryFile(const Gtk::TreeModel::Path &path,
			   Gtk::TreeViewColumn *column);
  void _selectCommandFile(const Gtk::TreeModel::Path &path,
			  Gtk::TreeViewColumn *column);
  void _selectLatlonFile(const Gtk::TreeModel::Path &path,
			 Gtk::TreeViewColumn *column);
  

};

#endif
