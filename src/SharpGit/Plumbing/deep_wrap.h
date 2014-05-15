#pragma once

#include <git2.h>

#ifdef __cplusplus
extern "C" {
#endif

int sharpgit_refspec_parse(git_refspec **refspec, const char *str, BOOL isFetch);
void sharpgit_refspec_free(git_refspec *refspec);

#ifdef __cplusplus
}
#endif
