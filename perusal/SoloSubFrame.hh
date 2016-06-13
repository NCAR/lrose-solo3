#ifndef SOLOSUBFRAME_HH_
#define SOLOSUBFRAME_HH_

#include <gdkmm/screen.h>
#include <gtkmm/window.h>

#include "Circle.hh"
#include "CRectangle.hh"
#include <dd_math.h>
#include "Label.hh"
#include "Line.hh"
#include "RGBImage.hh"
#include <se_bnd.hh>
#include <se_utils.hh>
#include <se_wgt_stf.hh>
#include "SeBoundaryList.hh"
#include <seds.h>
#include "sii_externals.h"
#include "sii_param_widgets.hh"
#include "sii_xyraster.hh"
#include <solo_editor_structs.h>
#include <solo_list_widget_ids.h>
#include "solo2.hh"
#include "soloii.hh"
#include "sp_basics.hh"

class SoloSubFrame : public Gtk::Window
{
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */
   
	SoloSubFrame();

  /**
   * @brief Constructor.
   *
   * @param[in] nframe                  Frame number.
   * @param[in] blowup                  Blow up indicator.
   * @param[in] imageclippingregion     Image clipping region.
   * @param[in] bup_factor              Blow up factor.
   * @param[in] redraw_bnd_ind          Redraw boundary indicator.
   */
   
	SoloSubFrame(guint nframe, gboolean blowup, GdkRectangle *imageclippingregion, guint bup_factor, gboolean redraw_bnd_ind);

  /**
   * @brief Destructor.
   */
   
	virtual ~SoloSubFrame();

  /**
   * @brief Draw frame function.
   */
   
	void Draw();

protected:

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Draw the base image.
   *        This includes the color bar, but not the color bar labels.
   */

	void _DrawBaseImage();

  /**
   * @brief Draw center crosshairs.
   */

	void _DrawCenterCrosshair();

  /**
   * @brief Get the center and landmark locations, needed for rendering the overlays.
   */

	void _CalcCenterLandmark();

  /**
   * @brief Draw the plot title and the color bar labels.
   */

	void _DrawPlotTitleColorbarLabels();

  /**
   * @brief Set the color for drawing the overlays.
   */

	void _SetColorOverlays();

  /**
   * @brief Draw the range rings.
   */

	void _DrawRangeRings();

  /**
   * @brief Draw the azimuth lines.
   */

	void _DrawAzimuthLines();

  /**
   * @brief Find the maximum range.
   */

	void _FindMaxRange();

  /**
   * @brief Draw the X/Y tic marks.
   */

	void _DrawXYTicks();

  /**
   * @brief Set the clip rectangle.
   */

	void _SetClipRectangle();

  /**
   * @brief Draw a two pixel width border.
   */

	void _DrawBorders();

  /**
   * @brief Look for partial circles.
   */

	gint _RngRingArcs(SiiFrameConfig *sfc, gdouble range);

  /**
   * @brief Set title font.
   */

	void _SetTitleFont();

  /**
   * @brief Get layout pixel size. 
   *        Determines the logical width and height of a PangoLayout in device units.
   *
   * @param[in] text    The text needs to measure.
   * @param[in] width   The logical width.
   * @param[in] height  The logical height.
   */

  void _GetLayoutPixelSize(const std::string &text, gint *width, gint *height);

  /**
   * @brief Draw RGB image.
   *
   * @param[in] data             Image data in 8-bit/sample packed format.
   * @param[in] has_alpha        Whether the data has an opacity channel.
   * @param[in] bits_per_sample  Number of bits per sample.
   * @param[in] width            Width of the image in pixels, must be > 0.
   * @param[in] height           Height of the image in pixels, must be > 0.
   * @param[in] rowstride        Distance in bytes between row starts.
   * @param[in] pixbuf_x         The x coordinate of the top-left corner in the drawable. 
   * @param[in] pixbuf_y         The y coordinate of the top-left corner in the drawable. 
   */

    void _DrawRGBImage(const guchar *data, gboolean has_alpha, 
    				   int bits_per_sample, int width, int height, int rowstride, 
                       gint pixbuf_x, gint pixbuf_y);
                       
