#include "pch.h"

#include "GitRepository.h"
#include "GitRemote.h"
#include "GitClient/GitClient.h"
#include "GitClient/GitFetchArgs.h"
#include "GitClient/GitPushArgs.h"
#include "GitRefSpec.h"

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
            _connected = false;
            git_remote_free(_remote);
        }
        finally
        {
            _remote = nullptr;
        }
}

GitRemote::~GitRemote()
{
    GitRemote::!GitRemote();
}

String ^ GitRemote::Name::get()
{
    if (!_name && _remote)
        _name = GitBase::Utf8_PtrToString(git_remote_name(Handle));

    return _name;
}

bool GitRemote::Fetch(IEnumerable<GitRefSpec^>^ refspecs, GitFetchArgs ^args, Implementation::IHasRemoteCallbacks ^cb)
{
    GitPool pool(_repository->Pool);
    GitAuthContext authContext;

    Git_strarray specs;
    git_strarray *pSpecs = &specs;

    // Copy refspecs
    if (refspecs)
    {
        List<String^>^ sp = gcnew List<String^>();

        for each(GitRefSpec ^spec in refspecs)
            sp->Add(spec->ToString());

        pSpecs = pool.AllocStringArray(sp);
    }

    int maxAttempts = 15;
    int r = 0;

    do
    {
        if (r)
            giterr_clear();

        authContext.Clear();

        r = git_remote_fetch(Handle, pSpecs, args->AllocOptions(%pool), nullptr);

        if (r)
            switch((GitError)giterr_last()->klass)
            {
              case GitError::Network:
              case GitError::Ssh:
              case GitError::SecureSockets:
                  continue;
            }

        break;
    }
    while (--maxAttempts && authContext.Continue());

    _connected = true;
    return args->HandleGitError(this, r);;
}

void GitRemote::Stop(GitArgs ^args)
{
    git_remote_stop(Handle);
}

String ^ GitRemote::DefaultBranch::get()
{
    if (!_connected)
        return nullptr;

    Git_buf buf;

    GIT_THROW(git_remote_default_branch(&buf, Handle));

    return Utf8_PtrToString(buf.ptr, buf.size);
}

String ^ GitRemote::FetchSpecTransformToSource(String ^spec)
{
    if (String::IsNullOrEmpty(spec))
        throw gcnew ArgumentNullException("spec");

    const git_refspec *spc = git_remote_get_refspec(Handle, 0);

    if (!spc)
        return nullptr;

    GitPool pool(_repository->Pool);
    Git_buf buf;
    GIT_THROW(git_refspec_rtransform(&buf, spc, pool.AllocString(spec)));

    return Utf8_PtrToString(buf.ptr, buf.size);
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

        GIT_THROW(git_remote_lookup(&rm, _repository->Handle, remotes.strings[i]));

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

    if (git_remote_lookup(&rm, _repository->Handle, pName))
    {
        giterr_clear();
        value = nullptr;
        return false;
    }

    value = gcnew GitRemote(_repository, rm);
    return true;
}

bool GitRemote::Delete()
{
    GitPool pool(_repository->Pool);

    const char *pName = pool.AllocString(Name);
    GIT_THROW(git_remote_delete(_repository->Handle, pName));

    return true;
}

GitRemote ^ GitRemoteCollection::CreateAnonymous(Uri ^remoteRepository)
{
    GitPool pool(_repository->Pool);
    git_remote *rm;

    GIT_THROW(git_remote_create_anonymous(&rm, _repository->Handle,
                                          svn_uri_canonicalize(pool.AllocString(remoteRepository->AbsoluteUri), pool.Handle)));

    return gcnew GitRemote(_repository, rm);
}

System::Uri ^ GitRemote::Uri::get()
{
    return gcnew System::Uri(GitBase::Utf8_PtrToString(git_remote_url(Handle)), UriKind::Absolute);
}

void GitRemote::Uri::set(System::Uri ^value)
{
    if (!value || !value->IsAbsoluteUri)
        throw gcnew InvalidOperationException();

    GitPool pool;
    GIT_THROW(git_remote_set_url(_repository->Handle, git_remote_name(Handle),
                                 svn_uri_canonicalize(pool.AllocString(value->AbsoluteUri), pool.Handle)));
}

