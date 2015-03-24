#include "stdafx.h"
#include "GitClient.h"
#include "GitUnstageArgs.h"

#include "Plumbing/GitRepository.h"
#include "Plumbing/GitIndex.h"
#include "Plumbing/GitCommit.h"
#include "Plumbing/GitTree.h"

#include <private/svn_sorts_private.h>

using namespace SharpGit;
using namespace SharpGit::Plumbing;

using namespace SharpGit;

bool GitClient::Unstage(String ^path)
{
	return Unstage(path, gcnew GitUnstageArgs());
}

bool GitClient::Unstage(String ^path, GitUnstageArgs^ args)
{
	return Unstage(gcnew array<String^> { path }, args);
}

bool GitClient::Unstage(IEnumerable<String^>^ paths)
{
	return Unstage(paths, gcnew GitUnstageArgs());
}

bool GitClient::Unstage(IEnumerable<String^>^ paths, GitUnstageArgs^ args)
{
    if (! paths)
        throw gcnew ArgumentNullException("path");
    else if (! args)
        throw gcnew ArgumentNullException("args");

    GitRepository repo;

    for each (String ^ p in paths)
    {
        if (!repo.Locate(p, args))
            return false;
    }

    if (repo.IsDisposed)
        return false;

    AssertNotBare(repo);

    GitPool pool(_pool);

    GitTree ^tree = nullptr;

    if (!repo.IsHeadUnborn)
    {
        GitId ^commitId;
        GitCommit ^commit;

        if (!repo.ResolveReference(repo.HeadReference, commitId)
            || !repo.Lookup<GitCommit^>(commitId, args, commit))
        {
          return false;
        }
        tree = commit->Tree;
    }

    git_index *index = repo.Index->Handle;
    git_tree *c_tree = tree ? tree->Handle : nullptr;
    for each (String ^path in paths)
    {
        const char *c_path = repo.MakeRelpath(path, %pool);
        const git_index_entry *i_entry = git_index_get_bypath(index, c_path, 0);
        git_tree_entry *t_entry;
        
        if (!c_tree || git_tree_entry_bypath(&t_entry, c_tree, c_path))
        {
            giterr_clear();
            t_entry = nullptr;
        }

        if (i_entry && !t_entry)
        {
            int r = git_index_remove(index, c_path, git_index_entry_stage(i_entry));
            if (r)
                args->HandleGitError(this, r);
        }
        else if (t_entry)
        {
            git_index_entry *new_entry = (git_index_entry *)pool.Alloc(sizeof(*new_entry));

            new_entry->path = c_path;
            new_entry->mode = git_tree_entry_filemode_raw(t_entry);
            new_entry->id = *git_tree_entry_id(t_entry);

            int r = git_index_add(index, new_entry);
            if (r)
                args->HandleGitError(this, r);
        }
    }

    return repo.Index->Write(args);
}
