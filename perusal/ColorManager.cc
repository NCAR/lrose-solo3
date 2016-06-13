#include <iostream>
#include <string.h>

#include <se_utils.hh>
#include <sii_param_widgets.hh>

#include "ColorManager.hh"

// Global variables

ColorManager *ColorManager::_instance = (ColorManager *)0;


/*********************************************************************
 * Constructor
 */

ColorManager::ColorManager() :
  _colorsTree(0)
{
  // Initialize the colors hash table

  _colorsHash = g_hash_table_new(g_str_hash, g_str_equal);
  
  // Initialize the colors tree

  _colorsTree = g_tree_new((GCompareFunc)strcmp);

  // Initialize the color list

  _colorNames.clear();
  
  // Set the initial colors

  _setAllXNamedColors();
  
  // Set the singleton instance pointer

  _instance = this;
}


/*********************************************************************
 * Destructor
 */

ColorManager::~ColorManager()
{
}


/*********************************************************************
 * Inst()
 */

ColorManager *ColorManager::getInstance()
{
  if (_instance == 0)
    new ColorManager();
  
  return _instance;
}


/*********************************************************************
 * setColor()
 */

GdkColor *ColorManager::setColor(const gchar *color_name,
				 const gfloat f_red, const gfloat f_green,
				 const gfloat f_blue )
{
  GdkColor *gcolor = _normalizedRGB2Gdk(f_red, f_green, f_blue);

  // NOTE:  See if we need to make this copy.  It seems like it isn't needed
  // unless a copy of the pointer is saved in the tree or hash table in the
  // addColor() call below.  Either way, it seems like it should be deleted
  // inside the gcolor2 != 0 condition.

  gchar *new_name = (gchar *)g_malloc0(strlen(color_name) + 1);
  strcpy(new_name, color_name);

  GdkColor *gcolor2 = getColor(new_name);
  if (gcolor2 != 0)
    return gcolor;
  
  addColor(new_name, gcolor);

  return gcolor;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _normalizedRGB2Gdk()
 */

GdkColor *ColorManager::_normalizedRGB2Gdk(const gfloat f_red,
					   const gfloat f_green,
					   const gfloat f_blue)
{
  GdkColor *c = (GdkColor *)g_malloc(sizeof(GdkColor));

  gulong gul = 0xffff;
  c->pixel = 0;
  c->red = (gushort)(f_red * gul );
  c->green = (gushort)(f_green * gul );
  c->blue = (gushort)(f_blue * gul );

  // Fill in the pixel field of the color.  Shouldn't we also do something if
  // ok is false???
  //
  // NOTE: gdk_color_alloc has been deprecated since version 2.2 and should
  // not be used in newly-written code. Use gdk_colormap_alloc_color() instead.

  gdk_color_alloc(gdk_rgb_get_cmap(), c);

  return c;
}


/*********************************************************************
 * _setAllXNamedColors()
 */

void ColorManager::_setAllXNamedColors()
{
  const gchar * all_X_named_colors[] = {
    "mediumvioletred        0.780 0.082 0.522",
    "lightsteelblue         0.690 0.769 0.871",
    "paleturquoise4         0.400 0.545 0.545",
    "mediumpurple2          0.624 0.475 0.933",
    "lightskyblue3          0.553 0.714 0.804",
    "springgreen2           0.000 0.933 0.463",
    "lightsalmon            1.000 0.627 0.478",
    "yellowgreen            0.604 0.804 0.196",
    "darkorchid2            0.698 0.227 0.933",
    "aquamarine4            0.271 0.545 0.455",
    "slateblue4             0.278 0.235 0.545",
    "slateblue1             0.514 0.435 1.000",
    "olivedrab1             0.753 1.000 0.243",
    "goldenrod4             0.545 0.412 0.078",
    "goldenrod3             0.804 0.608 0.114",
    "cadetblue2             0.557 0.898 0.933",
    "burlywood1             1.000 0.827 0.608",
    "slategrey              0.439 0.502 0.565",
    "mistyrose              1.000 0.894 0.882",
    "limegreen              0.196 0.804 0.196",
    "lightcyan              0.878 1.000 1.000",
    "goldenrod              0.855 0.647 0.125",
    "gainsboro              0.863 0.863 0.863",
    "skyblue1               0.529 0.808 1.000",
    "honeydew               0.941 1.000 0.941",
    "yellow2                0.933 0.933 0.000",
    "tomato3                0.804 0.310 0.224",
    "skyblue                0.529 0.808 0.922",
    "purple4                0.333 0.102 0.545",
    "orange3                0.804 0.522 0.000",
    "bisque3                0.804 0.718 0.620",
    "bisque2                0.933 0.835 0.718",
    "grey34                 0.341 0.341 0.341",
    "gray99                 0.988 0.988 0.988",
    "gray63                 0.631 0.631 0.631",
    "gray44                 0.439 0.439 0.439",
    "gray37                 0.369 0.369 0.369",
    "gray33                 0.329 0.329 0.329",
    "gray26                 0.259 0.259 0.259",
    "azure1                 0.941 1.000 1.000",
    "snow4                  0.545 0.537 0.537",
    "peru                   0.804 0.522 0.247",
    "palevioletred          0.859 0.439 0.576",
    "lightgoldenrod4        0.545 0.506 0.298",
    "mediumseagreen         0.235 0.702 0.443",
    "lavenderblush          1.000 0.941 0.961",
    "mediumorchid2          0.820 0.373 0.933",
    "lightskyblue1          0.690 0.886 1.000",
    "darkslateblue          0.282 0.239 0.545",
    "midnightblue           0.098 0.098 0.439",
    "lightsalmon1           1.000 0.627 0.478",
    "lemonchiffon           1.000 0.980 0.804",
    "greenyellow            0.678 1.000 0.184",
    "lightsalmon            1.000 0.627 0.478",
    "lightcoral             0.941 0.502 0.502",
    "dodgerblue3            0.094 0.455 0.804",
    "darkorange4            0.545 0.271 0.000",
    "slateblue              0.416 0.353 0.804",
    "royalblue4             0.153 0.251 0.545",
    "orangered              1.000 0.271 0.000",
    "limegreen              0.196 0.804 0.196",
    "lightcyan              0.878 1.000 1.000",
    "darkviolet             0.580 0.000 0.827",
    "darksalmon             0.914 0.588 0.478",
    "darkorange             1.000 0.549 0.000",
    "cadetblue              0.373 0.620 0.627",
    "deeppink               1.000 0.078 0.576",
    "magenta2               0.933 0.000 0.933",
    "sienna4                0.545 0.278 0.149",
    "khaki2                 0.933 0.902 0.522",
    "grey75                 0.749 0.749 0.749",
    "grey74                 0.741 0.741 0.741",
    "grey73                 0.729 0.729 0.729",
    "grey69                 0.690 0.690 0.690",
    "grey68                 0.678 0.678 0.678",
    "grey35                 0.349 0.349 0.349",
    "grey13                 0.129 0.129 0.129",
    "gray90                 0.898 0.898 0.898",
    "gray81                 0.812 0.812 0.812",
    "gray55                 0.549 0.549 0.549",
    "gray51                 0.510 0.510 0.510",
    "gray31                 0.310 0.310 0.310",
    "snow2                  0.933 0.914 0.914",
    "pink3                  0.804 0.569 0.620",
    "grey7                  0.071 0.071 0.071",
    "gray1                  0.012 0.012 0.012",
    "red4                   0.545 0.000 0.000",
    "red3                   0.804 0.000 0.000",
    "tan                    0.824 0.706 0.549",
    "red                    1.000 0.000 0.000",
    "mediumvioletred        0.780 0.082 0.522",
    "lightslategrey         0.467 0.533 0.600",
    "lightskyblue           0.529 0.808 0.980",
    "lightsalmon4           0.545 0.341 0.259",
    "forestgreen            0.133 0.545 0.133",
    "dodgerblue4            0.063 0.306 0.545",
    "darkorchid             0.600 0.196 0.800",
    "rosybrown              0.737 0.561 0.561",
    "peachpuff3             0.804 0.686 0.584",
    "palegreen3             0.486 0.804 0.486",
    "orangered2             0.933 0.251 0.000",
    "lightcyan4             0.478 0.545 0.545",
    "indianred4             0.545 0.227 0.227",
    "indianred3             0.804 0.333 0.333",
    "navyblue               0.000 0.000 0.502",
    "dimgrey                0.412 0.412 0.412",
    "deeppink               1.000 0.078 0.576",
    "salmon4                0.545 0.298 0.224",
    "salmon3                0.804 0.439 0.329",
    "grey77                 0.769 0.769 0.769",
    "gray57                 0.569 0.569 0.569",
    "gray11                 0.110 0.110 0.110",
    "plum3                  0.804 0.588 0.804",
    "gray9                  0.090 0.090 0.090",
    "gray8                  0.078 0.078 0.078",
    "blue4                  0.000 0.000 0.545",
    "beige                  0.961 0.961 0.863",
    "lightgoldenrodyellow   0.980 0.980 0.824",
    "lavenderblush4         0.545 0.514 0.525",
    "darkturquoise          0.000 0.808 0.820",
    "darkturquoise          0.000 0.808 0.820",
    "darkslategrey          0.184 0.310 0.310",
    "lightsalmon3           0.804 0.506 0.384",
    "rosybrown4             0.545 0.412 0.412",
    "mistyrose              1.000 0.894 0.882",
    "seagreen2              0.306 0.933 0.580",
    "indianred              0.804 0.361 0.361",
    "deeppink1              1.000 0.078 0.576",
    "darkblue               0.000 0.000 0.545",
    "lavender               0.902 0.902 0.980",
    "oldlace                0.992 0.961 0.902",
    "grey78                 0.780 0.780 0.780",
    "grey54                 0.541 0.541 0.541",
    "grey45                 0.451 0.451 0.451",
    "grey21                 0.212 0.212 0.212",
    "gray97                 0.969 0.969 0.969",
    "gray96                 0.961 0.961 0.961",
    "gray95                 0.949 0.949 0.949",
    "gray88                 0.878 0.878 0.878",
    "gray87                 0.871 0.871 0.871",
    "gray86                 0.859 0.859 0.859",
    "gray70                 0.702 0.702 0.702",
    "gray38                 0.380 0.380 0.380",
    "gray12                 0.122 0.122 0.122",
    "linen                  0.980 0.941 0.902",
    "mediumturquoise        0.282 0.820 0.800",
    "darkslateblue          0.282 0.239 0.545",
    "lemonchiffon4          0.545 0.537 0.439",
    "darkseagreen1          0.757 1.000 0.757",
    "antiquewhite3          0.804 0.753 0.690",
    "mediumorchid           0.729 0.333 0.827",
    "springgreen            0.000 1.000 0.498",
    "turquoise4             0.000 0.525 0.545",
    "steelblue3             0.310 0.580 0.804",
    "mistyrose2             0.933 0.835 0.824",
    "lightcyan2             0.820 0.933 0.933",
    "indianred              0.804 0.361 0.361",
    "firebrick2             0.933 0.173 0.173",
    "royalblue              0.255 0.412 0.882",
    "cadetblue              0.373 0.620 0.627",
    "skyblue3               0.424 0.651 0.804",
    "yellow3                0.804 0.804 0.000",
    "salmon1                1.000 0.549 0.412",
    "orange4                0.545 0.353 0.000",
    "hotpink                1.000 0.412 0.706",
    "grey90                 0.898 0.898 0.898",
    "gray56                 0.561 0.561 0.561",
    "gray39                 0.388 0.388 0.388",
    "gray18                 0.180 0.180 0.180",
    "gray14                 0.141 0.141 0.141",
    "plum4                  0.545 0.400 0.545",
    "grey6                  0.059 0.059 0.059",
    "gray6                  0.059 0.059 0.059",
    "gold3                  0.804 0.678 0.000",
    "gold1                  1.000 0.843 0.000",
    "blue2                  0.000 0.000 0.933",
    "tan2                   0.933 0.604 0.286",
    "cyan                   0.000 1.000 1.000",
    "mediumspringgreen      0.000 0.980 0.604",
    "darkolivegreen2        0.737 0.933 0.408",
    "palegoldenrod          0.933 0.910 0.667",
    "lightsteelblue         0.690 0.769 0.871",
    "sandybrown             0.957 0.643 0.376",
    "papayawhip             1.000 0.937 0.835",
    "chartreuse3            0.400 0.804 0.000",
    "violetred4             0.545 0.133 0.322",
    "royalblue2             0.263 0.431 0.933",
    "royalblue1             0.282 0.463 1.000",
    "papayawhip             1.000 0.937 0.835",
    "mistyrose3             0.804 0.718 0.710",
    "lightcyan1             0.878 1.000 1.000",
    "aquamarine             0.498 1.000 0.831",
    "skyblue4               0.290 0.439 0.545",
    "hotpink4               0.545 0.227 0.384",
    "hotpink3               0.804 0.376 0.565",
    "hotpink2               0.933 0.416 0.655",
    "darkgrey               0.663 0.663 0.663",
    "dimgray                0.412 0.412 0.412",
    "tomato                 1.000 0.388 0.278",
    "grey66                 0.659 0.659 0.659",
    "grey65                 0.651 0.651 0.651",
    "grey64                 0.639 0.639 0.639",
    "grey33                 0.329 0.329 0.329",
    "grey27                 0.271 0.271 0.271",
    "gray76                 0.761 0.761 0.761",
    "gray69                 0.690 0.690 0.690",
    "gray68                 0.678 0.678 0.678",
    "grey0                  0.000 0.000 0.000",
    "azure                  0.941 1.000 1.000",
    "mediumspringgreen      0.000 0.980 0.604",
    "darkgoldenrod4         0.545 0.396 0.031",
    "darkgoldenrod3         0.804 0.584 0.047",
    "darkgoldenrod2         0.933 0.678 0.055",
    "darkgoldenrod          0.722 0.525 0.043",
    "saddlebrown            0.545 0.271 0.075",
    "lightsalmon2           0.933 0.584 0.447",
    "deepskyblue4           0.000 0.408 0.545",
    "deepskyblue3           0.000 0.604 0.804",
    "deepskyblue2           0.000 0.698 0.933",
    "deepskyblue            0.000 0.749 1.000",
    "darkorange1            1.000 0.498 0.000",
    "violetred3             0.804 0.196 0.471",
    "violetred2             0.933 0.227 0.549",
    "violetred1             1.000 0.243 0.588",
    "slateblue3             0.412 0.349 0.804",
    "slateblue2             0.478 0.404 0.933",
    "olivedrab              0.420 0.557 0.137",
    "indianred1             1.000 0.416 0.416",
    "firebrick1             1.000 0.188 0.188",
    "cadetblue4             0.325 0.525 0.545",
    "violetred              0.816 0.125 0.565",
    "rosybrown              0.737 0.561 0.561",
    "navyblue               0.000 0.000 0.502",
    "firebrick              0.698 0.133 0.133",
    "darkred                0.545 0.000 0.000",
    "grey100                1.000 1.000 1.000",
    "wheat4                 0.545 0.494 0.400",
    "grey79                 0.788 0.788 0.788",
    "grey76                 0.761 0.761 0.761",
    "grey61                 0.612 0.612 0.612",
    "gray93                 0.929 0.929 0.929",
    "gray84                 0.839 0.839 0.839",
    "gray65                 0.651 0.651 0.651",
    "gray36                 0.361 0.361 0.361",
    "gray32                 0.322 0.322 0.322",
    "gray13                 0.129 0.129 0.129",
    "gray10                 0.102 0.102 0.102",
    "azure3                 0.757 0.804 0.804",
    "snow1                  1.000 0.980 0.980",
    "tan1                   1.000 0.647 0.310",
    "lightslategray         0.467 0.533 0.600",
    "darkolivegreen1        0.792 1.000 0.439",
    "cornflowerblue         0.392 0.584 0.929",
    "blanchedalmond         1.000 0.922 0.804",
    "lavenderblush3         0.804 0.757 0.773",
    "lavenderblush2         0.933 0.878 0.898",
    "lavenderblush1         1.000 0.941 0.961",
    "darkolivegreen         0.333 0.420 0.184",
    "lavenderblush          1.000 0.941 0.961",
    "aquamarine2            0.463 0.933 0.776",
    "violetred              0.816 0.125 0.565",
    "olivedrab2             0.702 0.933 0.227",
    "mistyrose4             0.545 0.490 0.482",
    "mistyrose1             1.000 0.894 0.882",
    "lightcyan3             0.706 0.804 0.804",
    "lightcoral             0.941 0.502 0.502",
    "chartreuse             0.498 1.000 0.000",
    "peachpuff              1.000 0.855 0.725",
    "palegreen              0.596 0.984 0.596",
    "mintcream              0.961 1.000 0.980",
    "skyblue2               0.494 0.753 0.933",
    "moccasin               1.000 0.894 0.710",
    "tomato1                1.000 0.388 0.278",
    "orchid3                0.804 0.412 0.788",
    "maroon3                0.804 0.161 0.565",
    "salmon                 0.980 0.502 0.447",
    "grey81                 0.812 0.812 0.812",
    "grey62                 0.620 0.620 0.620",
    "grey39                 0.388 0.388 0.388",
    "grey38                 0.380 0.380 0.380",
    "grey37                 0.369 0.369 0.369",
    "gray92                 0.922 0.922 0.922",
    "gray83                 0.831 0.831 0.831",
    "gray66                 0.659 0.659 0.659",
    "gray54                 0.541 0.541 0.541",
    "gray50                 0.498 0.498 0.498",
    "gray30                 0.302 0.302 0.302",
    "gray19                 0.188 0.188 0.188",
    "gray15                 0.149 0.149 0.149",
    "azure4                 0.514 0.545 0.545",
    "grey3                  0.031 0.031 0.031",
    "tan3                   0.804 0.522 0.247",
    "pink                   1.000 0.753 0.796",
    "gray                   0.745 0.745 0.745",
    "blue                   0.000 0.000 1.000",
    "lightsteelblue2        0.737 0.824 0.933",
    "lightsteelblue1        0.792 0.882 1.000",
    "lightseagreen          0.125 0.698 0.667",
    "lightslategray         0.467 0.533 0.600",
    "lemonchiffon2          0.933 0.914 0.749",
    "springgreen1           0.000 1.000 0.498",
    "greenyellow            0.678 1.000 0.184",
    "chartreuse2            0.463 0.933 0.000",
    "slategrey              0.439 0.502 0.565",
    "royalblue3             0.227 0.373 0.804",
    "powderblue             0.690 0.878 0.902",
    "peachpuff2             0.933 0.796 0.678",
    "palegreen2             0.565 0.933 0.565",
    "mintcream              0.961 1.000 0.980",
    "slateblue              0.416 0.353 0.804",
    "seashell2              0.933 0.898 0.871",
    "deeppink2              0.933 0.071 0.537",
    "darkkhaki              0.741 0.718 0.420",
    "maroon4                0.545 0.110 0.384",
    "sienna                 0.627 0.322 0.176",
    "grey71                 0.710 0.710 0.710",
    "grey67                 0.671 0.671 0.671",
    "grey18                 0.180 0.180 0.180",
    "gray59                 0.588 0.588 0.588",
    "gray43                 0.431 0.431 0.431",
    "gray25                 0.251 0.251 0.251",
    "bisque                 1.000 0.894 0.769",
    "red1                   1.000 0.000 0.000",
    "mediumslateblue        0.482 0.408 0.933",
    "lightgoldenrod1        1.000 0.925 0.545",
    "lightgoldenrod         0.933 0.867 0.510",
    "paleturquoise3         0.588 0.804 0.804",
    "lightskyblue4          0.376 0.482 0.545",
    "springgreen            0.000 1.000 0.498",
    "lightyellow            1.000 1.000 0.878",
    "whitesmoke             0.961 0.961 0.961",
    "mediumblue             0.000 0.000 0.804",
    "ghostwhite             0.973 0.973 1.000",
    "steelblue4             0.212 0.392 0.545",
    "rosybrown3             0.804 0.608 0.608",
    "peachpuff1             1.000 0.855 0.725",
    "palegreen1             0.604 1.000 0.604",
    "blueviolet             0.541 0.169 0.886",
    "seashell4              0.545 0.525 0.510",
    "darkgray               0.663 0.663 0.663",
    "sienna3                0.804 0.408 0.224",
    "grey40                 0.400 0.400 0.400",
    "gray91                 0.910 0.910 0.910",
    "gray82                 0.820 0.820 0.820",
    "gray5                  0.051 0.051 0.051",
    "cyan2                  0.000 0.933 0.933",
    "cyan1                  0.000 1.000 1.000",
    "blue1                  0.000 0.000 1.000",
    "snow                   1.000 0.980 0.980",
    "lightgoldenrod2        0.933 0.863 0.510",
    "lightslateblue         0.518 0.439 1.000",
    "mediumorchid3          0.706 0.322 0.804",
    "darkseagreen4          0.412 0.545 0.412",
    "springgreen3           0.000 0.804 0.400",
    "forestgreen            0.133 0.545 0.133",
    "slategray4             0.424 0.482 0.545",
    "slategray3             0.624 0.714 0.804",
    "slategray2             0.725 0.827 0.933",
    "royalblue              0.255 0.412 0.882",
    "peachpuff4             0.545 0.467 0.396",
    "palegreen4             0.329 0.545 0.329",
    "palegreen              0.596 0.984 0.596",
    "orangered3             0.804 0.216 0.000",
    "goldenrod1             1.000 0.757 0.145",
    "ghostwhite             0.973 0.973 1.000",
    "firebrick4             0.545 0.102 0.102",
    "firebrick3             0.804 0.149 0.149",
    "cadetblue3             0.478 0.773 0.804",
    "slategray              0.439 0.502 0.565",
    "seashell3              0.804 0.773 0.749",
    "honeydew3              0.757 0.804 0.757",
    "cornsilk4              0.545 0.533 0.471",
    "cornsilk2              0.933 0.910 0.804",
    "purple1                0.608 0.188 1.000",
    "dimgrey                0.412 0.412 0.412",
    "darkred                0.545 0.000 0.000",
    "khaki1                 1.000 0.965 0.561",
    "ivory3                 0.804 0.804 0.757",
    "grey70                 0.702 0.702 0.702",
    "grey60                 0.600 0.600 0.600",
    "grey32                 0.322 0.322 0.322",
    "grey22                 0.220 0.220 0.220",
    "grey12                 0.122 0.122 0.122",
    "gray98                 0.980 0.980 0.980",
    "gray89                 0.890 0.890 0.890",
    "gray71                 0.710 0.710 0.710",
    "gray64                 0.639 0.639 0.639",
    "gray60                 0.600 0.600 0.600",
    "gray49                 0.490 0.490 0.490",
    "azure2                 0.878 0.933 0.933",
    "gray3                  0.031 0.031 0.031",
    "paleturquoise1         0.733 1.000 1.000",
    "mediumpurple1          0.671 0.510 1.000",
    "mediumpurple           0.576 0.439 0.859",
    "lemonchiffon1          1.000 0.980 0.804",
    "deepskyblue            0.000 0.749 1.000",
    "navajowhite3           0.804 0.702 0.545",
    "darkorchid1            0.749 0.243 1.000",
    "darkorange             1.000 0.549 0.000",
    "goldenrod2             0.933 0.706 0.133",
    "darkkhaki              0.741 0.718 0.420",
    "chocolate2             0.933 0.463 0.129",
    "burlywood2             0.933 0.773 0.569",
    "honeydew1              0.941 1.000 0.941",
    "darkgreen              0.000 0.392 0.000",
    "thistle3               0.804 0.710 0.804",
    "thistle2               0.933 0.824 0.933",
    "thistle1               1.000 0.882 1.000",
    "darkblue               0.000 0.000 0.545",
    "thistle                0.847 0.749 0.847",
    "maroon2                0.933 0.188 0.655",
    "maroon1                1.000 0.204 0.702",
    "grey53                 0.529 0.529 0.529",
    "grey44                 0.439 0.439 0.439",
    "grey25                 0.251 0.251 0.251",
    "gray74                 0.741 0.741 0.741",
    "gray45                 0.451 0.451 0.451",
    "gray41                 0.412 0.412 0.412",
    "gray35                 0.349 0.349 0.349",
    "gray27                 0.271 0.271 0.271",
    "gray23                 0.231 0.231 0.231",
    "gray16                 0.161 0.161 0.161",
    "brown4                 0.545 0.137 0.137",
    "wheat                  0.961 0.871 0.702",
    "coral                  1.000 0.498 0.314",
    "tan4                   0.545 0.353 0.169",
    "lightgoldenrodyellow   0.980 0.980 0.824",
    "lightslateblue         0.518 0.439 1.000",
    "darkolivegreen         0.333 0.420 0.184",
    "darkslategray          0.184 0.310 0.310",
    "palevioletred3         0.804 0.408 0.537",
    "mediumpurple4          0.365 0.278 0.545",
    "mediumpurple3          0.537 0.408 0.804",
    "saddlebrown            0.545 0.271 0.075",
    "powderblue             0.690 0.878 0.902",
    "darkorchid4            0.408 0.133 0.545",
    "darkorchid3            0.604 0.196 0.804",
    "peachpuff              1.000 0.855 0.725",
    "olivedrab4             0.412 0.545 0.133",
    "lightblue4             0.408 0.514 0.545",
    "lightpink              1.000 0.714 0.757",
    "lightgray              0.827 0.827 0.827",
    "honeydew2              0.878 0.933 0.878",
    "cornsilk1              1.000 0.973 0.863",
    "oldlace                0.992 0.961 0.902",
    "sienna1                1.000 0.510 0.278",
    "bisque4                0.545 0.490 0.420",
    "orchid                 0.855 0.439 0.839",
    "khaki3                 0.804 0.776 0.451",
    "grey84                 0.839 0.839 0.839",
    "grey83                 0.831 0.831 0.831",
    "grey82                 0.820 0.820 0.820",
    "grey72                 0.722 0.722 0.722",
    "grey52                 0.522 0.522 0.522",
    "grey43                 0.431 0.431 0.431",
    "grey26                 0.259 0.259 0.259",
    "grey14                 0.141 0.141 0.141",
    "grey10                 0.102 0.102 0.102",
    "gray75                 0.749 0.749 0.749",
    "gray53                 0.529 0.529 0.529",
    "gray21                 0.212 0.212 0.212",
    "gray20                 0.200 0.200 0.200",
    "brown3                 0.804 0.200 0.200",
    "grey8                  0.078 0.078 0.078",
    "red2                   0.933 0.000 0.000",
    "navy                   0.000 0.000 0.502",
    "grey                   0.745 0.745 0.745",
    "gold                   1.000 0.843 0.000",
    "mediumaquamarine       0.400 0.804 0.667",
    "lightgoldenrod         0.933 0.867 0.510",
    "darkslategray4         0.322 0.545 0.545",
    "darkseagreen3          0.608 0.804 0.608",
    "darkseagreen2          0.706 0.933 0.706",
    "antiquewhite4          0.545 0.514 0.471",
    "antiquewhite           0.980 0.922 0.843",
    "springgreen4           0.000 0.545 0.271",
    "lightyellow4           0.545 0.545 0.478",
    "floralwhite            1.000 0.980 0.941",
    "aquamarine1            0.498 1.000 0.831",
    "turquoise3             0.000 0.773 0.804",
    "steelblue2             0.361 0.675 0.933",
    "rosybrown2             0.933 0.706 0.706",
    "lightpink              1.000 0.714 0.757",
    "lightgray              0.827 0.827 0.827",
    "indianred2             0.933 0.388 0.388",
    "dodgerblue             0.118 0.565 1.000",
    "darkgreen              0.000 0.392 0.000",
    "seagreen1              0.329 1.000 0.624",
    "deeppink4              0.545 0.039 0.314",
    "aliceblue              0.941 0.973 1.000",
    "magenta1               1.000 0.000 1.000",
    "hotpink                1.000 0.412 0.706",
    "sienna2                0.933 0.475 0.259",
    "orchid1                1.000 0.514 0.980",
    "gray100                1.000 1.000 1.000",
    "grey97                 0.969 0.969 0.969",
    "grey94                 0.941 0.941 0.941",
    "grey87                 0.871 0.871 0.871",
    "grey86                 0.859 0.859 0.859",
    "grey51                 0.510 0.510 0.510",
    "grey42                 0.420 0.420 0.420",
    "grey19                 0.188 0.188 0.188",
    "gray94                 0.941 0.941 0.941",
    "gray85                 0.851 0.851 0.851",
    "gray61                 0.612 0.612 0.612",
    "brown2                 0.933 0.231 0.231",
    "khaki                  0.941 0.902 0.549",
    "grey1                  0.012 0.012 0.012",
    "gold4                  0.545 0.459 0.000",
    "mediumslateblue        0.482 0.408 0.933",
    "mediumseagreen         0.235 0.702 0.443",
    "darkslategrey          0.184 0.310 0.310",
    "paleturquoise          0.686 0.933 0.933",
    "paleturquoise          0.686 0.933 0.933",
    "mediumorchid4          0.478 0.216 0.545",
    "antiquewhite2          0.933 0.875 0.800",
    "lightyellow2           0.933 0.933 0.820",
    "lightgreen             0.565 0.933 0.565",
    "darkviolet             0.580 0.000 0.827",
    "darksalmon             0.914 0.588 0.478",
    "chartreuse1            0.498 1.000 0.000",
    "turquoise1             0.000 0.961 1.000",
    "sandybrown             0.957 0.643 0.376",
    "orangered1             1.000 0.271 0.000",
    "lightpink1             1.000 0.682 0.725",
    "lightblue2             0.698 0.875 0.933",
    "lightblue1             0.749 0.937 1.000",
    "lightgrey              0.827 0.827 0.827",
    "seagreen4              0.180 0.545 0.341",
    "seagreen3              0.263 0.804 0.502",
    "lightblue              0.678 0.847 0.902",
    "deeppink3              0.804 0.063 0.463",
    "darkgrey               0.663 0.663 0.663",
    "darkcyan               0.000 0.545 0.545",
    "burlywood              0.871 0.722 0.529",
    "seashell               1.000 0.961 0.933",
    "hotpink1               1.000 0.431 0.706",
    "dimgray                0.412 0.412 0.412",
    "darkcyan               0.000 0.545 0.545",
    "yellow4                0.545 0.545 0.000",
    "yellow                 1.000 1.000 0.000",
    "purple                 0.627 0.125 0.941",
    "orange                 1.000 0.647 0.000",
    "ivory4                 0.545 0.545 0.514",
    "grey99                 0.988 0.988 0.988",
    "grey89                 0.890 0.890 0.890",
    "grey63                 0.631 0.631 0.631",
    "grey58                 0.580 0.580 0.580",
    "grey49                 0.490 0.490 0.490",
    "grey31                 0.310 0.310 0.310",
    "grey24                 0.239 0.239 0.239",
    "grey20                 0.200 0.200 0.200",
    "green4                 0.000 0.545 0.000",
    "green1                 0.000 1.000 0.000",
    "gray73                 0.729 0.729 0.729",
    "gray67                 0.671 0.671 0.671",
    "coral3                 0.804 0.357 0.271",
    "coral2                 0.933 0.416 0.314",
    "plum2                  0.933 0.682 0.933",
    "pink4                  0.545 0.388 0.424",
    "ivory                  1.000 1.000 0.941",
    "gray4                  0.039 0.039 0.039",
    "gray2                  0.020 0.020 0.020",
    "gold2                  0.933 0.788 0.000",
    "mediumaquamarine       0.400 0.804 0.667",
    "lightslategrey         0.467 0.533 0.600",
    "lightgoldenrod3        0.804 0.745 0.439",
    "darkolivegreen3        0.635 0.804 0.353",
    "darkgoldenrod1         1.000 0.725 0.059",
    "darkgoldenrod          0.722 0.525 0.043",
    "mediumorchid           0.729 0.333 0.827",
    "lemonchiffon           1.000 0.980 0.804",
    "navajowhite4           0.545 0.475 0.369",
    "deepskyblue1           0.000 0.749 1.000",
    "lightyellow            1.000 1.000 0.878",
    "floralwhite            1.000 0.980 0.941",
    "dodgerblue             0.118 0.565 1.000",
    "mediumblue             0.000 0.000 0.804",
    "lightgreen             0.565 0.933 0.565",
    "chocolate4             0.545 0.271 0.075",
    "chocolate3             0.804 0.400 0.114",
    "burlywood4             0.545 0.451 0.333",
    "turquoise              0.251 0.878 0.816",
    "steelblue              0.275 0.510 0.706",
    "seagreen               0.180 0.545 0.341",
    "lawngreen              0.486 0.988 0.000",
    "honeydew4              0.514 0.545 0.514",
    "darkgray               0.663 0.663 0.663",
    "seagreen               0.180 0.545 0.341",
    "orchid4                0.545 0.278 0.537",
    "wheat1                 1.000 0.906 0.729",
    "violet                 0.933 0.510 0.933",
    "ivory1                 1.000 1.000 0.941",
    "grey88                 0.878 0.878 0.878",
    "grey85                 0.851 0.851 0.851",
    "grey57                 0.569 0.569 0.569",
    "grey56                 0.561 0.561 0.561",
    "grey55                 0.549 0.549 0.549",
    "grey48                 0.478 0.478 0.478",
    "grey47                 0.471 0.471 0.471",
    "grey46                 0.459 0.459 0.459",
    "grey30                 0.302 0.302 0.302",
    "grey17                 0.169 0.169 0.169",
    "gray47                 0.471 0.471 0.471",
    "gray29                 0.290 0.290 0.290",
    "pink2                  0.933 0.663 0.722",
    "grey5                  0.051 0.051 0.051",
    "grey4                  0.039 0.039 0.039",
    "green                  0.000 1.000 0.000",
    "gray0                  0.000 0.000 0.000",
    "brown                  0.647 0.165 0.165",
    "lightsteelblue4        0.431 0.482 0.545",
    "darkolivegreen4        0.431 0.545 0.239",
    "palevioletred4         0.545 0.278 0.365",
    "lightskyblue           0.529 0.808 0.980",
    "darkslategray3         0.475 0.804 0.804",
    "darkslategray2         0.553 0.933 0.933",
    "darkslategray1         0.592 1.000 1.000",
    "blanchedalmond         1.000 0.922 0.804",
    "palegoldenrod          0.933 0.910 0.667",
    "midnightblue           0.098 0.098 0.439",
    "lightseagreen          0.125 0.698 0.667",
    "lemonchiffon3          0.804 0.788 0.647",
    "darkslategray          0.184 0.310 0.310",
    "yellowgreen            0.604 0.804 0.196",
    "darkseagreen           0.561 0.737 0.561",
    "antiquewhite           0.980 0.922 0.843",
    "darkorange2            0.933 0.463 0.000",
    "chartreuse4            0.271 0.545 0.000",
    "steelblue              0.275 0.510 0.706",
    "rosybrown1             1.000 0.757 0.757",
    "olivedrab3             0.604 0.804 0.196",
    "lightpink2             0.933 0.635 0.678",
    "orangered              1.000 0.271 0.000",
    "thistle4               0.545 0.482 0.545",
    "skyblue                0.529 0.808 0.922",
    "cornsilk               1.000 0.973 0.863",
    "salmon2                0.933 0.510 0.384",
    "orchid2                0.933 0.478 0.914",
    "ivory2                 0.933 0.933 0.878",
    "grey93                 0.929 0.929 0.929",
    "grey92                 0.922 0.922 0.922",
    "grey91                 0.910 0.910 0.910",
    "grey36                 0.361 0.361 0.361",
    "grey29                 0.290 0.290 0.290",
    "grey28                 0.278 0.278 0.278",
    "grey16                 0.161 0.161 0.161",
    "gray79                 0.788 0.788 0.788",
    "gray78                 0.780 0.780 0.780",
    "gray77                 0.769 0.769 0.769",
    "gray48                 0.478 0.478 0.478",
    "gray17                 0.169 0.169 0.169",
    "coral4                 0.545 0.243 0.184",
    "coral1                 1.000 0.447 0.337",
    "plum1                  1.000 0.733 1.000",
    "pink1                  1.000 0.710 0.773",
    "grey9                  0.090 0.090 0.090",
    "grey2                  0.020 0.020 0.020",
    "gray7                  0.071 0.071 0.071",
    "cyan4                  0.000 0.545 0.545",
    "blue3                  0.000 0.000 0.804",
    "plum                   0.867 0.627 0.867",
    "cornflowerblue         0.392 0.584 0.929",
    "lightskyblue2          0.643 0.827 0.933",
    "antiquewhite1          1.000 0.937 0.859",
    "navajowhite2           0.933 0.812 0.631",
    "navajowhite1           1.000 0.871 0.678",
    "lightyellow3           0.804 0.804 0.706",
    "darkmagenta            0.545 0.000 0.545",
    "navajowhite            1.000 0.871 0.678",
    "darkorange3            0.804 0.400 0.000",
    "whitesmoke             0.961 0.961 0.961",
    "turquoise2             0.000 0.898 0.933",
    "steelblue1             0.388 0.722 1.000",
    "lightpink4             0.545 0.373 0.396",
    "lightblue3             0.604 0.753 0.804",
    "lawngreen              0.486 0.988 0.000",
    "chocolate1             1.000 0.498 0.141",
    "aliceblue              0.941 0.973 1.000",
    "olivedrab              0.420 0.557 0.137",
    "lightgrey              0.827 0.827 0.827",
    "chocolate              0.824 0.412 0.118",
    "magenta4               0.545 0.000 0.545",
    "magenta3               0.804 0.000 0.804",
    "yellow1                1.000 1.000 0.000",
    "purple3                0.490 0.149 0.804",
    "purple2                0.569 0.173 0.933",
    "orange2                0.933 0.604 0.000",
    "orange1                1.000 0.647 0.000",
    "magenta                1.000 0.000 1.000",
    "bisque1                1.000 0.894 0.769",
    "wheat2                 0.933 0.847 0.682",
    "maroon                 0.690 0.188 0.376",
    "khaki4                 0.545 0.525 0.306",
    "grey96                 0.961 0.961 0.961",
    "grey95                 0.949 0.949 0.949",
    "grey80                 0.800 0.800 0.800",
    "grey50                 0.498 0.498 0.498",
    "grey41                 0.412 0.412 0.412",
    "grey15                 0.149 0.149 0.149",
    "grey11                 0.110 0.110 0.110",
    "gray80                 0.800 0.800 0.800",
    "gray58                 0.580 0.580 0.580",
    "gray40                 0.400 0.400 0.400",
    "gray34                 0.341 0.341 0.341",
    "gray22                 0.220 0.220 0.220",
    "brown1                 1.000 0.251 0.251",
    "snow3                  0.804 0.788 0.788",
    "mediumturquoise        0.282 0.820 0.800",
    "lightsteelblue3        0.635 0.710 0.804",
    "palevioletred2         0.933 0.475 0.624",
    "palevioletred1         1.000 0.510 0.671",
    "paleturquoise2         0.682 0.933 0.933",
    "darkseagreen           0.561 0.737 0.561",
    "palevioletred          0.859 0.439 0.576",
    "mediumorchid1          0.878 0.400 1.000",
    "navajowhite            1.000 0.871 0.678",
    "mediumpurple           0.576 0.439 0.859",
    "lightyellow1           1.000 1.000 0.878",
    "dodgerblue2            0.110 0.525 0.933",
    "dodgerblue1            0.118 0.565 1.000",
    "darkmagenta            0.545 0.000 0.545",
    "blueviolet             0.541 0.169 0.886",
    "aquamarine3            0.400 0.804 0.667",
    "slategray1             0.776 0.886 1.000",
    "slategray              0.439 0.502 0.565",
    "orangered4             0.545 0.145 0.000",
    "lightpink3             0.804 0.549 0.584",
    "lightblue              0.678 0.847 0.902",
    "darkorchid             0.600 0.196 0.800",
    "cadetblue1             0.596 0.961 1.000",
    "burlywood3             0.804 0.667 0.490",
    "seashell1              1.000 0.961 0.933",
    "cornsilk3              0.804 0.784 0.694",
    "tomato4                0.545 0.212 0.149",
    "tomato2                0.933 0.361 0.259",
    "wheat3                 0.804 0.729 0.588",
    "grey98                 0.980 0.980 0.980",
    "grey59                 0.588 0.588 0.588",
    "grey23                 0.231 0.231 0.231",
    "green3                 0.000 0.804 0.000",
    "green2                 0.000 0.933 0.000",
    "gray72                 0.722 0.722 0.722",
    "gray62                 0.620 0.620 0.620",
    "gray52                 0.522 0.522 0.522",
    "gray46                 0.459 0.459 0.459",
    "gray42                 0.420 0.420 0.420",
    "gray28                 0.278 0.278 0.278",
    "gray24                 0.239 0.239 0.239",
    "white                  1.000 1.000 1.000",
    "cyan3                  0.000 0.804 0.804",
    "black                  0.006 0.006 0.006",
  };

  int nn = sizeof (all_X_named_colors)/sizeof (char *);
   
  const gchar **cptrs = all_X_named_colors;
  int count = 0;
  
  for ( ; nn--; cptrs++)
  {
    std::string color_name = *cptrs;
    std::vector< std::string > tokens;
    tokenize(color_name, tokens);
    if (tokens.size() < 4)
      break;

    // Don't use grey (I don't know why)

    if (color_name.find("grey") == std::string::npos)
    {
      count++;

      float r, g, b;
      
      sscanf(tokens[1].c_str(), "%f", &r);
      sscanf(tokens[2].c_str(), "%f", &g);
      sscanf(tokens[3].c_str(), "%f", &b);
	
      setColor(tokens[0].c_str(), r, g, b);
    }
  }
}


/*********************************************************************
 * _setDefaultColors()
 */

void ColorManager::_setDefaultColors()
{
  gfloat norm = 255;
  GdkColor *gcolor;
  
  gcolor = setColor("red", 1.0, 0.0, 0.0);
  gcolor = _toneDownColor( "red", .1 );
  gcolor = _toneDownColor( "red", .2 );
  gcolor = _toneDownColor( "red", .3 );

  gcolor = setColor("green", 0.0, 1.0, 0.0);
  gcolor = _toneDownColor( "green", .1 );
  gcolor = _toneDownColor( "green", .2 );
  gcolor = _toneDownColor( "green", .3 );
  gcolor = _toneDownColor( "green", .4 );
  gcolor = _toneDownColor( "green", .5 );

  gcolor = setColor("blue", 0.0, 0.0, 1.0);
  gcolor = _toneDownColor( "blue", .1 );
  gcolor = _toneDownColor( "blue", .2 );
  gcolor = _toneDownColor( "blue", .3 );
  gcolor = _toneDownColor( "blue", .4 );

  gcolor = setColor("yellow", 1.0, 1.0, 0.0);
  gcolor = _toneDownColor( "yellow", .1 );
  gcolor = _toneDownColor( "yellow", .2 );

  gcolor = setColor("cyan", 0.0, 1.0, 1.0);
  // Default grid color
  gcolor = _toneDownColor( "cyan", .1 );
  gcolor = _toneDownColor( "cyan", .25 );
  gcolor = _toneDownColor( "cyan", .5 );

  gcolor = setColor("magenta", 1.0, 0.0, 1.0);
  gcolor = _toneDownColor( "magenta", .1 );
  gcolor = _toneDownColor( "magenta", .25 );

  gcolor = setColor("black", 0.006, 0.006, 0.006);
  gcolor = setColor("white", 1.0, 1.0, 1.0);
  gcolor = _toneDownColor( "white", .06 );
  gcolor = _toneDownColor( "white", .1 );
  gcolor = _toneDownColor( "white", .25 );
  gcolor = _toneDownColor( "white", .44 );
  gcolor = _toneDownColor( "white", .66);

  // NOTE:  Does the second color get added if it has the same name???

  gcolor = setColor("chocolate", 0.67, 0.25, 0.25);
  gcolor = setColor("chocolate", 0.64, 0.24, 0.24);
  gcolor = setColor("coffee", 0.3, 0.1, 0.1);
  gcolor = setColor("orange", 1.0, 0.64, 0.24);

  // Background
  gcolor = setColor("midnightblue", 25/norm, 25/norm, 112/norm);
  // Missing data
  gcolor = setColor("darkslateblue", 71/norm, 61/norm, 139/norm);

  gcolor = setColor("gold", 255/norm, 215/norm, 0.0);
  gcolor = setColor("thistle", 216/norm, 191/norm, 216/norm);
  gcolor = setColor("wheat", 245/norm, 222/norm, 179/norm);
  gcolor = setColor("hotpink", 1.0, 0.4, 0.7);
}


/*********************************************************************
 * _toneDownColor()
 */

GdkColor *ColorManager::_toneDownColor(const gchar *name, const gfloat atten)
{
  GdkColor *old = getColor(name);

  gchar new_name[80];
  sprintf(new_name, "%s%02d", name, (gint)(atten * 100.0 +0.5));

  gulong gul = 0xffff;
  gfloat value = 1.0 - atten;

  gfloat r = (gfloat)old->red/gul * value;
  gfloat g = (gfloat)old->green/gul * value;
  gfloat b = (gfloat)old->blue/gul * value;

  return setColor(new_name, r, g, b);
}
