#ifndef DataWindow_HH
#define DataWindow_HH

#include <iostream>

#include <gdk/gdk.h>
#include <gtkmm/box.h>
#include <gtkmm/textview.h>
#include <gtkmm/window.h>

class DataWindow : public Gtk::Window
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   *
   * @param[in] default_font   The default font to use for the widgets.
   * @param[in] frame_num      The index of the assocated frame.
   */

  DataWindow(const Pango::FontDescription &default_font,
	     const int frame_index);
  
  /**
   * @brief Destructor.
   */

  virtual ~DataWindow();


  /**
   * @brief Set the text to be displayed in the window.
   *
   * @param[in] text    The text to display.
   */

  inline void setText(const std::string &text)
  {
    Glib::RefPtr<Gtk::TextBuffer> text_buffer = _textView.get_buffer();
    text_buffer->set_text(text);
  }
  

  /**
   * @brief Toggle the window.  That is, if the window is visible, hide it.
   *        If it isn't visible, show it.
   */
  
  inline void toggleWindow()
  {
    if (is_visible())
      hide();
    else
      show_all();
  }
  
protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief Template for the text that goes in this window.
   */

  static const std::string _clickTemplate;
  
  /**
   * @brief Text to display when there isn't a current data selection.
   */

  static const std::string _sampleClickText;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief The index for the associated frame.
   */

  int _frameIndex;
  
  /**
   * @brief The default font to use for rendering.
   */

  Pango::FontDescription _defaultFont;
  

  // Widgets //

  Gtk::VBox _vbox;
  Gtk::TextView _textView;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  // Callback methods //

};

#endif
