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
        _name = GitBase::Utf8_PtrToString(git_remote_name(Handle));

    return _name;
}

void GitRemote::SetCallbacks(const git_remote_callbacks *callbacks)
{
    GIT_THROW(git_remote_set_callbacks(Handle, callbacks));
}

bool GitRemote::Connect(bool forFetch, GitArgs ^args)
{
    GIT_THROW(git_remote_connect(Handle, forFetch ? GIT_DIRECTION_FETCH : GIT_DIRECTION_PUSH));

    return true;
}

bool GitRemote::Download(GitArgs ^args)
{
    GIT_THROW(git_remote_download(Handle));

    return true;
}
bool GitRemote::Disconnect(GitArgs ^args)
{
    git_remote_disconnect(Handle);
    return true;
}

bool GitRemote::UpdateTips(GitCreateRefArgs ^args)
{
    GitPool pool(_repository->Pool);

    GIT_THROW(git_remote_update_tips(Handle, args->Signature->Alloc(_repository, %pool), args->AllocLogMessage(%pool)));
    return true;
}

bool GitRemote::Save(GitArgs ^args)
{
    GIT_THROW(git_remote_save(Handle));

    return true;
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
    Git_strarray remotes;
    GIT_THROW(git_remote_list(&remotes, _repository->Handle));

    array<GitRemote^>^ gitRemotes = gcnew array<GitRemote^>(remotes.count);

    for (int i = 0; i < remotes.count; i++)
    {
        git_remote *rm;

        GIT_THROW(git_remote_load(&rm, _repository->Handle, remotes.strings[i]));

        gitRemotes[i] = gcnew GitRemote(_repository, rm);
    }

    return ((System::Collections::Generic::IEnumerable<GitRemote^>^)gitRemotes)->GetEnumerator();
}

bool GitRemoteCollection::TryGet(String ^name, [Out] GitRemote ^%value)
{
    if (String::IsNullOrEmpty(name))
        throw gcnew ArgumentNullException("name");

    GitPool pool(_repository->Pool);

    git_remote *rm;

    const char *pName = pool.AllocString(name);

    if (!git_remote_is_valid_name(pName))
    {
        value = nullptr;
        return false;
    }

    if (!git_remote_load(&rm, _repository->Handle, pName))
    {
        giterr_clear();
        value = nullptr;
        return false;
    }

    value = gcnew GitRemote(_repository, rm);
    return true;
}

GitRemote ^ GitRemoteCollection::CreateAnonymous(Uri ^remoteRepository)
{
    GitPool pool(_repository->Pool);
    git_remote *rm;

    GIT_THROW(git_remote_create_anonymous(&rm, _repository->Handle,
                                          svn_uri_canonicalize(pool.AllocString(remoteRepository->AbsoluteUri), pool.Handle),
                                          nullptr));

    return gcnew GitRemote(_repository, rm);
}

