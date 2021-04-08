#include "pch.h"

#include "GitClient.h"
#include "Plumbing/GitRemote.h"
#include "GitLibraryAttribute.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace SharpGit;
using namespace SharpGit::Implementation;

GitClient::GitClient()
{
    _clientBaton = gcnew AprBaton<GitClient^>(this);
    _credentialHandlers = gcnew List<System::EventHandler<GitCredentialEventArgs^>^>();
}

GitClient::~GitClient()
{
    delete _clientBaton;
}

struct remote_baton_t
{
    void *client;
    int _authNr;
    apr_pool_t *pool;

static int __cdecl sideband_progress(const char *str, int len, void *data)
{
    remote_baton_t *payload = (remote_baton_t*)data;
    GitClient ^client = AprBaton<GitClient^>::Get(payload->client);

    GitProgressEventArgs ^ee = gcnew GitProgressEventArgs(str, len);

    try
    {
        client->InvokeProgress(ee);

        return 0;
    }
    catch (Exception ^e)
    {
        return GitBase::WrapError(e);
    }
    finally
    {
        ee->Detach(false);
    }
}

static int __cdecl completion(git_remote_completion_type type, void *data)
{
    remote_baton_t *payload = (remote_baton_t*)data;
    GitClient ^client = AprBaton<GitClient^>::Get(payload->client);

    GitRemoteCompletedEventArgs ^ee = gcnew GitRemoteCompletedEventArgs(type);
    try
    {
        client->InvokeRemoteCompleted(ee);

        return 0;
    }
    catch (Exception ^e)
    {
        return GitBase::WrapError(e);
    }
    finally
    {
        ee->Detach(false);
    }
}

static int __cdecl credentials(git_cred **cred, const char *url, const char *username_from_url, unsigned int allowed_types, void *data)
{
    remote_baton_t *payload = (remote_baton_t*)data;
    GitClient ^client = AprBaton<GitClient^>::Get(payload->client);

    GitCredentialEventArgs ^ee = gcnew GitCredentialEventArgs(cred, url, username_from_url, allowed_types, payload->_authNr++);

    try
    {
        bool ok = false;

        try
        {
            client->InvokeCredential(ee);
            ok = true;
            if (*cred && GitAuthContext::Current)
                GitAuthContext::Current->Attempt();
        }
        finally
        {
            if (!ok && *cred)
            {
                (*cred)->free(*cred);
                *cred = nullptr;
            }
        }

        if (*cred != nullptr)
            return 0;

        giterr_set_str(GITERR_CALLBACK, "Callback didn't provide credentials");
        return -1;
    }
    catch (Exception ^e)
    {
        return GitBase::WrapError(e);
    }
    finally
    {
        ee->Detach(false);
    }
}

static int __cdecl certificate_check(git_cert *cert, int valid, const char *host, void *data)
{
    remote_baton_t *payload = (remote_baton_t*)data;
    GitClient ^client = AprBaton<GitClient^>::Get(payload->client);

    GitCertificateEventArgs ^ee = gcnew GitCertificateEventArgs(cert, valid, host);

    try
    {
        //client->InvokeTransferProgress(gcnew GitTransferProgressEventArgs(*stats));
        client->InvokeCertificate(ee);

        return 0;
    }
    catch (Exception ^e)
    {
        return GitBase::WrapError(e);
    }
    finally
    {
        ee->Detach(false);
    }
}

static int __cdecl transfer_progress(const git_transfer_progress *stats, void *data)
{
    remote_baton_t *payload = (remote_baton_t*)data;
    GitClient ^client = AprBaton<GitClient^>::Get(payload->client);

    GitTransferProgressEventArgs ^ee = gcnew GitTransferProgressEventArgs(*stats);
    try
    {
        client->InvokeTransferProgress(ee);

        return 0;
    }
    catch (Exception ^e)
    {
        return GitBase::WrapError(e);
    }
    finally
    {
        ee->Detach(false);
    }
}

static int __cdecl update_tips(const char *refname, const git_oid *a, const git_oid *b, void *data)
{
    remote_baton_t *payload = (remote_baton_t*)data;
    GitClient ^client = AprBaton<GitClient^>::Get(payload->client);

    GitUpdateTipsEventArgs ^ee = gcnew GitUpdateTipsEventArgs(refname, a, b);
    try
    {
        client->InvokeUpdateTips(ee);

        return 0;
    }
    catch (Exception ^e)
    {
        return GitBase::WrapError(e);
    }
    finally
    {
        ee->Detach(false);
    }
}

static int __cdecl pack_progress(int stage, unsigned int current, unsigned int total, void *data)
{
  remote_baton_t *payload = (remote_baton_t*)data;
  GitClient ^client = AprBaton<GitClient^>::Get(payload->client);

  GitPackProgressEventArgs ^ee = gcnew GitPackProgressEventArgs(stage, current, total);
  try
  {
      client->InvokePackProgress(ee);

      return 0;
  }
  catch (Exception ^e)
  {
      return GitBase::WrapError(e);
  }
  finally
  {
      ee->Detach(false);
  }
}

static int __cdecl push_transfer_progress(unsigned int current, unsigned int total, size_t bytes, void* data)
{
  remote_baton_t *payload = (remote_baton_t*)data;
  GitClient ^client = AprBaton<GitClient^>::Get(payload->client);

  GitPushTransferProgressEventArgs ^ee = gcnew GitPushTransferProgressEventArgs(current, total, bytes);
  try
  {
      client->InvokePushTransferProgress(ee);

      return 0;
  }
  catch (Exception ^e)
  {
      return GitBase::WrapError(e);
  }
  finally
  {
      ee->Detach(false);
  }
}

static int __cdecl push_update_reference(const char *ref, const char *msg, void *data)
{
  remote_baton_t *payload = (remote_baton_t*)data;
  GitClient ^client = AprBaton<GitClient^>::Get(payload->client);

  GitPushUpdateReferenceEventArgs ^ee = gcnew GitPushUpdateReferenceEventArgs(ref, msg);
  try
  {
      client->InvokePushUpdateReference(ee);

      return 0;
  }
  catch (Exception ^e)
  {
      return GitBase::WrapError(e);
  }
  finally
  {
      ee->Detach(false);
  }
}

};

