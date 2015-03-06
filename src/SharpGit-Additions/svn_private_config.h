#pragma once

#define _(x) (x)
#define Q_(x1, x2, n) (((n) == 1) ? x1 : x2)
#define gettext(x) (x)
#define dgettext(domain, x) (x)
#define N_(X) X

#define SVN_PATH_LOCAL_SEPARATOR '\\'
#define SVN_BUILD_HOST "SharpGit"
#define SVN_UNUSED(v) ( (void)(v) )
#ifndef alloca
#define alloca  _alloca
#endif