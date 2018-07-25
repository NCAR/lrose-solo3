/* 	$Id$	 */

#include <iostream>
#include <math.h>
#include <stdio.h>
#include <string>
#include <gtk/gtktext.h>

#include <ColorManager.hh>
#include <ColorTableManager.hh>
#include <DataManager.hh>
#include <PaletteManager.hh>
#include <se_utils.hh>
#include "sii_colors_stuff.hh"
#include "sii_links_widget.hh"
#include "sii_param_widgets.hh"
#include "sii_utils.hh"
#include <sii_widget_content_structs.h>
#include <solo_window_structs.h>
#include "solo2.hh"
#include "soloii.h"
#include "soloii.hh"
#include "sp_accepts.hh"
#include "sp_basics.hh"
#include "sp_lists.hh"
#include "SoloSubFrame.hh"
#include "CRectangle.hh"


#define CB_ROUND .000001
#define COMMA ","

static const gchar * log_rr_symbols[] = {
".01",
".03",
".06",
".16",
".4",
"1.0",
"2.5",
"6.3",
"16",
"40",
"100",
};

static const gchar * pid_symbols[] = {
"cld",
"drz",
"lrn",
"mrn",
"hrn",
"hail",
"rhm",
"gsh",
"grn",
"dsn",
"wsn",
"icr",
"iicr",
"sld",
"bgs",
"2tr",
"gclt",
};

static const gchar * pid_labelz[] = {
"Cloud",
"Drizzle",
"Light.Rain",
"Moderate.Rain",
"Heavy.Rain",
"Hail",
"Rain.Hail.Mix",
"Grpl.Sml.Hail",
"Grpl.Rain",
"Dry.Snow",
"Wet.Snow",
"Ice.Crystals",
"Irreg.Ice.Crystls",
"Spr.Cool.Liq.Drps",
"Insects",
"Birds",
"Ground.Clutter",
};

/* c---------------------------------------------------------------------- */

void init_param_data(const guint frame_num)
{
  // Get access to the parameter data.  If the data hasn't been initialized
  // yet, then initialize it.

  ParamData *pd = frame_configs[frame_num]->param_data;

  if (!pd)
  {
    pd = new ParamData;
    pd->change_count = 0;
    for (int i = 0; i < PARAM_MAX_WIDGETS; ++i)
    {
      pd->toggle[i] = false;
    }
    pd->electric_params = false;
    pd->cb_loc = 0;
    pd->cb_labels_state = 0;
    pd->num_colors = 0;
    pd->pal = 0;
    pd->orig_pal = 0;
    pd->param_links = 0;
    pd->orientation = 0;
    pd ->cb_labels = 0;
    pd->cb_symbols = 0;
    pd->field_toggle_count = 0;
    pd->fields_list = 0;
    pd->toggle_field = 0;

    frame_configs[frame_num]->param_data = pd;

    pd->param_links =
      sii_new_links_info("Parameter Links", frame_num,
			 FRAME_PARAM_LINKS, TRUE );
    frame_configs[frame_num]->link_set[LI_PARAM] = pd->param_links;
    pd->cb_loc = PARAM_CB_BOTTOM;
    pd->toggle[PARAM_HILIGHT] = TRUE;
  }
}

/* c---------------------------------------------------------------------- */

void param_set_cb_loc (int frame_num, int loc)
{
  ParamData *pd = frame_configs[frame_num]->param_data;
  ParameterWindow *param_window = frame_configs[frame_num]->param_window;
  
  switch (loc)
  {
  case -1:
    pd->cb_loc = PARAM_CB_LEFT;
    if (param_window != 0)
      param_window->setColorbarLeft();
    break;
  case 1:
    pd->cb_loc = PARAM_CB_RIGHT;
    if (param_window != 0)
      param_window->setColorbarRight();
    break;
  default:
    pd->cb_loc = PARAM_CB_BOTTOM;
    if (param_window != 0)
      param_window->setColorbarBottom();
    break;
  };
}

/* c---------------------------------------------------------------------- */

