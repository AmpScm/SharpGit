#include "stdafx.h"
#include "GitClient.h"
#include "GitDelete.h"

#include "Args/GitMergeArgs.h"

#include "Plumbing/GitRepository.h"
#include "Plumbing/GitRemote.h"

using namespace System;
using namespace SharpGit;
using namespace SharpGit::Plumbing;

git_merge_options *GitMergeArgs::AllocMergeOptions(GitPool ^pool)
{
    git_merge_options *mo = (git_merge_options *)pool->Alloc(sizeof(*mo));

    GIT_THROW(git_merge_init_options(mo, GIT_MERGE_OPTIONS_VERSION));

    return mo;
}


/*bool GitClient::MergeFetchHeads(String ^localRepository)
{
    return MergeFetchHeads(localRepository, gcnew GitMergeArgs());
}

bool MergeFetchHeads(String ^localRepository, GitMergeArgs ^args)
{
    return false;
}*/