System::Uri ^ GitRemote::PushUri::get()
{
    return gcnew System::Uri(GitBase::Utf8_PtrToString(git_remote_pushurl(Handle)), UriKind::Absolute);
}

void GitRemote::PushUri::set(System::Uri ^value)
{
    if (!value || !value->IsAbsoluteUri)
        throw gcnew InvalidOperationException();

    GitPool pool;
    GIT_THROW(git_remote_set_pushurl(_repository->Handle, git_remote_name(Handle),
                                     svn_uri_canonicalize(pool.AllocString(value->AbsoluteUri), pool.Handle)));
}

IEnumerable<GitRefSpec^>^ GitRemote::FetchRefSpecs::get()
{
    Git_strarray arr;

    GIT_THROW(git_remote_get_fetch_refspecs(&arr, Handle));

    array<GitRefSpec^> ^specs = gcnew array<GitRefSpec^>(arr.count);

    for (int i = 0; i < arr.count; i++)
    {
        specs[i] = GitRefSpec::Parse(GitBase::Utf8_PtrToString(arr.strings[i]));
    }

    return specs;
}

void GitRemote::FetchRefSpecs::set(IEnumerable<GitRefSpec^>^ value)
{
    throw gcnew NotImplementedException();
}

IEnumerable<GitRefSpec^>^ GitRemote::PushRefSpecs::get()
{
    Git_strarray arr;

    GIT_THROW(git_remote_get_push_refspecs(&arr, Handle));

    array<GitRefSpec^> ^specs = gcnew array<GitRefSpec^>(arr.count);

    for (int i = 0; i < arr.count; i++)
    {
        specs[i] = GitRefSpec::Parse(GitBase::Utf8_PtrToString(arr.strings[i]));
    }

    return specs;
}

void GitRemote::PushRefSpecs::set(IEnumerable<GitRefSpec^>^ value)
{
    throw gcnew NotImplementedException();
}

IList<GitRemoteHead^> ^ GitRemote::GetHeads()
{
    if (!_connected)
        return nullptr;

    const git_remote_head **p_heads;
    size_t heads_cnt;
    GIT_THROW(git_remote_ls(&p_heads, &heads_cnt, Handle));

    array<GitRemoteHead^>^ heads = gcnew array<GitRemoteHead^>(heads_cnt);

    for(size_t i = 0; i < heads_cnt; i++)
      {
        heads[i] = gcnew GitRemoteHead(p_heads[i]);
      }

    return dynamic_cast<IList<GitRemoteHead^>^>(heads);
}

bool GitRemote::Push(IEnumerable<GitRefSpec^> ^refspecs, GitPushArgs ^args, Implementation::IHasRemoteCallbacks ^cb)
{
    if (!args)
        throw gcnew ArgumentNullException("args");

    GitPool pool(_repository->Pool);
    GitAuthContext authContext;
    Git_strarray specs;
    git_strarray *pSpecs = &specs;

    // Copy refspecs
    if (refspecs)
    {
        List<String^>^ sp = gcnew List<String^>();

        for each(GitRefSpec ^spec in refspecs)
            sp->Add(spec->ToString());

        pSpecs = pool.AllocStringArray(sp);
    }

    git_push_options *options = args->AllocOptions(%pool);

    options->callbacks = *cb->get_callbacks(%pool);

    int r = 0;
    int maxAttempts = 15;

    do
    {
        if (r)
            giterr_clear();

        authContext.Clear();

        r = git_remote_push(Handle,
                            pSpecs,
                            options);

        if (r)
            switch((GitError)giterr_last()->klass)
            {
              case GitError::Network:
              case GitError::Ssh:
              case GitError::SecureSockets:
                  continue;
            }

        break;
    }
    while (--maxAttempts && authContext.Continue());

    return args->HandleGitError(this, r);
}

git_push_options * GitPushArgs::AllocOptions(GitPool ^pool)
{
    git_push_options *options = (git_push_options *)pool->Alloc(sizeof(*options));

    git_push_init_options(options, GIT_PUSH_OPTIONS_VERSION);

    options->pb_parallelism = 0; // Auto

    return options;
}


git_fetch_options * GitFetchArgs::AllocOptions(GitPool ^pool)
{
    git_fetch_options *options = (git_fetch_options *)pool->Alloc(sizeof(*options));

    git_fetch_init_options(options, GIT_FETCH_OPTIONS_VERSION);


    return options;
}