const gchar *set_cb_labeling_info (guint frame_num, gdouble *relative_locs)
{
  gdouble ctr, inc, log10diff, dval, dloc, dinc, d;
  gint jj, nc, nn, nlabs, nnlabs = 0;
  ParamData *pd = frame_configs[frame_num]->param_data;
  gchar *name;
  WW_PTR wwptr = solo_return_wwptr(frame_num);
  const gchar **syms=NULL;

  /* Take the log10 of the increment to get decimal places in the label
   * n = (gint)((f < 0) ? f-.5 : f+.5);
   */
  nc = pd->pal->getNumColors();
  ctr = pd->pal->getCenterDataValue();
  inc = pd->pal->getColorWidth();
  name = wwptr->parameter.parameter_name;

  if (pd->toggle[PARAM_CB_SYMBOLS] || wwptr->color_bar_symbols) {
    if (strstr ("RR_DSD,RNX,RZD,RKD,", name )) {
      syms = log_rr_symbols;
      nnlabs = sizeof (log_rr_symbols)/sizeof (char *);
    }
    else if (strstr ("PD,WPD,HYDID,", name )) {
      syms = pid_labelz;
      nnlabs = sizeof (pid_labelz)/sizeof (char *);
      syms = pid_symbols;
      nnlabs = sizeof (pid_symbols)/sizeof (char *);
    }
  }

  nlabs = (nc < 7) ? nc : 7;
  switch (nc) {
  case 13:
  case 12:
  case 11:
  case 10:
  case 9:
    nlabs = 9;
    break;
  };

  d = nc*inc;
  log10diff = log10 (d);

  std::string fmt;
  
  if (log10diff > 1.3)		/* 20 */
    { fmt = "%.0f "; }
  else if (log10diff > -.15)	/* .7 */
    { fmt = "%.1f "; }
  else 
    { fmt = "%.2f "; }

  nn = (gint)((double)nc/(nlabs +1) + .4);
  dinc = (double)nn/nc;
  dval = ctr - (nlabs/2) * nn * inc;
  dloc = .5 - (nlabs/2) * dinc;

  if (syms) {
    nlabs = nnlabs;
    nn = 1;
    dinc = 1.0/nc;
    dloc = .5 * dinc;
  }

  if (!pd->cb_labels)
    { pd->cb_labels = g_string_new (""); }
  else
    { g_string_truncate (pd->cb_labels, 0); }

  for (jj = 0; jj < nlabs; jj++, dval += nn*inc, dloc += dinc) {
    d = (dval < 0) ? dval-.0001 : dval+.0001;
    if (syms) {
      g_string_sprintfa (pd->cb_labels, "%s ", syms[jj]);
    }
    else {
      g_string_sprintfa (pd->cb_labels, fmt.c_str(), d);
    }
    relative_locs[jj] = dloc;
  }
  
  return (const gchar *)pd->cb_labels->str;
}

/* c---------------------------------------------------------------------- */

