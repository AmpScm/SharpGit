#include "stdafx.h"

#include "GitCommit.h"
#include "GitConfiguration.h"
#include "GitRepository.h"
#include "GitTree.h"
#include "GitReference.h"
#include "GitRemote.h"
#include "GitBranch.h"
#include "GitRefSpec.h"

using namespace SharpGit::Plumbing;
using namespace SharpGit;
using System::Collections::Generic::List;

GitBranch^ GitRepository::HeadBranch::get()
{
    GitReference^ ref = HeadReference;

    if (!ref)
        return nullptr;

    return gcnew GitBranch(this, ref->Name, GIT_BRANCH_LOCAL);
}


bool GitBranch::IsHead::get()
{
    return git_branch_is_head(Reference->Handle) != 0;
}

String ^ GitBranch::RemoteName::get()
{
    Git_buf buf;

    GitPool pool(_repository->Pool);

    if (!IsRemote)
    {
        String ^rn;
        if(_repository->Configuration->TryGetString(GitConfiguration::Join("branch", ShortName, "remote"), rn))
            return rn;

        return nullptr;
    }

    if (git_branch_remote_name(&buf, _repository->Handle, pool.AllocString(Name)))
    {
        giterr_clear();
        return nullptr;
    }

    String ^result = GitBase::Utf8_PtrToString(buf.ptr, buf.size);

    return result;
}

String ^ GitBranch::Name::get()
{
    if (!_name)
        _name = _reference->Name;

    return _name;
}

String ^ GitBranch::ShortName::get()
{
    if (!_shortName)
    {
        const char *name;
        if (!git_branch_name(&name, Reference->Handle))
        {
            _shortName = GitBase::Utf8_PtrToString(name);
        }
        else
            giterr_clear();
    }
    return _shortName;
}


GitReference ^ GitBranch::UpstreamReference::get()
{
    if (!_upstreamReference && !_resolvedUpstream)
    {
        _resolvedUpstream = true;

        git_reference *ref;

        if (!git_branch_upstream(&ref, Reference->Handle))
        {
            _upstreamReference = gcnew GitReference(_repository, ref);
        }
    }

    return _upstreamReference;
}

String ^ GitBranch::LocalUpstreamName::get()
{
    if (!_localUpstreamName && !_resolvedLocalUpstreamName)
    {
        _resolvedLocalUpstreamName = true;

        GitPool pool(_repository->Pool);
        Git_buf result;

        if (!git_branch_upstream_name(&result, _repository->Handle, pool.AllocString(Name)))
        {
            _localUpstreamName = GitBase::Utf8_PtrToString(result.ptr, result.size);
        }
        else
            giterr_clear();
    }

    return _localUpstreamName;
}

String ^ GitBranch::RemoteUpstreamName::get()
{
    if (!_remoteUpstreamName && !_resolvedRemoteUpstreamName)
    {
        _resolvedRemoteUpstreamName = true;

        GitRemote ^rm;
        String^ name;
        if (IsRemote && Repository->Remotes->TryGet(RemoteName, rm))
        {
            try
            {
                _remoteUpstreamName = rm->FetchSpecTransformToSource(Name);
            }
            finally
            {
                delete rm;
            }
        }
        else if (IsLocal && Repository->Configuration->TryGetString(GitConfiguration::Join("branch", ShortName, "merge"), name))
        {
            _remoteUpstreamName = name;
        }
    }

    return _remoteUpstreamName;
}


bool GitBranch::IsTracking::get()
{
    if (IsRemote || !LocalUpstreamName)
        return false;

    return (TrackedBranch != nullptr);
}

GitBranch ^ GitBranch::TrackedBranch::get()
{
    String ^name = LocalUpstreamName;
    if (!name)
        return nullptr;

    GitBranch ^result;
    if (_repository->Branches->TryGet(name, result))
        return result;

    return gcnew GitBranch(_repository, name, GIT_BRANCH_REMOTE /* ### ? */);
}

GitRefSpec ^GitBranch::AsRefSpec()
{
    return gcnew GitRefSpec(Name, RemoteUpstreamName);
}

