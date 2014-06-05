#pragma once
#include "GitClientContext.h"
#include "GitMergeArgs.h"

namespace SharpGit {
    public ref class GitRevertCommitArgs : public GitMergeArgs
    {
    internal:
        const git_revert_options * AllocRevertOptions(GitPool ^pool);

    };
}