#include "stdafx.h"
#include "GitClient.h"
#include "GitStatusArgs.h"
#include "Plumbing/GitRepository.h"

using namespace SharpGit;
using namespace SharpGit::Plumbing;

GitStatusEventArgs::GitStatusEventArgs(const char *path, const char *wcPath, unsigned status, const git_index_entry *conflict_stages[3], GitStatusArgs ^args, Implementation::GitPool ^pool)
{
    UNUSED(args);
    _path = path;
    _wcPath = wcPath;
    _status = status;
    _pool = pool;
    _conflicted = GitConflicted::None;
    if (conflict_stages)
    {
        if (conflict_stages[0])
            _conflicted = (_conflicted | GitConflicted::HasAncestor);
        if (conflict_stages[1])
            _conflicted = (_conflicted | GitConflicted::HasMine);
        if (conflict_stages[2])
            _conflicted = (_conflicted | GitConflicted::HasTheirs);
    }
}

const git_status_options* GitStatusArgs::MakeOptions(String^ path, Implementation::GitPool ^pool)
{
    git_status_options *opts = (git_status_options *)pool->Alloc(sizeof(*opts));

    git_status_init_options(opts, GIT_STATUS_OPTIONS_VERSION);

    if (! this->WorkingDirectoryStatus)
        opts->show = GIT_STATUS_SHOW_INDEX_ONLY;
    else if (! this->IndexStatus)
        opts->show = GIT_STATUS_SHOW_WORKDIR_ONLY;
    else
        opts->show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;

    opts->flags = 0;
    if (this->IncludeUnversioned)
        opts->flags |= GIT_STATUS_OPT_INCLUDE_UNTRACKED;
    if (this->IncludeIgnored)
        opts->flags |= GIT_STATUS_OPT_INCLUDE_IGNORED;
    if (this->IncludeUnmodified)
        opts->flags |= GIT_STATUS_OPT_INCLUDE_UNMODIFIED;
    if (! this->IncludeSubmodules)
        opts->flags |= GIT_STATUS_OPT_EXCLUDE_SUBMODULES;
    if (this->IncludeUnversionedRecursive)
        opts->flags |= GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS;
    if (this->IncludeIgnoredRecursive)
        opts->flags |= GIT_STATUS_OPT_RECURSE_IGNORED_DIRS;

    if (!String::IsNullOrEmpty(path))
    {
        opts->pathspec = *pool->AllocStringArray(path);
    }
    else
        opts->flags |= GIT_STATUS_OPT_DISABLE_PATHSPEC_MATCH;

    if (this->SortCaseInsensitive)
        opts->flags |= GIT_STATUS_OPT_SORT_CASE_INSENSITIVELY;
    else
        opts->flags |= GIT_STATUS_OPT_SORT_CASE_SENSITIVELY;

    opts->flags |= GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX;
    opts->flags |= GIT_STATUS_OPT_RENAMES_INDEX_TO_WORKDIR;

    //opts->flags |= GIT_STATUS_OPT_SORT_CASE_INSENSITIVELY;
    opts->flags |= GIT_STATUS_OPT_RENAMES_FROM_REWRITES;

    if (! NoCacheUpdate)
        opts->flags |= GIT_STATUS_OPT_UPDATE_INDEX;

    if (NoRefresh)
        opts->flags |= GIT_STATUS_OPT_NO_REFRESH;

    return opts;
}

bool GitClient::Status(String ^path, EventHandler<GitStatusEventArgs^>^ status)
{
    return GitClient::Status(path, gcnew GitStatusArgs(), status);
}

bool GitClient::Status(String ^path, GitStatusArgs ^args, EventHandler<GitStatusEventArgs^>^ status)
{
    GitRepository repo;

    if (!repo.Locate(path, args))
        return false;

    return repo.Status(repo.MakeRelativePath(path), args, status);
}
