/* 	$Id: dorade_share.cc 487 2011-11-16 14:22:57Z rehak $	 */

// NOTE: This is a temporary copy of the solo_list_mgmt routines.  I've copied
// them here since I'm in the process of encapsulating the reliance on the
// translate functions into the DataManager class to get ready for replacing
// translate with Radx.  These routines will go away when I replace the
// solo_list_mgmt objects with vector< string > objects, but I don't want to
// add that complication to the vast changes I'm making right now.

#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <se_utils.hh>
#include <solo_list_mgmt.hh>
#include <solo2.hh>

/* c------------------------------------------------------------------------ */

// NOTE: This function is only used locally, below

int _getreply (char *s, int lim)
{
    int c, ii=0;
    char *a=s;

    *a = '\0';

    for(;;) {
	if(lim-- <= 0)
	      break;

	c = getc(stdin);

	if(c == '\n')
	      break;

	if(c == EOF)
	      break;

	*a++ = c;
	ii++;
    }

    *a = '\0';
    return(ii);
}

/* c------------------------------------------------------------------------ */

// NOTE: This function is only used locally, below

int _okint ( /* see if string is an integer */
    char s[],
    int n,
    int *ival
)
{
    int sign, i;
    
    *ival = 0;

    for( i=0; i < n && s[i] == ' '; i++ )  /* skip leading blanks */
    if( i == n )
	 return(1);

    for( ; --n >= i && s[n] == ' ';); /* skip trailing blanks */
    if( i > n )
	 return(1);
    
    sign = 1;
    if( s[i] == '+' )
	 i++;
    else if( s[i] == '-' ){
	sign = -1;
	i++;
    }
    for(; i <= n; i++ )
	 if( isdigit( s[i] ))
	      *ival = *ival*10 + s[i]-'0';
	 else {
	     *ival = 0;
	     return(0);
	 }
    *ival *= sign;
    return(1);
}

/* c------------------------------------------------------------------------ */

// NOTE: This function is only used locally, below

int _okreal ( /* determine if string is real number */
    char s[],
    int n,
    float *val
)
{
    int d, i, ival;
    float frak=1.0;

    *val = 0;
    for( d=0; d < n && s[d] != '.'; d++ )
	 ;
    i = _okint( s, d, &ival ); /* get the whole part */
    *val = (float) ival;
    
    if( d == n ) {
	if( !i )
	     return(0);
	return(1);
    }
    i = n-d-1;
    if( !_okint( s+d+1, i, &ival)) /* get the fraction */
	 return(0);

    while( i-- > 0 )
	 frak *= 0.1;

    for( i=0; i < n && s[i] != '-'; i++ )
	 ;
    if( i == n ) /* no minus sign */
	 *val += (float) ival * frak;
    else
	 *val -= (float) ival * frak;

    return(1);
}

/* c------------------------------------------------------------------------ */

// NOTE: This function is only used locally, below

int _oketype ( /* determine if string is real number */
    char s[],
    int n,
    float *val
)
{
    int d, i, ival;
    float tenx=1.0;

    *val = 0;

    for( d=0; d < n && s[d] != 'e' && s[d] != 'E'; d++ )
	 ;
    i = _okreal( s, d, val ); /* get the real part */
    
    if( d == n ) {
	if( !i )
	     return(0);
	return(1);
    }
    if( !_okint( s+(d+1), n-(d+1), &ival)) /* get the power part */
	 return(0);

    if( ival < 0 )
	 while( ival++ < 0 )
	      tenx *= 0.1;
    else
	 while( ival-- > 0 )
	      tenx *= 10.0;

    *val *= tenx;

 /* this doesn't work!
    *val = (double)(*val) * rpow( 10, ival ); 
  */
    return(1);
}

/* c------------------------------------------------------------------------ */

// NOTE: This function is only used locally, below

int _cdcode ( /* try to decode the string */
    char *s,
    int n,
    int *ival,
    float *rval
)
{
  if( _okint( s, n, ival ))
	 {
	     *rval = (float)(*ival);
	     return(1);
	 }
    else if( _oketype( s, n, rval ))
	 {
	     if( fabs((double)(*rval)) <= 1000000000. )
		  *ival = (int)(*rval +0.5); 
	     else
		  *ival = 0;
	     return(2);
	 }
    return(0);
}

