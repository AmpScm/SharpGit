#include "stdafx.h"
#include "GitClient.h"
#include "GitStageArgs.h"

#include "Plumbing/GitRepository.h"
#include "Plumbing/GitIndex.h"

#include <private/svn_sorts_private.h>

using System::IO::FileInfo;
using namespace SharpGit;
using namespace SharpGit::Plumbing;

void GitClient::AssertNotBare(GitRepository ^repository)
{
    if (! repository)
        throw gcnew ArgumentNullException("repository");

    if (repository->IsBare)
        throw gcnew InvalidOperationException("Not valid on a bare repository");
}

void GitClient::AssertNotBare(GitRepository %repository)
{
    AssertNotBare(%repository);
}

bool GitClient::Add(String ^path)
{
    return Stage(path, gcnew GitStageArgs());
}

bool GitClient::Add(String ^path, GitStageArgs ^args)
{
    return Stage(path, args);
}

bool GitClient::Stage(String ^path)
{
	return Stage(path, gcnew GitStageArgs());
}

bool GitClient::Stage(String ^path, GitStageArgs^ args)
{
    if (String::IsNullOrEmpty(path))
        throw gcnew ArgumentNullException("path");

	return Stage(gcnew array<String^> { path }, args);
}

bool GitClient::Stage(IEnumerable<String^>^ paths)
{
	return Stage(paths, gcnew GitStageArgs());
}

static int
compare_strict(const void *a, const void *b)
{
  const char *item1 = *((const char * const *) a);
  const char *item2 = *((const char * const *) b);

  return strcmp(item1, item2);
}


bool GitClient::Stage(IEnumerable<String^>^ paths, GitStageArgs^ args)
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

    apr_array_header_t *c_paths = apr_array_make(pool.Handle, 16, sizeof(const char*));

    for each (String ^path in paths)
    {
        APR_ARRAY_PUSH(c_paths, const char*) = repo.MakeRelpath(path, %pool);
    }

    svn_sort__array(c_paths, compare_strict);

    git_index *index = repo.Index->Handle;
    int nPath = 0;
    size_t nIndex = 0;

    bool havePath = (nPath < c_paths->nelts);
    bool haveIndex = (nIndex < git_index_entrycount(index));
    const char *repo_abspath = nullptr;

    while (havePath)
    {
        if (haveIndex)
        {
            const git_index_entry *entry = git_index_get_byindex(index, nIndex);
            const char *c_path = APR_ARRAY_IDX(c_paths, nPath, const char*);

            int n = strcmp(entry->path, c_path);
            if (n == 0)
            {
                if (!repo_abspath)
                    repo_abspath = svn_dirent_internal_style(git_repository_workdir(repo.Handle), pool.Handle);

                const char *abspath = svn_dirent_join(repo_abspath, c_path, pool.Handle);
                svn_node_kind_t kind;

                SVN_THROW(svn_io_check_path(abspath, &kind, pool.Handle));

                if (kind != svn_node_none)
                {
                    if (!repo.Index->Add(c_path, args, %pool))
                        return false;
                    nIndex++;
                }
                else
                {
                    repo.Index->Remove(nIndex);
                    // Keep nIndex
                }
                nPath++;
            }
            else if (n < 0)
                nIndex++;
            else if (svn_relpath_skip_ancestor(c_path, entry->path))
            {
                nPath++;
            }
            else
            {
                if (!repo_abspath)
                    repo_abspath = svn_dirent_internal_style(git_repository_workdir(repo.Handle), pool.Handle);

                const char *abspath = svn_dirent_join(repo_abspath, c_path, pool.Handle);
                svn_node_kind_t kind;

                SVN_THROW(svn_io_check_path(abspath, &kind, pool.Handle));

                if (kind != svn_node_none)
                {
                    if (!repo.Index->Add(c_path, args, %pool))
                        return false;
                    nIndex++;
                }
                else
                {
                    repo.Index->Remove(nIndex);
                    nIndex--;
                }
                nPath++;
            }
        }
        else
        {
            const char *c_path = APR_ARRAY_IDX(c_paths, nPath, const char*);

            if (!repo.Index->Add(c_path, args, %pool))
                return false;
            else
                nIndex++; /* Now in index */
            nPath++;
        }

        havePath = (nPath < c_paths->nelts);
        haveIndex = (nIndex < git_index_entrycount(index));
    }

    return repo.Index->Write(args);
}