void set_color_bar (SiiFrameConfig *sfc, WW_PTR wwptr)
{
  // Get the parameter data

  ParamData *pd = sfc->param_data;

  // Get the length of the color bar in pixels.

  gint len = (pd->cb_loc == PARAM_CB_LEFT || pd->cb_loc == PARAM_CB_RIGHT)
    ? sfc->height : sfc->width;

  // If the colorbar needs to be longer, allocate more space for it.

  if (len > (gint)sfc->cb_pattern_len)
  {
    if (sfc->cb_pattern)
      g_free (sfc->cb_pattern);
    sfc->cb_pattern = (guchar *)g_malloc0 (len);
    sfc->cb_pattern_len = len;
  }

  len -= BORDER_PIXELS * 2;

  // Get the data value at the center of the color bar.

  double ctr = pd->pal->getCenterDataValue();

  // Get the length (in data values) of half of the color bar.  The color width
  // is the length, in data values, of each color in the bar.  (So, for refl,
  // this might be something like 5 dbz.)

  double half = .5 * pd->pal->getNumColors() * pd->pal->getColorWidth();

  // Fill in the color values for the color bar pattern

  double f_min = ctr - half;
  double f_inc = 2 * half / len;
  double f_val = f_min;

  guchar *cb_pattern = sfc->cb_pattern;

  for (gint jj = 0; jj < len; f_val += f_inc, ++jj)
  {
    gint scaled_data_value =
      (gint)DataManager::scaleValue(f_val, wwptr->parameter_scale,
				    wwptr->parameter_bias);
    if (scaled_data_value < -32768) scaled_data_value = -32768;
    if (scaled_data_value > 32767) scaled_data_value = 32767;
    *cb_pattern++ = *(wwptr->data_color_lut + scaled_data_value);
  }

  // Now take the color bar pattern and use it to fill in the image pixels
  // for the actual color bar.  These are put in the data value section of
  // the image data.

  cb_pattern = sfc->cb_pattern;

  guint font_height = sfc->font_height;

  if (pd->cb_loc == PARAM_CB_LEFT || pd->cb_loc == PARAM_CB_RIGHT)
  {
    // Color bar increases from bottom to top.  Set data_ptr to the bottom
    // position of the color bar in the data.

    guchar *data_ptr =
      (guchar *)sfc->image->data + ((sfc->height - BORDER_PIXELS) * sfc->width);

    data_ptr +=
      (pd->cb_loc == PARAM_CB_LEFT) ? 1 : sfc->width - font_height - 1;

    // Calculate the stride value for the rendering.  This is the distance we
    // need to move the data pointer to get to the next row in the color bar.
    // We need to add font_height to the frame width to get the stride because
    // we increment the data pointer through the width of the color bar when
    // rendering the bar.

    gint stride = sfc->width + font_height;

    // Render each row in the color bar

    for (gint jj = 0; jj < len; jj++, cb_pattern++, data_ptr -= stride)
    {
      guchar *end_ptr = data_ptr + font_height;
      for (; data_ptr < end_ptr; *data_ptr++ = *cb_pattern);
    }
  }
  else
  {
    // The color bar is rendered across the bottom of the frame.

    gint width = sfc->width;
    gint y = sfc->height - BORDER_PIXELS;

    guchar *data_value =
      (guchar *)sfc->image->data + (y * width) + BORDER_PIXELS;

    for (guint jj = 0; jj < font_height; jj++, data_value -= width)
    {
      memcpy (data_value, cb_pattern, len);
    }
  }
 }

/* c---------------------------------------------------------------------- */

GdkColor *sii_boundary_color (guint frame_num, gint exposed)
{
  GdkColor *gcolor;
  ParamData *pd = frame_configs[frame_num]->param_data;
  SiiPalette *pal;

  pal = (exposed) ? pd->orig_pal : pd->pal;
  gcolor = (GdkColor *)pal->getFeatureColor(SiiPalette::FEATURE_BND).gobj();

  return gcolor;
}

/* c---------------------------------------------------------------------- */

void sii_colorize_image (SiiFrameConfig *sfc)
{
  // Get the parameter data

  ParamData *pd = sfc->param_data;

  // Increment the colorize count.  Not sure what this is for.

  sfc->colorize_count++;

  // Get the image size

  gint image_size = sfc->width * sfc->height;

  // Get a pointer to the data values

  guchar *data_value = (guchar *)sfc->image->data;

  // Get a pointer to the pixel values.  The 3-byte color data space follows
  // the image data.

  guchar *pixel_value = (guchar *)sfc->image->data + (sfc->height * sfc->width);

  // Set the pixel color values.

  for (gint jj=0; jj < image_size; jj++, data_value++)
  {
    guchar red, green, blue;
    
    pd->pal->getColorTableRgb(*data_value, red, green, blue);
    
    *pixel_value++ = red;
    *pixel_value++ = green;
    *pixel_value++ = blue;
  }
}

/* c---------------------------------------------------------------------- */

