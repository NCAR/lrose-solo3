#ifndef SoloMainWindow_HH
#define SoloMainWindow_HH

#include <gtkmm/actiongroup.h>
#include <gtkmm/box.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/table.h>
#include <gtkmm/toggleaction.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/window.h>

#include "MainConfigWindow.hh"
#include "ImageGenWindow.hh"

class SoloMainWindow : public Gtk::Window
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  SoloMainWindow(const Pango::FontDescription &default_font);

  /**
   * @brief Destructor.
   */

  virtual ~SoloMainWindow();

  /**
   * @brief Get the main event box.
   *
   * @return Returns the main event box.
   */

  inline const Gtk::EventBox &getEventBox() const
  {
    return _eventBox;
  }
  

  /**
   * @brief Reset the table widget.  This must be done whenever the user
   *        reconfigures the display and before adding each new frame.
   *
   * @param[in] nrows    Number of rows in table.
   * @param[in] ncols    Number of columns in table.
   */

  inline void resetTable(const guint nrows, const guint ncols)
  {
    if (_table)
    {
      _eventBox.remove();
      delete _table;
    }
    
    _table = new Gtk::Table(nrows, ncols, true);
    _eventBox.add(*_table);
  }
  

  /**
   * @brief Add the given frame to the window.  When the window configuration
   *        changes, first resetTable() must be called and then each new frame
   *        must be added using this method.
   *
   * @param[in] frame         The frame object.
   * @param[in] left_attach   The left attachment for the frame.
   * @param[in] right_attach  The right attachment for the frame.
   * @param[in] top_attach    The top attachment for the frame.
   * @param[in] bottom_attach The bottom attachment for the frame.
   *
   * @note See the Gtk::Table documentation for how to set the attachment
   *       values.
   */

  inline void addFrame(Gtk::DrawingArea *frame,
		       const guint left_attach, const guint right_attach,
		       const guint top_attach, const guint bottom_attach)
  {
    _table->attach(*frame,
		   left_attach, right_attach,
		   top_attach, bottom_attach,
		   Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		   Gtk::EXPAND | Gtk::SHRINK | Gtk::FILL,
		   0,0);
  }
  
  /**
   * @brief Change the current window cursor.
   *
   * @param[in] normal  Flag indicating whether our state is normal or busy.
   *                    True for normal, false for busy.
   */

  void changeCursor(const bool normal);
  

  /**
   * @brief Configure the frames in the window in a regular grid.
   *
   * @param[in] ncols   Number of columns.
   * @param[in] nrows   Number of rows.
   */

  void configFrames(const guint ncols, const guint nrows);
      
  /**
   * @brief Show the configuration window.
   */

  void showConfigWindow();


  // Access methods //

  /**
   * @brief Set the root directory.
   *
   * @param[in] root_dir  The new root directory.
   */

  inline void setRootDir(const Glib::ustring &root_dir)
  {
    _rootDir = root_dir;

    _configWindow.setConfigDir(_rootDir);
    _configWindow.setSweepFileDir(_rootDir);
  }
  
  /**
   * @brief Set the image directory.
   *
   * @param[in] image_dir  The new image directory.
   */

  inline void setImageDir(const Glib::ustring &image_dir)
  {
    _imageGenWindow.setImageDir(image_dir);
  }
  

  /**
   * @brief See if electric centering is currently set.
   *
   * @return Returns true if electric centering is on, false otherwise.
   */

  inline bool isElectricCentering() const
  {
    return _electricCenteringToggle->get_active();
  }
  

  /**
   * @brief See if crosshair centering is currently set.
   *
   * @return Returns true if crosshair centering is on, false otherwise.
   */

  inline bool isCrosshairCentering() const
  {
    return _crosshairCenteringToggle->get_active();
  }
  
  /**
   * @brief Return the GdkWindow associated with _eventBox, i.e., the "main"
   * solo3 GdkWindow.
   * @return the GdkWindow associated with _eventBox, i.e., the "main"
   * solo3 GdkWindow.
   */
  inline GdkWindow * gdkWindow() {
    GtkWidget * widget = reinterpret_cast<GtkWidget*>(_eventBox.gobj());
    return(gtk_widget_get_window(widget));
  }

