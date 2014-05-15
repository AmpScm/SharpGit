#include "stdafx.h"

#include "GitRefSpec.h"
#include "GitClientContext.h"
#include "deep_wrap.h"

using namespace SharpGit;
using namespace SharpGit::Implementation;
using namespace SharpGit::Plumbing;
using System::Collections::Generic::List;


GitRefSpec::GitRefSpec(String ^source, String ^destination, bool updateAlways)
{
    if (String::IsNullOrEmpty(source))
        throw gcnew ArgumentNullException("source");
    else if (String::IsNullOrEmpty(destination))
        throw gcnew ArgumentNullException("destination");

    GitPool pool;

    _src = GitBase::Utf8_PtrToString(svn_relpath_canonicalize(pool.AllocString(source), pool.Handle));
    _dst = GitBase::Utf8_PtrToString(svn_relpath_canonicalize(pool.AllocString(destination), pool.Handle));
    _updateAlways = updateAlways;
}

GitRefSpec::GitRefSpec(String ^source, String ^destination)
{
    GitRefSpec::GitRefSpec(source, destination, false);
}

GitRefSpec::GitRefSpec(const git_refspec &refspec)
{
    GitPool pool;
    _src = GitBase::Utf8_PtrToString(svn_relpath_canonicalize(git_refspec_src(&refspec), pool.Handle));
    _dst = GitBase::Utf8_PtrToString(svn_relpath_canonicalize(git_refspec_dst(&refspec), pool.Handle));
    _updateAlways = git_refspec_force(&refspec) != 0;
}

GitRefSpec ^ GitRefSpec::Parse(String ^refspec)
{
    GitRefSpec ^spec;
    if (TryParse(refspec, spec))
        return spec;

    throw gcnew ArgumentException("Invalid refspec", "refspec");
}

bool GitRefSpec::TryParse(String ^refspec, [Out]GitRefSpec ^%value)
{
    if (String::IsNullOrEmpty(refspec))
        throw gcnew ArgumentNullException("refspec");

    GitPool pool;
    git_refspec *result;
    if (sharpgit_refspec_parse(&result, pool.AllocString(refspec), true))
    {
        giterr_clear();
        value = nullptr;
        return false;
    }

    try
    {
        value = gcnew GitRefSpec(*result);
        return true;
    }
    finally
    {
        sharpgit_refspec_free(result);
    }
}

#include "UnmanagedStructs.h"