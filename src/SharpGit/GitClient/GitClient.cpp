#include "stdafx.h"

#include "GitClient.h"
#include "GitLibraryAttribute.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace SharpGit;
using namespace SharpGit::Implementation;

GitClient::GitClient()
{
    _clientBaton = gcnew AprBaton<GitClient^>(this);
}

GitClient::~GitClient()
{
    delete _clientBaton;
}

static int __cdecl remotecb_sideband_progress(const char *str, int len, void *data)
{
    GitClient ^client = AprBaton<GitClient^>::Get(data);

    try
    {
        client->InvokeProgress(gcnew GitProgressEventArgs(str, len));

        return 0;
    }
    catch (Exception ^e)
    {
        return GitBase::WrapError(e);
    }
}

static int __cdecl remotecb_completion(git_remote_completion_type type, void *data)
{
    GitClient ^client = AprBaton<GitClient^>::Get(data);

    try
    {
        client->InvokeRemoteCompleted(gcnew GitRemoteCompletedEventArgs(type));

        return 0;
    }
    catch (Exception ^e)
    {
        return GitBase::WrapError(e);
    }
}

static int __cdecl remotecb_credentials(git_cred **cred, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload)
{
    GitClient ^client = AprBaton<GitClient^>::Get(payload);

    try
    {
        GitCredentialsEventArgs ^e = gcnew GitCredentialsEventArgs(url, username_from_url, allowed_types);

        client->InvokeCredentials(*cred, e);

        if (*cred != nullptr)
            return 0;

        giterr_set_str(GITERR_CALLBACK, "Callback didn't provide credentials");
        return -1;
    }
    catch (Exception ^e)
    {
        return GitBase::WrapError(e);
    }
}

static int __cdecl remotecb_transfer_progress(const git_transfer_progress *stats, void *payload)
{
    GitClient ^client = AprBaton<GitClient^>::Get(payload);

    try
    {
        client->InvokeTransferProgress(gcnew GitTransferProgressEventArgs(*stats));

        return 0;
    }
    catch (Exception ^e)
    {
        return GitBase::WrapError(e);
    }
}

static int __cdecl remotecb_update_tips(const char *refname, const git_oid *a, const git_oid *b, void *data)
{
    GitClient ^client = AprBaton<GitClient^>::Get(data);

    try
    {
        client->InvokeUpdateTips(gcnew GitUpdateTipsEventArgs(refname, a, b));

        return 0;
    }
    catch (Exception ^e)
    {
        return GitBase::WrapError(e);
    }
}

git_remote_callbacks *GitClient::get_callbacks()
{
    if (_callbacks)
        return _callbacks;

    git_remote_callbacks* cb = (git_remote_callbacks*)_pool.Alloc(sizeof(*cb));
    git_remote_init_callbacks(cb, GIT_REMOTE_CALLBACKS_VERSION);

    cb->payload = _clientBaton->Handle;
    cb->sideband_progress = remotecb_sideband_progress;
    cb->completion = remotecb_completion;
    cb->credentials = remotecb_credentials;
    cb->transfer_progress = remotecb_transfer_progress;
    cb->update_tips = remotecb_update_tips;

    return _callbacks = cb;
}

System::Version^ GitClient::Version::get()
{
    int major, minor, rev;

    git_libgit2_version(&major, &minor, &rev);

    return gcnew System::Version(major, minor, rev);
}

System::Version^ GitClient::SharpGitVersion::get()
{
    return (gcnew System::Reflection::AssemblyName(GitClient::typeid->Assembly->FullName))->Version;
}

static int CompareLibrary(GitLibrary^ x, GitLibrary^ y)
{
    return StringComparer::OrdinalIgnoreCase->Compare(x->Name, y->Name);
}

ICollection<GitLibrary^>^ GitClient::GitLibraries::get()
{
    if (_gitLibraries)
        return _gitLibraries;

    System::Collections::Generic::List<GitLibrary^>^ libs
        = gcnew System::Collections::Generic::List<GitLibrary^>();

    for each(GitLibraryAttribute^ i in GitClient::typeid->Assembly->GetCustomAttributes(GitLibraryAttribute::typeid, false))
    {
        libs->Add(gcnew GitLibrary(i));
    }

    libs->Sort(gcnew Comparison<GitLibrary^>(CompareLibrary));

    return _gitLibraries = libs->AsReadOnly();
}