void sii_double_colorize_image (guint frame_num)
{
  SiiFrameConfig *sfc = frame_configs[frame_num];
  SiiFrameConfig *sfc0 = frame_configs[0];
  ParamData *pd = sfc->param_data;

  gint stride = sfc->width * 3 * 2;

  guchar *image_data = (guchar *)sfc->image->data;
  guchar *big_image_data = (guchar *)sfc0->big_image->data;
  guchar *big_image_data_ptr = big_image_data;

  for (guint kk = 0; kk < sfc->height; kk++, big_image_data += stride)
  {
    big_image_data_ptr = big_image_data;
    for (guint jj = 0; jj < sfc->width; jj++, image_data++)
    {
      guchar red, green, blue;
      
      pd->pal->getColorTableRgb(*image_data, red, green, blue);
      
      *big_image_data++ = red;
      *big_image_data++ = green;
      *big_image_data++ = blue;
      *big_image_data++ = red;
      *big_image_data++ = green;
      *big_image_data++ = blue;
    }
    memcpy (big_image_data, big_image_data_ptr, stride);
  }
}

/* c---------------------------------------------------------------------- */

GdkColor *sii_grid_color (guint frame_num, gint exposed)
{
  GdkColor *gcolor;
  ParamData *pd = frame_configs[frame_num]->param_data;
  SiiPalette *pal;

  pal = (exposed) ? pd->orig_pal : pd->pal;
  gcolor = (GdkColor *)pal->getFeatureColor(SiiPalette::FEATURE_RNG_AZ_GRID).gobj();

  return gcolor;
}

/* c---------------------------------------------------------------------- */

void sii_initialize_parameter(const guint frame_num, const std::string &name)
{
  // If the data hasn't been initialized yet, then initialize it.

  init_param_data(frame_num);
  ParamData *pd = frame_configs[frame_num]->param_data;

  pd->pal = PaletteManager::getInstance()->setPalette(name.c_str());
  
  /* set up circular que of toggle info */

  pd->fields_list = sii_init_circ_que(MAX_FRAMES);
  pd->field_toggle_count = 2;
}

/* c---------------------------------------------------------------------- */

void sii_min_max_from_ctr_inc(const guint ncolors,
			      const gfloat ctr, const gfloat inc,
			      gfloat &min, gfloat &max )
{
   min = ctr - inc * ncolors * .5;
   max = min + inc * ncolors;

   min += (min < 0) ? -CB_ROUND : CB_ROUND;
   max += (max < 0) ? -CB_ROUND : CB_ROUND;

}

/* c---------------------------------------------------------------------- */

const gchar *sii_param_palette_name (guint frame_num)
{
   ParamData *pd = frame_configs[frame_num]->param_data;

   return pd->pal->getPaletteName().c_str();
}

/* c---------------------------------------------------------------------- */

void sii_param_set_plot_field (int frame_num, char *fname)
{
  /* for toggling */

  ParamData *pd = frame_configs[frame_num]->param_data;
  SiiLinkedList *sll = pd->fields_list;
  ParamFieldInfo *pfi;
  char str[16];

  strcpy (str, fname);
  g_strstrip (str);

  if (!sll->data) {		/* first time */
    sll->data = (gpointer)g_malloc0 (sizeof (*pfi));
    pfi = (ParamFieldInfo *)sll->data;
    strcpy (pfi->name, str);
    return;
  }
  pfi = (ParamFieldInfo *)sll->data;
  if (strcmp (pfi->name, str) == 0) /* replotting the same field */
    { return;}

  pd->toggle_field = pd->fields_list;
  pd->fields_list = sll = sll->next;

  if (!sll->data) {
    sll->data = (gpointer)g_malloc0 (sizeof (*pfi));
  }
  pfi = (ParamFieldInfo *)sll->data;
  strcpy (pfi->name, str);
}

/* c---------------------------------------------------------------------- */

void sii_reset_image (guint frame_num)
{
  SiiFrameConfig *sfc = frame_configs[frame_num];
  WW_PTR wwptr = solo_return_wwptr(frame_num);
  guchar *img;
  int jj;

  jj = wwptr->background_color_num;

  if (!(img = (guchar *)sfc->image->data))
    { return; }

  memset (img, jj, sfc->width * sfc->height);
}

/* c---------------------------------------------------------------------- */

