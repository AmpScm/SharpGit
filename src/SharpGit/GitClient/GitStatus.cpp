#include "stdafx.h"
#include "GitClient.h"
#include "GitStatusArgs.h"
#include "Plumbing/GitIndex.h"
#include "Plumbing/GitRepository.h"

using namespace SharpGit;
using namespace SharpGit::Implementation;
using namespace SharpGit::Plumbing;

GitStatusEventArgs::GitStatusEventArgs(const char *path, const char *wcPath, bool directory, const git_status_entry *entry, const git_index_entry *conflict_stages[3], GitStatusArgs ^args, Implementation::GitPool ^pool)
{
    UNUSED(args);
    _entry = entry;
    _path = path;
    _wcPath = wcPath;
    _directory = directory;
    _status = entry ? entry->status : 0;
    _pool = pool;
    _conflicted = GitConflicted::None;

    _has = GitStatusHas::None;
    if (!entry)
        _has = _has | GitStatusHas::Dummy;

    if (directory)
        _has = _has | GitStatusHas::Directory;
    else
        _has = _has | GitStatusHas::File;

    if (entry && entry->head_to_index)
        _has = _has | GitStatusHas::HeadToIndex;
    if (entry && entry->index_to_workdir)
        _has = _has | GitStatusHas::IndexToWorking;

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
    if (this->IncludeUnversioned && this->IncludeDirectories)
        opts->flags |= GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS;
    if (this->IncludeIgnored && this->IncludeDirectories)
        opts->flags |= GIT_STATUS_OPT_RECURSE_IGNORED_DIRS;
    if (this->IncludeUnreadable)
        opts->flags |= GIT_STATUS_OPT_INCLUDE_UNREADABLE;

    if (!String::IsNullOrEmpty(path))
    {
        opts->pathspec = *pool->AllocStringArray(path);

        if (! this->UseGlobPath)
            opts->flags |= GIT_STATUS_OPT_DISABLE_PATHSPEC_MATCH;
    }
    else
        opts->flags |= GIT_STATUS_OPT_DISABLE_PATHSPEC_MATCH;

    if (this->SortCaseInsensitive)
        opts->flags |= GIT_STATUS_OPT_SORT_CASE_INSENSITIVELY;
    else
        opts->flags |= GIT_STATUS_OPT_SORT_CASE_SENSITIVELY;

    if (this->FindRenames)
    {
        opts->flags |= GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX;
        opts->flags |= GIT_STATUS_OPT_RENAMES_INDEX_TO_WORKDIR;
        opts->flags |= GIT_STATUS_OPT_RENAMES_FROM_REWRITES;
    }

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

#pragma region STATUS

struct Git_status_data
{
    GitRoot<GitStatusArgs^> Args;
    const char *wcPath;
    apr_hash_t *walk_conflicts;
    git_index *index;

public:
    Git_status_data(const char *pPath, GitStatusArgs^ args, GitPool ^ pool)
      : Args(args, pool), wcPath(pPath), walk_conflicts(nullptr), index(nullptr)
    {}
};

static int on_status(bool directory, const char *path, const git_status_entry *status, const Git_status_data& data)
{
    GitPool pool(data.Args.GetPool());
    GitStatusEventArgs^ ee;

    if (!data.walk_conflicts)
        ee = gcnew GitStatusEventArgs(path, data.wcPath, directory, status, nullptr, data.Args, %pool);
    else
    {
        const git_index_entry *stages[3];

        GIT_THROW(git_index_conflict_get(&stages[0], &stages[1], &stages[2], data.index, path));

        ee = gcnew GitStatusEventArgs(path, data.wcPath, directory, status, stages, data.Args, %pool);
        svn_hash_sets(data.walk_conflicts, path, nullptr);
    }

    try
    {

        data.Args->OnStatus(ee);
    }
    finally
    {
        ee->Detach(false);
    }

    return 0;
}

static void add_one_component(svn_stringbuf_t *sb, const char *dir)
{
    const char *p = svn_relpath_skip_ancestor(sb->data, dir);
    const char *pN = (p && *p) ? strchr(p+1, '/') : NULL;

    if (sb->len)
        svn_stringbuf_appendbyte(sb, '/');

    if (pN)
        svn_stringbuf_appendbytes(sb, p, pN-p);
    else
        svn_stringbuf_appendcstr(sb, p);
}

bool GitRepository::Status(String ^path, GitStatusArgs ^args, EventHandler<GitStatusEventArgs^>^ handler)
{
    if (! path)
        throw gcnew ArgumentNullException("path");
    else if (!args)
        throw gcnew ArgumentNullException("args");

    if (handler)
        args->Status += handler;

    try
    {
        GitPool pool(Pool);
        Git_status_data data(svn_dirent_internal_style(git_repository_workdir(_repository), pool.Handle), args, %pool);

        GitIndex ^idx = Index;

        if (idx->HasConflicts)
        {
            git_index_conflict_iterator *it;
            const git_index_entry *ancestor, *our, *their;
            int r;

            data.index = idx->Handle;
            GIT_THROW(git_index_conflict_iterator_new(&it, data.index));
            data.walk_conflicts = apr_hash_make(pool.Handle);

            while (0 == (r = git_index_conflict_next(&ancestor, &our, &their, it)))
            {
                const git_index_entry *ii = ancestor ? ancestor : (our ? our : their);

                const char *name = apr_pstrdup(pool.Handle, ii->path);

                svn_hash_sets(data.walk_conflicts, name, name);
            }

            git_index_conflict_iterator_free(it);
        }

        git_status_list *status;
        GIT_THROW(git_status_list_new(&status, Handle, args->MakeOptions(path, %pool)));

        try
        {
            GitPool iterpool(%pool);
            svn_stringbuf_t *sb = nullptr;
            svn_stringbuf_t *dir_tmp = nullptr;
            const bool introduce_dirs = (args->GenerateVersionedDirs && args->IncludeUnmodified);

            if (introduce_dirs)
            {
                sb = svn_stringbuf_create(pool.AllocDirent(path), pool.Handle);

                if (String::IsNullOrEmpty(path))
                    GIT_THROW(on_status(true, "", nullptr, data));
                else
                    svn_path_remove_component(sb);
            }

            for (size_t i = 0, cnt = git_status_list_entrycount(status); i < cnt; i++)
            {
                const git_status_entry *status_entry = git_status_byindex(status, i);
                const char *raw_entry_path = status_entry->head_to_index
                                              ? status_entry->head_to_index->old_file.path
                                              : status_entry->index_to_workdir->old_file.path;
                size_t raw_entry_len = strlen(raw_entry_path);
                bool entry_is_dir = !raw_entry_len || (raw_entry_path[raw_entry_len-1] == '/');
                const char *entry_path;

                if (entry_is_dir)
                {
                    if (!dir_tmp)
                        dir_tmp = svn_stringbuf_create(raw_entry_path, pool.Handle);
                    else
                        svn_stringbuf_set(dir_tmp, raw_entry_path);

                    svn_stringbuf_chop(dir_tmp, 1);
                    entry_path = dir_tmp->data;
                }
                else
                    entry_path = raw_entry_path;

                if (introduce_dirs)
                {
                    const char *dir = svn_relpath_dirname(entry_path, iterpool.Handle);

                    if (strcmp(dir, sb->data))
                    {
                        while (sb->len && !svn_relpath_skip_ancestor(sb->data, dir))
                            svn_path_remove_component(sb);

                        while (*dir && *svn_relpath_skip_ancestor(sb->data, dir))
                        {
                            add_one_component(sb, dir);

                            GIT_THROW(on_status(true, sb->data, nullptr, data));
                        }
                        sb = sb;
                    }
                }

                GIT_THROW(on_status(entry_is_dir, entry_path, status_entry, data));
            }
        }
        finally
        {
            git_status_list_free(status);
        }

        if (args->IncludeConflicts && data.walk_conflicts && apr_hash_count(data.walk_conflicts))
        {
            for (apr_hash_index_t *hi = apr_hash_first(pool.Handle, data.walk_conflicts); hi; hi = apr_hash_next(hi))
            {
                const char *path = static_cast<const char*>(svn__apr_hash_index_key(hi));
                const git_index_entry *stages[3];

                GIT_THROW(git_index_conflict_get(&stages[0], &stages[1], &stages[2], data.index, path));

                args->OnStatus(gcnew GitStatusEventArgs(path, data.wcPath, false, nullptr, stages, args, %pool));
            }
        }

        return args->HandleGitError(this, 0);
    }
    finally
    {
        if (handler)
            args->Status -= handler;
    }
}

#pragma endregion STATUS

#include "UnmanagedStructs.h"