bool GitBranch::RecordAsHeadBranch(GitClientArgs ^args)
{
    GitPool pool(Repository->Pool);

    GIT_THROW(git_repository_set_head(Repository->Handle, pool.AllocString(Name)));

    return true;
}

GitBranchCollection^ GitRepository::Branches::get()
{
    if (! _branches)
        _branches = gcnew GitBranchCollection(this);

    return _branches;
}

GitReference^ GitBranch::Reference::get()
{
    if (!_reference && !_repository->IsDisposed)
        _reference = gcnew GitReference(_repository, Name);

    return _reference;
}

IEnumerator<GitBranch^>^ GitBranchCollection::GetEnumerator()
{
    IEnumerable<GitBranch^>^ walker = GetEnumerable((git_branch_t)(GIT_BRANCH_LOCAL | GIT_BRANCH_REMOTE));

    return walker->GetEnumerator();
}

IEnumerable<GitBranch^>^ GitBranchCollection::Local::get()
{
    return GetEnumerable(GIT_BRANCH_LOCAL);
}

IEnumerable<GitBranch^>^ GitBranchCollection::Remote::get()
{
    return GetEnumerable(GIT_BRANCH_REMOTE);
}

IEnumerable<GitBranch^>^ GitBranchCollection::GetEnumerable(git_branch_t flags)
{
    List<GitBranch^> ^branches = gcnew List<GitBranch^>();
    if (_repository->IsDisposed)
        return branches->AsReadOnly();

    git_branch_iterator *iter;
    int r = git_branch_iterator_new(&iter, _repository->Handle, flags);
    if (r != 0)
    {
        (gcnew GitNoArgs())->HandleGitError(this, r);
        return branches;
    }

    git_reference *ref;
    git_branch_t branch_type;
    while (! git_branch_next(&ref, &branch_type, iter))
    {
        GitReference ^gr = gcnew GitReference(_repository, ref);

        branches->Add(gcnew GitBranch(_repository, gr, branch_type));
    }
    git_branch_iterator_free(iter);

    return branches->AsReadOnly();
}

bool GitBranchCollection::Create(GitCommit^ commit, String^ name)
{
    GitBranch ^ignored;
    return Create(commit, name, gcnew GitNoArgs(), ignored);
}

bool GitBranchCollection::Create(GitCommit^ commit, String^ name, GitClientArgs^ args)
{
    GitBranch ^ignored;
    return Create(commit, name, args, ignored);
}

bool GitBranchCollection::Create(GitCommit^ commit, String^ name, [Out] GitBranch^% branch)
{
    return Create(commit, name, gcnew GitNoArgs(), branch);
}

bool GitBranchCollection::Create(GitCommit^ commit, String^ name, GitClientArgs^ args, [Out] GitBranch^% branch)
{
    if (_repository->IsDisposed)
        throw gcnew ObjectDisposedException("repository");
    else if (!commit)
        throw gcnew ArgumentNullException("commit");
    else if (String::IsNullOrEmpty(name))
        throw gcnew ArgumentNullException("name");
    else if(commit->IsDisposed || commit->Repository != _repository)
        throw gcnew ArgumentOutOfRangeException("commit");

    GitPool pool(_repository->Pool);
    git_reference *result;

    const char *p_name = pool.AllocString(name);

    const char *slash = strrchr(p_name, '/');

    if (slash)
      p_name = slash + 1;

    int r = git_branch_create(&result, _repository->Handle, p_name, commit->Handle, FALSE /* ### force */);

    if (!r)
        branch = gcnew GitBranch(_repository, name, gcnew GitReference(_repository, result));
    else
        branch = nullptr;

    return args->HandleGitError(this, r);
}

bool GitBranchCollection::TryGet(String^ name, [Out]GitBranch ^%branch)
{
    for each(GitBranch ^b in this)
    {
        if (b->Name == name)
        {
            branch = b;
            return true;
        }
    }
    return false;
}

#include "UnmanagedStructs.h"
