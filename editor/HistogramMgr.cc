#include <se_utils.hh>
#include <sii_utils.hh>

#include "HistogramMgr.hh"

// Global variables

HistogramMgr *HistogramMgr::_instance =
  (HistogramMgr *)0;


/*********************************************************************
 * Constructor
 */

HistogramMgr::HistogramMgr() :
  _histoStream(0),
  _xyStream(0),
  _areaXval(0.0)
{
  // Set the instance pointer to point to this singleton instance

  _instance = this;
}


/*********************************************************************
 * Destructor
 */

HistogramMgr::~HistogramMgr()
{
}


/*********************************************************************
 * getInstance()
 */

HistogramMgr *HistogramMgr::getInstance()
{
  if (_instance == 0)
    new HistogramMgr();
  
  return _instance;
}


/*********************************************************************
 * histoOutput()
 */

void HistogramMgr::histoOutput()
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  se_print_strings(seds->h_output);

  if (seds->histo_output_key == SE_HST_COPY)
  {
    if (!_histoStream)
    {
      std::string histo_file_path = seds->histo_directory;
      if (histo_file_path[histo_file_path.size()-1] != '/')
	histo_file_path += "/";
      histo_file_path += seds->histo_filename;

      if (seds->histo_comment.size() > 0)
      {
	histo_file_path += ".";
	se_fix_comment(seds->histo_comment);
	histo_file_path += seds->histo_comment;
      }

      if ((_histoStream = fopen(histo_file_path.c_str(), "w")) == 0)
      {
	char message[1024];
	
	sprintf(message, "Could not open histogram file : %s\n",
		histo_file_path.c_str());
	sii_message(message);
	return;
      }
    }
    else
    {
      fprintf(_histoStream, "\n\n");
    }

    for (size_t i = 0; i < seds->h_output.size(); ++i)
      fprintf(_histoStream, "%s", seds->h_output[i].c_str());
  }

  if (seds->histo_flush && _histoStream)
    fflush(_histoStream);

  return;
}

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
