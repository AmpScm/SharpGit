#include "stdafx.h"

#include "GitCommit.h"
#include "GitRepository.h"
#include "GitTree.h"
#include "GitReference.h"
#include "GitBranch.h"

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
    git_buf buf = GIT_BUF_INIT_CONST("", 0);

    GitPool pool;

    if (!git_branch_remote_name(&buf, _repository->Handle, pool.AllocString(Name)))
    {
        String ^result = GitBase::Utf8_PtrToString(buf.ptr, buf.size);

        git_buf_free(&buf);

        return result;
    }

    return nullptr;
}