  /**
   * @brief Draw Circle.
   *
   * @param[in] dashflag  Flag indicating whether the line is dashed or not.
   *                      True for dashed line, false for normal line.
   * @param[in] x         The x coordinate of the center.
   * @param[in] y         The y coordinate of the center.
   * @param[in] r         The radius of the circle. 
   */
	void _DrawCircle(gboolean dashflag, gint x, gint y, gint r);                 

  /**
   * @brief Draw a line.
   *
   * @param[in] dashflag  Flag indicating whether the line is dashed or not.
   *                      True for dashed line, false for normal line.
   * @param[in] x1        The x-axis of the starting point.
   * @param[in] y1        The y-axis of the starting point.
   * @param[in] x2        The x-axis of the ending point.
   * @param[in] y2        The y-axis of the ending point.
   */

	void _DrawLine(gboolean dashflag, gint x1, gint y1, gint x2, gint y2);

  /**
   * @brief Draw a rectangle.
   *
   * @param[in] fill       Flag indicating whether to fill the rectangle.
   *                       True for fill, false for not fill.
   * @param[in] x          The X coordinate of the top left corner of the rectangle.
   * @param[in] y          The Y coordinate of the top left corner of the rectangle.
   * @param[in] width      The width of the rectangle.
   * @param[in] height     The height of the rectangle.
   */

	void _DrawRectangle(gboolean fill, gint x, gint y, gint width, gint height);

  /**
   * @brief Draw a rectangle.
   *
   * @param[in] fill       Flag indicating whether to fill the rectangle.
   *                       True for fill, false for not fill.
   * @param[in] lineWidth  The line width. The default value is 1.0.
   * @param[in] x          The X coordinate of the top left corner of the rectangle.
   * @param[in] y          The Y coordinate of the top left corner of the rectangle.
   * @param[in] width      The width of the rectangle.
   * @param[in] height     The height of the rectangle.
   */

	void _DrawRectangle(gboolean fill, gint lineWidth, gint x, gint y, gint width, gint height);
	
  /**
   * @brief Set foreground color.
   *
   * @param[in] gcolor     GdkColor.
   */

	void _SetForegroundColor(GdkColor *gcolor);

  /**
   * @brief Draw text.
   *
   * @param[in] text    The text needs to draw.
   * @param[in] x       The X coordinate of the position of the text.
   * @param[in] y       The Y coordinate of the position of the text.
   */

  void _DrawText(const std::string &text, gint x, gint y);

private:  

  /////////////////////
  // Private members //
  /////////////////////

  gchar *small_pro_fonta;
  gchar *med_pro_fonta;
  gchar *big_pro_fonta;  

  SiiArc *m_sii_arcs;
  guint m_plot_function;
  guint m_frame_num;
  gint m_bup_factor;
  gdouble m_xctr, m_yctr;
  gdouble m_xlmrk, m_ylmrk;
  gint m_exposed;
  gboolean m_redraw_bnd;
  
  gdouble max_range;
  gdouble pixels_per_km;
  SiiPoint *pt_max;
  gchar *m_font;
  Rectangle clip;
  bool xtics;
  bool ytics;

  gboolean time_series;
  gdouble ts_start;
  gdouble ts_span;

  GdkRectangle *m_image_clipping_region;
  gboolean m_blow_up;
  SiiFrameConfig *m_sfc;
  WW_PTR m_wwptr;
  guchar *m_img;
  GtkWidget *m_frame;
  cairo_t *m_cr;
  
  /////////////////////
  // Private methods //
  /////////////////////

  gdouble __sii_tic1 (gdouble tic, gdouble ticinc);
  void __sii_lmrk_coords_to_frame_coords (SiiFrameConfig *sfc, WW_PTR wwptr,
					  gdouble xx_km, gdouble yy_km,
					  gint *x, gint *y);
  gboolean __sii_lmrk_inside (SiiFrameConfig *sfc);				     
  void __sii_do_annotation();
  void __sii_get_clip_rectangle (Rectangle *clip);
  void __se_redraw_all_bnds_for_frame(int frame_num);
  void __se_draw_bnd_for_frame(int frame_num, struct bnd_point_mgmt *bpm, int num, int erase);

  static bool _initTimeSeries(guint frame_num,
			      gdouble &tstart, gdouble &tspan);

};

#endif /*SOLOSUBFRAME_HH_*/