protected:

  /////////////////////
  // Protected types //
  /////////////////////

  /**
   * @brief The supported custom configurations.
   */

  typedef enum
  {
    ONE_BIG_TWOV_SMALL =  1012,
    ONE_BIG_TWO_SMALL =   2012,
    ONE_BIG_THREE_SMALL = 1013,
    ONE_BIG_FIVE_SMALL =  1015,
    ONE_BIG_SEVEN_SMALL = 1017,
    TWO_BIG_FOURV_SMALL = 1024,
    TWO_BIG_FOUR_SMALL =  2024,
    FOUR_512x512 =        4022,
    FOUR_DEFAULT =        2022
  } custom_config_t;
  
    
  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief The root directory for accessing files.  This directory applies to
   *        sweep files and configuration files.
   */

  Glib::ustring _rootDir;
  
  /**
   * @brief The default font to use for the widgets.
   */

  Pango::FontDescription _defaultFont;
  

  // Widgets //

  /**
   * @brief The main vertical box for containing the widgets in the window.
   *        At some point, this will need to be replaced by a Gtk::Grid
   *        widget.
   */

  Gtk::VBox _mainVbox;

  /**
   * @brief The event box which contains all of the frames.  This is an
   *        invisible widget that can have events attached to it.  It also
   *        contains the table used for storing the data frames.
   */

  Gtk::EventBox _eventBox;

  /**
   * @brief The table widget used for storing the data frames.  I had to make
   *        this a pointer since it didn't work to resize this table widget on
   *        the fly.
   */

  Gtk::Table *_table;
  

  // GUI creation widgets //

  /**
   * @brief The user interface manager used for creating all of the menus.
   */

  Glib::RefPtr<Gtk::UIManager> _uiManager;

  /**
   * @brief The actions used by the menu items.
   */

  Glib::RefPtr<Gtk::ActionGroup> _actionGroup;


  // Menu actions //

  Glib::RefPtr<Gtk::ToggleAction> _electricCenteringToggle;
  Glib::RefPtr<Gtk::ToggleAction> _crosshairCenteringToggle;
  

  // Child windows //

  /**
   * @brief The main configuration window.
   */

  MainConfigWindow _configWindow;

  /**
   * @brief The image generation window.
   */

  ImageGenWindow _imageGenWindow;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Create the main menu.
   *
   * @param[in] container    The parent container for the menu.
   */

  void _createMainMenu(Gtk::Box &container);
  
  // Event handlers //

  /**
   * @brief Handle a key release event.  We use a key release event rather than
   *        a key press event because some keys don't seem to generate a key
   *        press event for some reason.
   *
   * @param[in] event_key    Information about the key release event.
   *
   * @return Returns true on success, false on failure.
   */

  bool _keyReleaseEvent(GdkEventKey *event_key);

  /**
   * @brief Center all of the frames on the radar.
   */

  void _centerFrames();
  
  /**
   * @brief Center the frames based on the given number of previous clicks.
   *
   * @param[in] num_clicks   The number of clicks to use for centering.
   */

  void _centerOnClicks(const guint num_clicks);
  
  /**
   * @brief Change the data zoom using the given zoom factor.  The current
   *        zoom level is multiplied by the zoom factor to get the new zoom
   *        level, so factors greater than 1.0 will cause the frames to zoom
   *        out while factors less than 1.0 will cause the frames to zoom in.
   *        Using a zoom factor of 1.0 will cause the frames to return to their
   *        original zoom.
   *
   * @param[in] zoom_factor   The zoom factor to apply.
   */

  void _changeDataZoom(const double zoom_factor);
  
  /**
   * @brief Change the configuration zoom using the given factor.  This
   *        increases or decreases the window size by the given factor.  The
   *        factor is multiplicative, so values greater than 1.0 will cause
   *        the window to grow while factors less than 1.0 will cause the
   *        window to shrink.
   *
   * @param[in] factor   The zoom factor.
   */

  void _changeConfigZoom(const gdouble factor);
  
  /**
   * @brief Apply one of the supported custom configurations.
   *
   * @param[in] config_type   The configuration type to apply.
   */

  void _customConfig(const custom_config_t config_type);
  
  /**
   * @brief Show all of the click data widgets.
   */

  void _hideAllClickDataWidgets();

  /**
   * @brief Hide all of the click data widgets.
   */

  void _showAllClickDataWidgets();
  
  /**
   * @brief Display the basic help window.
   */

  void _displayHelpBasic();

  /**
   * @brief Display the file help window.
   */

  void _displayHelpFile();

  /**
   * @brief Display the zoom help window.
   */

  void _displayHelpZoom();

  /**
   * @brief Display the centering help window.
   */

  void _displayHelpCentering();

  /**
   * @brief Display the configuration help window.
   */

  void _displayHelpConfig();

  /**
   * @brief Display the shortcuts help window.
   */

  void _displayHelpShortcuts();

  /**
   * @brief Display the compare help window.
   */

  void _displayHelpCompare();

  /**
   * @brief Display the about help window.
   */

  void _displayHelpAbout();
  
  /**
   * @brief Callback for when the event box is entered.
   *
   * @param[in] event   The event.
   */

  bool _eventBoxEntered(GdkEventCrossing *event);

  /**
   * @brief Callback for when the event box is configured.
   *
   * @param[in] event   The event.
   */

  bool _eventBoxConfigured(GdkEventConfigure *event);
  
  /**
   * @brief Change the shapes of the frames to square.
   */

  void _shapeSquare();

  /**
   * @brief Change the shapes of the frame to a horizontal slide.
   */

  void _shapeSlide();

  /**
   * @brief Change the shapes of the frame to a vertical slide.
   */

  void _shapeVSlide();
  
};

#endif