git_remote_callbacks *GitClient::get_callbacks(GitPool ^pool)
{
    git_remote_callbacks* cb = (git_remote_callbacks*)pool->Alloc(sizeof(*cb));
    git_remote_init_callbacks(cb, GIT_REMOTE_CALLBACKS_VERSION);

    remote_baton_t *payload = (remote_baton_t *)pool->Alloc(sizeof(*payload));

    payload->client = _clientBaton->Handle;
    payload->_authNr = 0;
    payload->pool = pool->Handle;

    cb->payload = payload;
    cb->sideband_progress = remote_baton_t::sideband_progress;
    cb->completion = remote_baton_t::completion;
    cb->credentials = remote_baton_t::credentials;
    cb->certificate_check = remote_baton_t::certificate_check;
    cb->transfer_progress = remote_baton_t::transfer_progress;
    cb->update_tips = remote_baton_t::update_tips;

    cb->pack_progress = remote_baton_t::pack_progress;
    cb->push_transfer_progress = remote_baton_t::push_transfer_progress;
    cb->push_update_reference = remote_baton_t::push_update_reference;

    return cb;
}

static GitClient::GitClient()
{
#ifndef LIBGIT2_VER_PATCH /* Unavailable pre 0.23.0 */
# define LIBGIT2_VER_PATCH 0
#endif
    _version = gcnew System::Version(LIBGIT2_VER_MAJOR, LIBGIT2_VER_MINOR, LIBGIT2_VER_REVISION, LIBGIT2_VER_PATCH);
}

System::Version^ GitClient::Version::get()
{
    return _version;
}

String^ GitClient::VersionString::get()
{
    return LIBGIT2_VERSION;
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

void GitClient::HookCredentials(bool add, EventHandler<GitCredentialEventArgs^>^ handler)
{
    if (add)
    {
        if (!_credentialHandlers->Contains(handler))
            _credentialHandlers->Add(handler);
    }
    else
    {
        _credentialHandlers->Remove(handler);
    }
}

void GitClient::OnCredential(GitCredentialEventArgs ^e)
{
    if (!e->Invocation)
        _nextCredentialHandler = 0;

    while (_nextCredentialHandler < _credentialHandlers->Count)
    {
        EventHandler<GitCredentialEventArgs^>^ ev_handler = _credentialHandlers[_nextCredentialHandler];

        ev_handler(this, e);

        if (e->HasCredential)
            break;

        _nextCredentialHandler++;
    }
}