void sii_set_param_names_list(const guint frame_num,
			      const std::vector< std::string> &param_names)
{
  init_param_data(frame_num);
  frame_configs[frame_num]->param_data->param_names_list = param_names;
}

/* c------------------------------------------------------------------------ */

bool solo_hardware_color_table(gint frame_num)
{
  gboolean ok2;
  float red, green, blue;

  // Get the needed frame information.

  ParamData *pd = frame_configs[frame_num]->param_data;
  ParameterWindow *param_window = frame_configs[frame_num]->param_window;
  GdkColormap *cmap = frame_configs[frame_num]->color_map;
  WW_PTR wwptr = solo_return_wwptr(frame_num);

  if (param_window == 0)
  {
    /* the palette might be wrong for this parameter */
    pd->pal =
      PaletteManager::getInstance()->setPalette(wwptr->parameter.parameter_name);
  }
  else if (strcmp(wwptr->parameter.parameter_name,
		  param_window->getOrigParamName().c_str()) == 0)
  {
    pd->pal =
      PaletteManager::getInstance()->setPalette(wwptr->parameter.parameter_name);
  }

  SiiPalette *pal = pd->pal;
//  struct solo_list_mgmt *lm = wwptr->list_colors;
  wwptr->num_colors_in_table = 0;
//  lm->num_entries = 0;
  std::vector< std::string > list_colors;
  
  gchar *table =
    ColorTableManager::getInstance()->getAsciiTable(pal->getColorTableName().c_str());
  
  if (!table)
  {
    sii_message(Glib::ustring("Color table ") + pal->getColorTableName() +
		 " cannot be found");
    return false;
  }
  int cmax = strlen(table);
  GdkColor *gclist = (GdkColor *)g_malloc0(256 * sizeof(GdkColor));
  
  bool ok = true;
  
  int num_colors = 0;
    
  for(int ndx = 0; ndx < cmax;)
  {
    int start;
    int end;
    
    ok = sii_nab_region_from_text (table, ndx, &start, &end);
    
    if (!ok || end <= start)
      break;

    // Advance to first non-whiteout character.

    gchar line[256];
    
    strncpy(line, table + start, end - start);
    line[end-start] = '\0';
    ndx = end + 1;
    g_strstrip(line);
    
    if (strlen(line) == 0 || strstr(line, "colortable") ||
	strstr(line, "endtable") || strstr(line, "!"))
      continue;
    
    gchar *aa;
    
    if ((aa = strstr(line, "xcolor")) != 0)
    {
      // X color name

      aa += strlen("xcolor");
      g_strstrip(aa);
//      SLM_add_list_entry(lm, aa);
      list_colors.push_back(aa);
    }
    else if(strstr(line, "/") != 0)
    {
      // Funkier zeb color files

      float rnum, rden;
      float gnum, gden;
      float bnum, bden;
      
      sscanf(line, "%f/%f %f/%f %f/%f",
	     &rnum, &rden,
	     &gnum, &gden,
	     &bnum, &bden);
      int kk = num_colors++;
      GdkColor *gcolor = gclist +kk;
      gcolor->red =   (gushort)((rnum/rden)*(65536 - 1));
      gcolor->green = (gushort)((gnum/gden)*(65536 - 1));
      gcolor->blue =  (gushort)((bnum/bden)*(65536 - 1));

      // NOTE: gdk_color_alloc is deprecated and should not be used in
      // newly-written code. Use gdk_colormap_alloc_color() instead.

      gboolean ok2 = gdk_color_alloc (cmap, gcolor);
      gcolor->pixel = kk;

      if (!ok2)
	ok = false;
    }
    else
    {
      // Assume rgb vals between 0 and 1

      sscanf(line, "%f %f %f", &red, &green, &blue);
      int kk = num_colors++;
      GdkColor *gcolor = gclist +kk;
      gcolor->red =   (gushort) (red*(65536 - 1));
      gcolor->green = (gushort) (green*(65536 - 1));
      gcolor->blue =  (gushort) (blue*(65536 - 1));
      gboolean ok2 = gdk_color_alloc (cmap, gcolor);
      gcolor->pixel = kk;
      if (!ok2)
	ok = false;
    }		   
  }

  if (num_colors == 0)
  {
//    lm = wwptr->list_colors;
//    num_colors = lm->num_entries;
//    for (int kk = 0; kk < num_colors; kk++)
    for (size_t kk = 0; kk < list_colors.size(); kk++)
    {
//      gchar *aa = *(lm->list +kk);
//      GdkColor *gcolor = ColorManager::getInstance()->getColor(aa);
      GdkColor *gcolor =
	ColorManager::getInstance()->getColor(list_colors[kk].c_str());

      // NOTE: Won't we core dump if gcolor is NULL?  Shouldn't we handle
      // this differently?

      gclist[kk] = *gcolor;

      if (!gcolor)
	ok = false;
      ok2 = gdk_color_alloc (cmap, gcolor);
      gcolor->pixel = kk;
      
      if (!ok2)
	ok = false;
    }
  }

  if (!ok)
  {
    sii_message(Glib::ustring("Unable to allocate colors for ") +
		pal->getColorTableName());
    return ok;
  }

  wwptr->num_colors_in_table = num_colors;
  pal->setNumColors(num_colors);

  for (int kk = 0; kk < num_colors; kk++)
  {
    GdkColor *gcolor = gclist + kk;
    pal->setDataColor(kk, Gdk::Color(gcolor));

    guchar red = (guchar)(gcolor->red >> 8);
    guchar green = (guchar)(gcolor->green >> 8);
    guchar blue = (guchar)(gcolor->blue >> 8);
    
    pal->setColorTableRgb(kk, red, green, blue);

  }

  g_free (gclist);

  if (!solo_palette_color_numbers(frame_num))
    ok = false;

  if (ok)
    pd->orig_pal = pal;

  return ok;
}