/* c------------------------------------------------------------------------ */

void SLM_dump_list (struct solo_list_mgmt *slm)
{
    char **ptr;
    int nl;

    if(!slm)
	  return;

    if(!(ptr = slm->list))
	  return;

    nl = slm->num_entries;

    for(; nl-- > 0;) {
       solo_message(*ptr++);
       solo_message("\n");
    }
}

/* c------------------------------------------------------------------------ */

void SLM_print_list (struct solo_list_mgmt *slm)
{
    char **pt, **ptrs=slm->list;
    int ll=0, mm, nl, last_mm;
    int nn, ival;
    float val;
    char str[128];
    int max = 256;

    if(!slm)
	  return;
    nl = slm->num_entries;

    if(nl < max) max = nl;

    last_mm = mm = max;

    for(;;) {
	for(pt=ptrs+ll; mm--; pt++) {
	    printf("%s\n", *pt);
	}
	// NOTE: Do we really want to stop and query the user at this point???
	// If not, get rid of the _getreply() function above.
	printf("Hit <return> to continue ");
	nn = _getreply(str, sizeof(str));
	if(_cdcode(str, nn, &ival, &val) != 1 || ival == 1) {
	    return;
	}
	mm = max;
	if(ival > 0) {
	    if(ll +ival +mm > nl) {
		ll = nl -mm;
	    }
	    else 
		  ll += ival;
	}
	else if( ival < 0) {
	    ll = ll +ival < 0 ? 0 : ll + ival;
	}
	else {
	    if(ll+last_mm == nl)
		  return;
	    ll += last_mm;
	    if(ll +mm > nl) {
		mm = nl -ll;
	    }
	}
	last_mm = mm;
    }
}

/* c------------------------------------------------------------------------ */

void SLM_add_list_entry (struct solo_list_mgmt *which, const std::string &entry)
{
    int ii;
    char *a, *c;

    if(!which)
	  return;

    int len = entry.size();

    if(which->num_entries >= which->max_entries) {
	which->max_entries += 30;
	if(which->list) {
	    which->list = (char **)realloc
		  (which->list, which->max_entries*sizeof(char *));
	}
	else {
	    which->list = (char **)malloc(which->max_entries*sizeof(char *));
	}
	for(ii=which->num_entries; ii < which->max_entries; ii++) {
	    *(which->list+ii) = a = (char *)malloc(which->sizeof_entries+1);
	    *a = '\0';
	}
    }
    len = len < which->sizeof_entries ? len : which->sizeof_entries;
    c = *(which->list+which->num_entries++);

    for (int i = 0; i < len; ++i, ++c)
      *c = entry[i];
    
    *c = '\0';
}

/* c------------------------------------------------------------------------ */

char *SLM_list_entry (struct solo_list_mgmt *which, int entry_num)
{
    char *c;

    if(!which || entry_num >= which->num_entries || entry_num < 0)
	  return(NULL);

    c = *(which->list+entry_num);
    return(c);
}

/* c------------------------------------------------------------------------ */

void SLM_list_remove_dups (struct solo_list_mgmt *slm)
{
    /* remove duplicate strings from the list
     */
    int ii, nd;

    if(!slm || slm->num_entries <= 0)
	  return;

    nd = slm->num_entries-1;

    for(ii=0; ii < slm->num_entries-1; ii++) {
	for(; ii < slm->num_entries-1 &&
	    !strcmp(*(slm->list +ii), *(slm->list +ii +1));) {
	    SLM_list_remove_entry(slm, ii+1, ii+1);
	}
    }
    return;
}

/* c------------------------------------------------------------------------ */

void SLM_list_remove_entry (struct solo_list_mgmt *slm, int ii, int jj)
{
    /* remove a span of entries where ii and jj are entry numbers
     */
    int kk, nn, nd;
    char *keep;

    if(!slm || slm->num_entries <= 0 || ii < 0 || ii >= slm->num_entries)
	  return;
    nd = slm->num_entries-1;

    if(jj >= slm->num_entries)
	  jj = nd;
    else if(jj < ii)
	  jj = ii;

    nn = jj - ii + 1;

    for(; nn--; ) {		/* keep shifting the entries up */
	keep = *(slm->list +ii);
	for(kk=ii; kk < nd; kk++) {
	    *(slm->list +kk) = *(slm->list +kk +1);
	}
	strcpy(keep, " ");
	*(slm->list +(nd--)) = keep;
    }
    slm->num_entries = nd +1;
    return;
}

