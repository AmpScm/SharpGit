#include "pch.h"
#include "GitClient.h"

#include "Plumbing/GitRepository.h"
#include "Plumbing/GitCommit.h"

#include "GitRevertCommitArgs.h"

using namespace SharpGit;
using namespace SharpGit::Plumbing;


const git_revert_options * GitRevertCommitArgs::AllocRevertOptions(GitPool ^pool)
{
    git_revert_options *ro = (git_revert_options *)pool->Alloc(sizeof(*ro));

    GIT_THROW(git_revert_init_options(ro, GIT_REVERT_OPTIONS_VERSION));

    ro->merge_opts = *this->AllocMergeOptions(pool);
    ro->checkout_opts = *this->AllocCheckOutOptions(pool);

    return ro;
}

bool GitClient::RevertCommit(String ^localRepository, GitId ^commitId)
{
    return RevertCommit(localRepository, commitId, gcnew GitRevertCommitArgs());
}

bool GitClient::RevertCommit(String ^localRepository, GitId ^commitId, GitRevertCommitArgs ^args)
{
    if (String::IsNullOrEmpty(localRepository))
        throw gcnew ArgumentNullException("localRepository");
    else if (! args)
        throw gcnew ArgumentNullException("args");

    GitRepository repo;

    if (!repo.Locate(localRepository, args))
        return false;

    GitCommit ^commit;
    if (repo.Lookup(commitId, commit))
    {
        return repo.RevertCommit(commit, args);
    }
    else
        throw gcnew ArgumentOutOfRangeException("commitId");
}