/* c------------------------------------------------------------------------ */

int solo_palette_color_numbers(int frame_num)
{
  /* this routine assumes that all the color names have been checked
   */
  gint kk, ok=YES, nc, ct_ndx;
  gboolean ok2;
  WW_PTR wwptr;
  SiiPalette *pal;
  gchar mess[256];
  GdkColor *gcolor;
  GdkColormap *cmap;
  ParamData *pd = frame_configs[frame_num]->param_data;
  
  wwptr = solo_return_wwptr(frame_num);
  pal = pd->pal;
  nc = pd->pal->getNumColors();
  cmap = frame_configs[frame_num]->color_map;
  cmap = gtk_widget_get_colormap (frame_configs[frame_num]->frame);
  cmap = gdk_rgb_get_cmap ();
  mess[0] = '\0';

  gcolor = ColorManager::getInstance()->getColor(pal->getExceededColor().c_str());
  if (!gcolor)
  {
    strcat (mess, pal->getExceededColor().c_str());
    strcat (mess, " ");
    ok = NO;
  }
  else {
    pal->setFeatureColor(SiiPalette::FEATURE_EXCEEDED, Gdk::Color(gcolor));
    ok2 = gdk_color_alloc (cmap, gcolor);
  }

  gcolor =
    ColorManager::getInstance()->getColor(pal->getMissingDataColor().c_str());
  if (!gcolor)
  {
    strcat (mess, pal->getMissingDataColor().c_str());
    strcat (mess, " ");
    ok = NO;
  }
  else {
    pal->setFeatureColor(SiiPalette::FEATURE_MISSING, Gdk::Color(gcolor));
    ok2 = gdk_color_alloc (cmap, gcolor);
  }
  
  gcolor =
    ColorManager::getInstance()->getColor(pal->getBackgroundColor().c_str());
  if (!gcolor)
  {
    strcat (mess, pal->getBackgroundColor().c_str());
    strcat (mess, " ");
    ok = NO;
  }
  else {
    pal->setFeatureColor(SiiPalette::FEATURE_BACKGROUND, Gdk::Color(gcolor));
    ok2 = gdk_color_alloc (cmap, gcolor);
  }

  gcolor = ColorManager::getInstance()->getColor(pal->getEmphasisColor().c_str());
  if (!gcolor)
  {
    strcat(mess, pal->getEmphasisColor().c_str());
    strcat (mess, " ");
    ok = NO;
  }
  else {
    pal->setFeatureColor(SiiPalette::FEATURE_EMPHASIS, Gdk::Color(gcolor));
    ok2 = gdk_color_alloc (cmap, gcolor);
  }

  /*
   * the following two colors are drawn directly
   * as opposed to being part of the rgb image
   */

  gcolor = ColorManager::getInstance()->getColor(pal->getBoundaryColor().c_str());
  if (!gcolor)
  {
    strcat (mess, pal->getBoundaryColor().c_str());
    strcat (mess, " ");
    ok = NO;
  }
  else {
    pal->setFeatureColor(SiiPalette::FEATURE_BND, Gdk::Color(gcolor));
    ok2 = gdk_color_alloc (cmap, gcolor);
    wwptr->bnd_color_num = gcolor->pixel;
  }

  gcolor =
    ColorManager::getInstance()->getColor(pal->getAnnotationColor().c_str());
  if (!gcolor)
  {
    strcat(mess, pal->getAnnotationColor().c_str());
    strcat (mess, " ");
    ok = NO;
  }
  else {
    pal->setFeatureColor(SiiPalette::FEATURE_ANNOTATION, Gdk::Color(gcolor));
    ok2 = gdk_color_alloc (cmap, gcolor);
    wwptr->annotation_color_num = gcolor->pixel;
  }

  gcolor = ColorManager::getInstance()->getColor(pal->getBoundaryColor().c_str());
  if (!gcolor)
  {
    strcat (mess, pal->getBoundaryColor().c_str());
    strcat (mess, " ");
    ok = NO;
  }
  else {
    pal->setFeatureColor(SiiPalette::FEATURE_BND, Gdk::Color(gcolor));
    ok2 = gdk_color_alloc (cmap, gcolor);
  }

  gcolor = ColorManager::getInstance()->getColor(pal->getGridColor().c_str());
  if (!gcolor)
  {
    strcat (mess, pal->getGridColor().c_str());
    strcat (mess, " ");
    ok = NO;
  }
  else {
    pal->setFeatureColor(SiiPalette::FEATURE_RNG_AZ_GRID, Gdk::Color(gcolor));
    pal->setFeatureColor(SiiPalette::FEATURE_TIC_MARKS, Gdk::Color(gcolor));
    ok2 = gdk_color_alloc (cmap, gcolor);
    wwptr->grid_color_num = gcolor->pixel;
  }

  ct_ndx = pal->getNumColors() - 1;

  for (kk=0; kk < SiiPalette::MAX_FEATURE_COLORS; kk++)
  {
    if (pal->getFeatureColor(kk).get_pixel())
    {
      // Color is set

      gcolor = (GdkColor *)pal->getFeatureColor(kk).gobj();
      
      guchar red = (guchar)(gcolor->red >> 8);
      guchar green = (guchar)(gcolor->green >> 8);
      guchar blue = (guchar)(gcolor->blue >> 8);
      
      pal->setFeatureRgb(kk, red, green, blue);
      
      switch (kk)
      {
      case SiiPalette::FEATURE_BACKGROUND:
	wwptr->background_color_num = ++ct_ndx;
	pal->setColorTableRgb(ct_ndx, red, green, blue);
	break;
      case SiiPalette::FEATURE_EXCEEDED:
	wwptr->exceeded_color_num = ++ct_ndx;
	pal->setColorTableRgb(ct_ndx, red, green, blue);
	break;
      case SiiPalette::FEATURE_MISSING:
	wwptr->missing_data_color_num = ++ct_ndx;
	pal->setColorTableRgb(ct_ndx, red, green, blue);
	break;
      case SiiPalette::FEATURE_EMPHASIS:
	wwptr->emphasis_color_num = ++ct_ndx;
	pal->setColorTableRgb(ct_ndx, red, green, blue);
	break;
      };
    }
  }

  if (!ok) {
    sprintf (mess+strlen(mess), "not useful color name(s) in frame %d"
	     , frame_num);
    sii_message (mess);
  }

  return ok;
}
