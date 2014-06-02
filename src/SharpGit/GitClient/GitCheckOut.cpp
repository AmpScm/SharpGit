#include "stdafx.h"
#include "GitClient.h"
#include "GitCheckOutArgs.h"

#include "Plumbing/GitRepository.h"
#include "Plumbing/GitIndex.h"

using namespace SharpGit;
using namespace SharpGit::Plumbing;

git_checkout_options * GitCheckOutArgs::MakeCheckOutOptions(GitPool ^pool)
{
    git_checkout_options *opts = (git_checkout_options *)pool->Alloc(sizeof(*opts));

    git_checkout_init_options(opts, GIT_CHECKOUT_OPTIONS_VERSION);

    if (! DryRun)
    {
        opts->checkout_strategy |= GIT_CHECKOUT_SAFE;

        if (! SkipMissingFiles)
            opts->checkout_strategy |= GIT_CHECKOUT_SAFE_CREATE;

        if (OverwriteExisting)
            opts->checkout_strategy |= GIT_CHECKOUT_FORCE;

        if (! FailOnConflicts)
            opts->checkout_strategy |= GIT_CHECKOUT_ALLOW_CONFLICTS;

        if (RemoveUnversioned)
            opts->checkout_strategy |= GIT_CHECKOUT_REMOVE_UNTRACKED;

        if (RemoveIgnored)
            opts->checkout_strategy |= GIT_CHECKOUT_REMOVE_IGNORED;

        if (UpdateOnly)
            opts->checkout_strategy |= GIT_CHECKOUT_UPDATE_ONLY;

        // GIT_CHECKOUT_DISABLE_PATHSPEC_MATCH

        // Defined but unimplemented:
        // GIT_CHECKOUT_SKIP_UNMERGED
        // GIT_CHECKOUT_UPDATE_SUBMODULES
        // GIT_CHECKOUT_UPDATE_SUBMODULES_IF_CHANGED

        switch (ConflictStrategy)
        {
            case GitCheckoutConflictStrategy::UseOurs:
                opts->checkout_strategy |= GIT_CHECKOUT_USE_OURS;
                break;
            case GitCheckoutConflictStrategy::UseTheirs:
                opts->checkout_strategy |= GIT_CHECKOUT_USE_THEIRS;
                break;

            case GitCheckoutConflictStrategy::Diff3:
                opts->checkout_strategy |= GIT_CHECKOUT_CONFLICT_STYLE_DIFF3;
                break;

            case GitCheckoutConflictStrategy::Merge:
                opts->checkout_strategy |= GIT_CHECKOUT_CONFLICT_STYLE_MERGE;
                break;

            default:
                break; // Will be retrieved from config.
        }
    }

    if (this->NoCacheUpdate)
        opts->checkout_strategy |= GIT_CHECKOUT_DONT_UPDATE_INDEX;

    if (this->NoRefresh)
        opts->checkout_strategy |= GIT_CHECKOUT_NO_REFRESH;

    return opts;
}
