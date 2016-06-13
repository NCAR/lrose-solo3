#ifndef ExamineDisplayWindow_HH
#define ExamineDisplayWindow_HH

#include <gtkmm/box.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/label.h>
#include <gtkmm/layout.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/window.h>

class ExamineWindow;

class ExamineDisplayWindow : public Gtk::Window
{

public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    DISPLAY_DATA,
    DISPLAY_RAYS,
    DISPLAY_METADATA,
    DISPLAY_EDIT_HISTORY
  } display_state_t;
  
    
  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   *
   * @param[in] parent               The parent window for this window.
   * @param[in] default_font         The default font to use for rendering.
   * @param[in] max_cells            The maximum number of cells in the window.
   * @param[in] max_chars_per_line   The maximum number of characters per line.
   */

  ExamineDisplayWindow(ExamineWindow *parent,
		       const Pango::FontDescription &default_font,
		       const int max_cells,
		       const int max_chars_per_line);
  
  /**
   * @brief Destructor.
   */

  virtual ~ExamineDisplayWindow();


  void refreshList(const std::vector< std::string > &examine_list,
		   const int max_cells,
		   const double clicked_range_km,
		   const double r0_km,
		   const double gs_km,
		   const display_state_t display_state);
  

  // Access methods //

  int getLabelHeight() const
  {
    return _labelHeight;
  }
  

  // Access methods -- widgets //

  int getLayoutHeight() const
  {
    return _layout.get_allocation().get_height();
  }
  
  void setLayoutVadjValue(const int value)
  {
    _layout.get_vadjustment()->set_value(value);
  }
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief The parent window for this window.
   */

  ExamineWindow *_parent;
  
  /**
   * @brief The default font to use for rendering.
   */

  Pango::FontDescription _defaultFont;
  

  int _priorNumEntries;
  
  int _labelWidth;    // label_char_width
  int _labelHeight;
  int _lastListIndex;
  int _lastCharIndex;
  
  // Widgets //

  Gtk::VBox _vbox00;
  
  Gtk::Label _labelBar0;
  Gtk::Label _labelBar1;
  
  Gtk::VBox _vbox0;
  Gtk::ScrolledWindow _scrolledWindow;
  Gtk::Layout _layout;
  
  Gtk::EventBox *_eventBoxes;
  Gtk::Label *_labelItems;


  ///////////////////////
  // Protected methods //
  ///////////////////////

  void _applyChanges();
  
  void _clickInList(const int which_list_entry,
		    const int which_character);
  
  void _scrollList(const int which_widget_button);
  

  // Callback methods //

  bool _buttonPressEvent(GdkEventButton *event_button, const int label_index);
  bool _keyReleaseEvent(GdkEventKey *event_key);

};

#endif
