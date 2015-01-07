#include <git2.h>

#pragma warning(disable: 4200) // nonstandard extension used : zero-sized array in struct/union

#include <git2/private/common.h>
#include <git2/private/refspec.h>
#include <git2/private/util.h>
#include <git2/private/posix.h>
#include <git2/private/refs.h>
#include <git2/private/vector.h>

#include <git2/private/refspec.h>

#include "deep_wrap.h"



int sharpgit_refspec_parse(git_refspec **refspec, const char *str, BOOL isFetch)
{
  int r;
  *refspec = malloc(sizeof(**refspec));

  r = git_refspec__parse(*refspec, str, isFetch != 0);

  if (r)
  {
      free(*refspec);
      *refspec = NULL;
  }
  return r;
}

void sharpgit_refspec_free(git_refspec *refspec)
{
  git_refspec__free(refspec);
  free(refspec);
}
