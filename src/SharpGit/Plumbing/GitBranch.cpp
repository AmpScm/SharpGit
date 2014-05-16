#include "stdafx.h"

#include "GitCommit.h"
#include "GitRepository.h"
#include "GitTree.h"
#include "GitReference.h"
#include "GitBranch.h"
#include "GitRefSpec.h"

using namespace SharpGit::Plumbing;
using namespace SharpGit;


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
        return nullptr;

    if (git_branch_remote_name(&buf, _repository->Handle, pool.AllocString(Name)))
    {
        giterr_clear();
        return nullptr;
    }

    String ^result = GitBase::Utf8_PtrToString(buf.ptr, buf.size);

    return result;
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

String ^ GitBranch::UpstreamName::get()
{
    if (!_upstreamName && !_resolvedUpstreamName)
    {
        _resolvedUpstreamName = true;

        GitPool pool(_repository->Pool);
        Git_buf result;

        if (!git_branch_upstream_name(&result, _repository->Handle, pool.AllocString(Name)))
        {
            _upstreamName = GitBase::Utf8_PtrToString(result.ptr, result.size);
        }
        else
            giterr_clear();
    }

    return _upstreamName;
}


bool GitBranch::IsTracking::get()
{
    if (IsRemote || !UpstreamName)
        return false;

    return (TrackedBranch != nullptr);
}

GitBranch ^ GitBranch::TrackedBranch::get()
{
    if (!IsRemote)
        return nullptr;

    String ^name = UpstreamName;
    if (!name)
        return nullptr;

    GitBranch ^result;
    if (_repository->Branches->TryGet(Name, result))
        return result;

    return gcnew GitBranch(_repository, name, GIT_BRANCH_REMOTE /* ### ? */);
}

GitRefSpec ^GitBranch::AsRefSpec()
{
    return gcnew GitRefSpec(Name, UpstreamName);
}

bool GitBranchCollection::TryGet(String^ name, [Out]GitBranch ^%branch)
{
    for each(GitBranch ^b in this)
    {
        if (branch->Name == name)
        {
            branch = b;
            return true;
        }
    }
    return false;
}