/* c------------------------------------------------------------------------ */

//void SLM_list_sort_file_names(struct solo_list_mgmt *slm)
//{
//  // If the list is empty, don't do anything
//
//  if (!slm || slm->num_entries <= 0)
//    return;
//
//  // Get a pointer to the data manager
//
//  DataManager *data_mgr = DataManager::getInstance();
//    
//  // Loop through the list, sorting as we go
//
//  for (int ii = 0; ii < slm->num_entries - 1; ii++)
//  {
//    for (int jj = ii + 1; jj < slm->num_entries; jj++)
//    {
//      if (data_mgr->getTimeFromFileName(*(slm->list+jj)).getTimeStamp() <
//	  data_mgr->getTimeFromFileName(*(slm->list+ii)).getTimeStamp())
//      {
//	char *keep = *(slm->list+ii);
//	*(slm->list+ii) = *(slm->list+jj);
//	*(slm->list+jj) = keep;
//      }
//    }
//  }
//
//  return;
//}

/* c------------------------------------------------------------------------ */

struct solo_list_mgmt *SLM_malloc_list_mgmt (int sizeof_entries)
{
    struct solo_list_mgmt *slm;

    slm = (struct solo_list_mgmt *)malloc(sizeof(struct solo_list_mgmt));
    memset(slm, 0, sizeof(struct solo_list_mgmt));
    slm->sizeof_entries = sizeof_entries;
    return(slm);
}

/* c------------------------------------------------------------------------ */

char *SLM_modify_list_entry(struct solo_list_mgmt *which, 
			    const char *entry, const int len,
			    const int entry_num)
{
  char *a = 0, *c;
  int local_len = len;
    
  if (which == 0 || entry_num > which->num_entries || entry_num < 0)
    return 0;

  if (local_len == 0) local_len = strlen(entry);

  if (entry_num == which->num_entries)
  {
    // This entry doesn't exist but it can be added

    SLM_add_list_entry(which, entry);
    a = *(which->list+entry_num);
    return a;
  }
  local_len =
    local_len < which->sizeof_entries ? local_len : which->sizeof_entries;
  c = *(which->list+entry_num);

  for (const char *a=entry; *a && local_len--; *c++ = *a++);
  *c = '\0';
  return a;
}

/* c------------------------------------------------------------------------ */

void SLM_reset_list_entries (struct solo_list_mgmt *which)
{
    int ii;

    if(!which || !which->num_entries)
	  return;

    for(ii=0; ii < which->num_entries; ii++) {
	strcpy(*(which->list +ii), " ");
    }
    which->num_entries = 0;
}

/* c------------------------------------------------------------------------ */

void SLM_unmalloc_list_mgmt (struct solo_list_mgmt *slm)
{
    int ii;

    if(!slm)
	  return;

    for(ii=0; ii < slm->max_entries; ii++) {
	free(*(slm->list +ii));
    }
    free(slm);
}

/* c------------------------------------------------------------------------ */

void SLM_sort_slm_entries (struct solo_list_mgmt *slm)
{
   char *aa, *bb;
   int ii, jj, nn = slm->num_entries;
   char str[256];
   
   for(ii=0; ii < nn-1; ii++) {
      aa = SLM_list_entry(slm, ii);
      for(jj=ii+1; jj < nn; jj++) {
	 bb = SLM_list_entry(slm, jj);
	 if(strcmp(bb, aa) < 0) {
	    strcpy (str, aa);
	    SLM_modify_list_entry(slm, bb, strlen (bb), ii);
	    aa = SLM_list_entry(slm, ii);
	    SLM_modify_list_entry(slm, str, strlen (str), jj);
	 }
      }
   }
}
  
/* c------------------------------------------------------------------------ */

void SLM_sort_strings (char **sptr, int ns)
{
    int ii, jj;
    char *keep;

    for(ii=0; ii < ns-1; ii++) {
	for(jj=ii+1; jj < ns; jj++) {
	    if(strcmp(*(sptr+jj), *(sptr+ii)) < 0) {
		keep = *(sptr+ii);
		*(sptr+ii) = *(sptr+jj);
		*(sptr+jj) = keep;
	    }
	}
    }
}
