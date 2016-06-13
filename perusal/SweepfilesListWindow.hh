#ifndef SweepfilesListWindow_HH
#define SweepfilesListWindow_HH

#include <map>
#include <string>
#include <vector>

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/listviewtext.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/window.h>

class SweepfileWindow;

class SweepfilesListWindow : public Gtk::Window
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

  SweepfilesListWindow(SweepfileWindow *parent,
		       const Pango::FontDescription &default_font);
  
  /**
   * @brief Destructor.
   */

  virtual ~SweepfilesListWindow();


  /**
   * @brief Reset the sweepfile list using the current directory for the
   * frame.
   */

  void resetSweepfileList();
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief The parent window for this window.
   */

  SweepfileWindow *_parent;
  
  /**
   * @brief The default font to use for rendering.
   */

  Pango::FontDescription _defaultFont;
  

  // Widgets //

  Gtk::VBox _vbox;
  Gtk::Label _helpLabel;
  Gtk::ListViewText _sweepfileListView;
  Gtk::ScrolledWindow _sweepfileListViewWindow;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  // Callback methods //

  void _selectSweepfile(const Gtk::TreeModel::Path &path,
			Gtk::TreeViewColumn *column);

};

#endif
