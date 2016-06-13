#ifndef solo_list_mgmt_hh
#define solo_list_mgmt_hh

# ifndef SOLO_LIST_MGMT
# define SOLO_LIST_MGMT
# define SLM_CODE

struct solo_list_mgmt {
    int num_entries;
    int sizeof_entries;
    int max_entries;
    char **list;
};

#endif

extern void SLM_dump_list(struct solo_list_mgmt *slm);
extern void SLM_print_list(struct solo_list_mgmt *slm);
extern void SLM_add_list_entry(struct solo_list_mgmt *which,
			       const std::string &entry);
extern char *SLM_list_entry(struct solo_list_mgmt *which, int entry_num);
extern void SLM_list_remove_dups(struct solo_list_mgmt *slm);
extern void SLM_list_remove_entry(struct solo_list_mgmt *slm, int ii, int jj);
//extern void SLM_list_sort_file_names(struct solo_list_mgmt *slm);
extern struct solo_list_mgmt *SLM_malloc_list_mgmt(int sizeof_entries);
extern char *SLM_modify_list_entry(struct solo_list_mgmt *which,
				   const char *entry, const int len,
				   const int entry_num);
extern void SLM_reset_list_entries(struct solo_list_mgmt *which);
extern void SLM_unmalloc_list_mgmt(struct solo_list_mgmt *slm);
extern void SLM_sort_slm_entries(struct solo_list_mgmt *slm);
extern void SLM_sort_strings (char **sptr, int ns);

#endif
