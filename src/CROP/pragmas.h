/* tell wcc to put underscores before symbol not after */
#pragma aux default parm caller [] modify [ax dx];
#pragma aux wait_vblank "_*" parm caller [] modify [ax dx];
#pragma aux jset_colors "_*" parm caller [] modify [ax dx];
#pragma aux cset_colors "_*" parm caller [] modify [ax dx];
#pragma aux sysint "_*" parm caller [] modify [ax cx dx];
#pragma aux norm_pointer "_*" parm caller [] modify [ax dx];
#pragma aux enorm_pointer "_*" parm caller [] modify [ax dx];
#pragma aux wait_novblank "_*" parm caller [] modify [ax dx];
#ifdef SOON
#endif /* SOON */

#define _toupper(c)	((c) + 'A' - 'a')
#define _tolower(c)	((c) + 'a' - 'A')
