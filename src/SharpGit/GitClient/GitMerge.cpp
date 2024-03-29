#include "pch.h"
#include "GitClient.h"

#include "GitMergeArgs.h"

#include "Plumbing/GitRepository.h"
#include "Plumbing/GitRemote.h"

using namespace System;
using namespace SharpGit;
using namespace SharpGit::Plumbing;

git_merge_options *GitMergeArgs::AllocMergeOptions(GitPool ^pool)
{
    git_merge_options *mo = (git_merge_options *)pool->Alloc(sizeof(*mo));

    GIT_THROW(git_merge_init_options(mo, GIT_MERGE_OPTIONS_VERSION));

    //mo->flags = (git_merge_flag_t)0;

    if (SkipFindingRenames)
        mo->flags = (git_merge_flag_t)((int)mo->flags & ~GIT_MERGE_FIND_RENAMES);
    else
        mo->flags = (git_merge_flag_t)((int)mo->flags | GIT_MERGE_FIND_RENAMES);

    mo->rename_threshold = RenameTreshold;
    mo->target_limit = RenameTargetLimit;

    mo->file_favor = GIT_MERGE_FILE_FAVOR_NORMAL;

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
