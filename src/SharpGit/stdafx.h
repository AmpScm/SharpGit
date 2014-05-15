#pragma once

#pragma warning(push)
#pragma warning(disable: 4634 4635)
#include <git2.h>
#include <libssh2\libssh2.h>
#include <apr.h>
#include <apr_file_io.h>
#include <svn_dirent_uri.h>
#include <svn_io.h>
#include <svn_pools.h>
#include <svn_utf.h>
#pragma warning(pop)

#include <assert.h>
#include <malloc.h>

#pragma warning(disable: 4706) // assignment within conditional expression

using System::String;

namespace SharpGit {
	namespace Implementation {
		ref class GitPool;
		ref class GitBase;
	}

	namespace Plumbing {
		ref class GitRepository;
		ref class GitIndex;
	}
}

using SharpGit::Implementation::GitBase;
using SharpGit::Implementation::GitPool;
using SharpGit::Plumbing::GitRepository;
using System::Runtime::InteropServices::OutAttribute;
using System::Collections::Generic::ICollection;
using System::Collections::Generic::IDictionary;
using System::Collections::Generic::IEnumerable;
using System::Collections::Generic::IEnumerator;
using System::Collections::Generic::KeyValuePair;

#include "GitId.h"
#pragma warning(disable: 4127) // Conditiona; expression is constant
#define GIT_THROW(expr)                                   \
    do                                                    \
    {                                                     \
        int __git_throw_r = (expr);                       \
                                                          \
        if (__git_throw_r)                                \
          {                                               \
              Exception ^__git_throw_ex =                 \
                  GitException::Create(__git_throw_r,     \
                                       giterr_last());    \
              giterr_clear();                             \
              throw __git_throw_ex;                       \
          }                                               \
    } while(0)

#define GIT_IF(expr)                                      \
    if (expr)                                             \
    { giterr_clear(); }                                   \
    else

struct Git_buf : public git_buf
{
    Git_buf()
    {
        ptr = "";
        asize = size = 0;
    }

    ~Git_buf()
    {
        if (ptr)
            git_buf_free(this);
    }
};

struct Git_strarray : public git_strarray
{
    Git_strarray()
    {
        count = 0;
        strings = nullptr;
    }

    ~Git_strarray()
    {
        if (strings)
            git_strarray_free(this);
    }
};
