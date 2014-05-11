#include "stdafx.h"

#include "GitRepository.h"
#include "GitRemote.h"

#include "UnmanagedStructs.h"

using namespace SharpGit;
using namespace SharpGit::Implementation;
using namespace SharpGit::Plumbing;
using System::Collections::Generic::List;

GitRemote::GitRemote(GitRepository ^repository, git_remote *remote)
{
    if (!repository)
        throw gcnew ArgumentNullException("repository");
    else if (!remote)
        throw gcnew ArgumentNullException("remote");

    _repository = repository;
    _remote = remote;
}

GitRemote::!GitRemote()
{
    if (_remote)
        try
        {
            git_remote_free(_remote);
        }
        finally
        {
            _remote = nullptr;
        }
}

GitRemote::~GitRemote()
{
    if (_remote)
        try
        {
            git_remote_free(_remote);
        }
        finally
        {
            _remote = nullptr;
        }
}

String ^ GitRemote::Name::get()
{
    if (!_name && _remote)
        _name = GitBase::Utf8_PtrToString(git_remote_name(_remote));

    return _name;
}

GitRemoteCollection^ GitRepository::Remotes::get()
{
    if (!_remotes)
    {
        if (IsDisposed)
              throw gcnew ObjectDisposedException("repository");

        _remotes = gcnew GitRemoteCollection(this);
    }

    return _remotes;
}

System::Collections::Generic::IEnumerator<GitRemote^>^ GitRemoteCollection::GetEnumerator()
{
    throw gcnew NotImplementedException();
}
