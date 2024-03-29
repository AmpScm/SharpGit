#include "pch.h"

#include "GitRepository.h"
#include "GitClient/GitStatusArgs.h"
#include "GitClient/GitCommitArgs.h"
#include "GitClient/GitCheckOutArgs.h"
#include "GitClient/GitInitArgs.h"
#include "GitClient/GitMergeArgs.h"
#include "GitClient/GitResetArgs.h"
#include "GitClient/GitRevertCommitArgs.h"
#include "GitClient/GitStashArgs.h"

#include "GitConfiguration.h"
#include "GitIndex.h"
#include "GitObjectDatabase.h"
#include "GitObject.h"
#include "GitTree.h"
#include "GitBranch.h"
#include "GitCommit.h"
#include "GitReference.h"

using namespace System;
using namespace SharpGit;
using namespace SharpGit::Plumbing;
using namespace SharpGit::Implementation;
using System::Collections::Generic::List;

struct git_repository {};

void GitRepository::ClearReferences()
{
    try
    {
        if (_configuration)
            delete _configuration;
        if (_indexRef)
            delete _indexRef;
        if (_dbRef)
            delete _dbRef;
    }
    finally
    {
        _configuration = nullptr;
        _indexRef = nullptr;
        _dbRef = nullptr;
    }
}

bool GitRepository::Open(String ^repositoryPath)
{
    return Open(repositoryPath, gcnew GitNoArgs());
}

bool GitRepository::Open(String ^repositoryPath, GitArgs ^args)
{
    AssertNotOpen();

    if (String::IsNullOrEmpty(repositoryPath))
        throw gcnew ArgumentNullException("repositoryPath");
    else if (! args)
        throw gcnew ArgumentNullException("args");

    GitPool pool;

    git_repository *repository;
    int r = git_repository_open(&repository, pool.AllocDirent(repositoryPath));

    if (r)
        return args->HandleGitError(this, r);

    _repository = repository;
    return true;
}

bool GitRepository::Locate(String ^path)
{
    return Locate(path, gcnew GitNoArgs());
}

bool GitRepository::Locate(String ^path, GitArgs ^args)
{
    AssertNotOpen();

    if (String::IsNullOrEmpty(path))
        throw gcnew ArgumentNullException("path");
    else if (! args)
        throw gcnew ArgumentNullException("args");

    GitPool pool;

    const char *ceiling_dirs = nullptr;

    char *dirent = const_cast<char*>(pool.AllocDirent(path));

    {
        char *star = strchr(dirent, '*');
        char *q = strchr(dirent, '?');

        if (star || q)
        {
            q = (q && (q < star)) ? q : star;

            while (q > dirent && *q != '/')
              q--;

            *q = 0;
        }
    }

    svn_node_kind_t kind;
    SVN_THROW(svn_io_check_path(dirent, &kind, pool.Handle));
    while (kind == svn_node_none
           && !svn_dirent_is_root(dirent, strlen(dirent)))
    {
        dirent = svn_dirent_dirname(dirent, pool.Handle);
        SVN_THROW(svn_io_check_path(dirent, &kind, pool.Handle));
    }

    git_repository *repository;
    int r = git_repository_open_ext(&repository,
                                    dirent,
                                    GIT_REPOSITORY_OPEN_CROSS_FS,
                                    ceiling_dirs);

    if (r)
        return args->HandleGitError(this, r);

    _repository = repository;
    return true;
}

GitRepository^ GitRepository::Create(String ^repositoryPath)
{
    return Create(repositoryPath, gcnew GitInitArgs());
}

GitRepository^ GitRepository::Create(String ^repositoryPath, GitInitArgs ^args)
{
    GitRepository^ repo = gcnew GitRepository();
    bool ok = false;
    try
    {
        if (repo->CreateAndOpen(repositoryPath, args))
        {
            ok = true;
            return repo;
        }

        args->HandleException(gcnew InvalidOperationException());
        return nullptr;
    }
    finally
    {
        if (!ok)
            delete repo;
    }
}

bool GitRepository::CreateAndOpen(String ^repositoryPath)
{
    AssertNotOpen();
    return CreateAndOpen(repositoryPath, gcnew GitInitArgs());
}

bool GitRepository::CreateAndOpen(String ^repositoryPath, GitInitArgs ^args)
{
    AssertNotOpen();

    GitPool pool;
    git_repository *repository;
    int r = git_repository_init_ext(&repository, pool.AllocDirent(repositoryPath), args->MakeInitOptions(%pool));

    if (r)
        return args->HandleGitError(this, r);

    _repository = repository;
    return true;
}

bool GitRepository::IsEmpty::get()
{
    AssertOpen();

    return git_repository_is_empty(_repository) != 0;
}

bool GitRepository::IsBare::get()
{
    AssertOpen();

    return git_repository_is_bare(_repository) != 0;
}

bool GitRepository::IsShallow::get()
{
    AssertOpen();

    return git_repository_is_shallow(_repository) != 0;
}

String^ GitRepository::RepositoryDirectory::get()
{
    AssertOpen();

    const char *path = git_repository_path(_repository);

    return GitBase::StringFromDirentNoPool(path);
}

String^ GitRepository::WorkingCopyDirectory::get()
{
    AssertOpen();

    const char *path = git_repository_workdir(_repository);

    return GitBase::StringFromDirentNoPool(path);
}

void GitRepository::WorkingCopyDirectory::set(String ^value)
{
    AssertOpen();

    if (String::IsNullOrEmpty(value))
        throw gcnew ArgumentNullException("value");

    GitPool pool;

    int r = git_repository_set_workdir(_repository, pool.AllocDirent(value), FALSE);
    if (r)
    {
        (gcnew GitNoArgs())->HandleGitError(this, r);
        throw gcnew InvalidOperationException();
    }
}

GitConfiguration^ GitRepository::Configuration::get()
{
    if (IsDisposed)
        return nullptr;

    AssertOpen();

    git_config *config;
    int r = git_repository_config(&config, _repository);
    if (r)
    {
        (gcnew GitNoArgs())->HandleGitError(this, r);
        throw gcnew InvalidOperationException();
    }

    return _configuration = gcnew GitConfiguration(this, config);
}

GitIndex^ GitRepository::GetIndexInstance()
{
    AssertOpen();

    git_index *index;
    int r = git_repository_index(&index, _repository);
    if (r)
    {
        (gcnew GitNoArgs())->HandleGitError(this, r);
        throw gcnew InvalidOperationException();
    }

    return gcnew GitIndex(index);
}

// Cache and provide as property?
GitObjectDatabase^ GitRepository::GetObjectDatabaseInstance()
{
    AssertOpen();

    git_odb *odb;
    int r = git_repository_odb(&odb, _repository);
    if (r)
    {
        (gcnew GitNoArgs())->HandleGitError(this, r);
        throw gcnew InvalidOperationException();
    }

    return gcnew GitObjectDatabase(odb);
}

const char *GitRepository::MakeRelpath(String ^path, GitPool ^pool)
{
    if (String::IsNullOrEmpty(path))
        throw gcnew ArgumentNullException("path");
    else if (! pool)
        throw gcnew ArgumentNullException("pool");

    AssertOpen();

    const char *wrk_path = svn_dirent_canonicalize(
                                                            git_repository_workdir(_repository),
                                                            pool->Handle);
    const char *item_path = pool->AllocDirent(path);

    return svn_dirent_skip_ancestor(wrk_path, item_path);
}

GitReference^ GitRepository::HeadReference::get()
{
    if (IsDisposed)
        return nullptr;

    git_reference *ref;
    int r = git_repository_head(&ref, _repository);

    if (!r)
        return gcnew GitReference(this, ref);

    return nullptr;
}

bool GitRepository::ResolveReference(String^ referenceName, [Out] GitId ^%id)
{
    if (String::IsNullOrEmpty(referenceName))
        throw gcnew ArgumentNullException("referenceName");

    AssertOpen();

    GitPool pool;
    git_oid oid;
    int r = git_reference_name_to_id(&oid, _repository, pool.AllocString(referenceName));

    if (r == 0)
    {
        id = gcnew GitId(oid);
        return true;
    }
    else
    {
        id = nullptr;
        return false;
    }
}

bool GitRepository::ResolveReference(GitReference^ reference, [Out] GitId ^%id)
{
    if (! reference)
        throw gcnew ArgumentNullException("reference");

    AssertOpen();

    GitPool pool;
    git_reference *ref;
    int r = git_reference_resolve(&ref, reference->Handle);

    if (r == 0)
    {
        const git_oid *oid = git_reference_target(ref);
        id = gcnew GitId(oid);

        git_reference_free(ref);

        return true;
    }
    else
    {
        id = nullptr;
        return false;
    }
}

bool GitRepository::Commit(GitTree ^tree, ICollection<GitCommit^>^ parents, GitCommitArgs ^args)
{
    GitId ^ignored;

    return Commit(tree, parents, args, ignored);
}

bool GitRepository::Commit(GitTree ^tree, ICollection<GitCommit^>^ parents, GitCommitArgs ^args, [Out] GitId^% id)
{
    if (!tree)
        throw gcnew ArgumentNullException("tree");
    else if (!args)
        throw gcnew ArgumentNullException("args");

    AssertOpen();

    GitPool pool;

    const git_commit** commits = nullptr;
    int nCommits = 0;

    if (parents)
    {
        // Theoretical problem: External change of parents might change count
        commits = (const git_commit**)alloca(sizeof(git_commit*) * parents->Count);

        for each (GitCommit^ c in parents)
            commits[nCommits++] = c->Handle;
    }

    const char *msg = args->AllocLogMessage(%pool);

    git_oid commit_id;
    int r = git_commit_create(&commit_id, _repository,
                                                      pool.AllocString(args->UpdateReference),
                                                      args->Author->Alloc(this, %pool),
                                                      args->Signature->Alloc(this, %pool),
                                                      NULL /* utf-8 */,
                                                      msg,
                                                      tree->Handle,
                                                      nCommits, commits);

    if (! r)
        id = gcnew GitId(commit_id);
    else
        id = nullptr;

    return args->HandleGitError(this, r);
}

bool GitRepository::RevertCommit(GitCommit^ commit)
{
    return RevertCommit(commit, gcnew GitRevertCommitArgs());
}

bool GitRepository::RevertCommit(GitCommit^ commit, GitRevertCommitArgs ^args)
{
    if (!commit)
        throw gcnew ArgumentNullException("commit");
    else if (!args)
        throw gcnew ArgumentNullException("args");

    GitPool pool(Pool);

    return args->HandleGitError(this, git_revert(Handle, commit->Handle, args->AllocRevertOptions(%pool)));
}

bool GitRepository::CheckOut(GitTree ^tree)
{
    if (! tree)
        throw gcnew ArgumentNullException("tree");

    return CheckOut(tree, gcnew GitCheckOutArgs());
}

bool GitRepository::CheckOut(GitTree ^tree, GitCheckOutArgs ^args)
{
    if (! tree)
        throw gcnew ArgumentNullException("tree");
    else if (! args)
        throw gcnew ArgumentNullException("args");

    AssertOpen();
    GitPool pool(_pool);

    int r = git_checkout_tree(Handle, safe_cast<GitObject^>(tree)->Handle,
                              args->AllocCheckOutOptions(%pool));

    return args->HandleGitError(this, r);
}

bool GitRepository::CleanupState()
{
    return CleanupState(gcnew GitNoArgs());
}

bool GitRepository::CleanupState(GitArgs ^args)
{
    if (! args)
        throw gcnew ArgumentNullException("args");

    AssertOpen();
    int r = git_repository_state_cleanup(Handle);

    return args->HandleGitError(this, r);
}

static int __cdecl add_oid_cb(const git_oid *oid, void *baton)
{
    GitRoot<List<GitId^>^> root(baton);

    root->Add(gcnew GitId(oid));
    return 0;
}

IEnumerable<GitId^>^ GitRepository::MergeHeads::get()
{
    if (!IsDisposed)
    {
        List<GitId^>^ results = gcnew List<GitId^>();
        GitRoot<List<GitId^>^> root(results);

        int r = git_repository_mergehead_foreach(Handle, add_oid_cb, root.GetBatonValue());

        if (r == 0)
            return results->AsReadOnly();
    }

    return gcnew array<GitId^>(0);
}

String ^GitRepository::MakeRelativePath(String ^path)
{
    if (String::IsNullOrEmpty(path))
        throw gcnew ArgumentNullException("path");

    GitPool pool(_pool);

    return Utf8_PtrToString(MakeRelpath(path, %pool));
}

bool GitRepository::OpenTree(GitId^ id, [Out] GitTree ^%tree)
{
    if (!id)
        throw gcnew ArgumentNullException("id");

    return OpenTree(id, gcnew GitNoArgs(), tree);
}

bool GitRepository::OpenTree(GitId^ id, GitArgs^ args, [Out] GitTree ^%tree)
{
    if (!id)
        throw gcnew ArgumentNullException("id");
    else if (!args)
        throw gcnew ArgumentNullException("args");

    AssertOpen();

    git_oid oid = id->AsOid();
    git_tree *gtree;

    int r = git_tree_lookup(&gtree, _repository, &oid);
    if (!r)
        tree = gcnew GitTree(this, gtree);
    else
        tree = nullptr;

    return args->HandleGitError(this, r);
}

bool GitRepository::Stash(GitStashArgs ^args)
{
    GitId ^id;
    return Stash(args, id);
}

bool GitRepository::Stash(GitStashArgs ^args, [Out] GitId ^%stashId)
{
    if (!args)
        throw gcnew ArgumentNullException("args");

    GitPool pool(Pool);

    git_oid rslt;
    stashId = nullptr;

    unsigned flags = 0;

    if (args->KeepChanges)
        flags |= GIT_STASH_KEEP_INDEX;
    if (args->IncludeUntracked)
        flags |= GIT_STASH_INCLUDE_UNTRACKED;
    if (args->IncludeIgnored)
        flags |= GIT_STASH_INCLUDE_IGNORED;

    int r = git_stash_save(&rslt, Handle, args->Signature->Alloc(this, %pool), args->AllocLogMessage(%pool), flags);

    if (!r)
      stashId = gcnew GitId(rslt);

    return args->HandleGitError(this, r);
}

bool GitRepository::Reset(GitResetArgs ^args)
{
    if (!args)
        throw gcnew ArgumentNullException("args");

    git_reset_t mode;
    switch(args->Mode)
    {
        case GitResetMode::Default:
            mode = GIT_RESET_SOFT;
            break;
        case GitResetMode::Mixed:
            mode = GIT_RESET_MIXED;
            break;
        case GitResetMode::Hard:
            mode = GIT_RESET_HARD;
            break;
        default:
            throw gcnew InvalidOperationException();
    }

    GitPool pool(Pool);

    GitCommit ^commit;

    if (Lookup(HeadReference->TargetId, commit))
    {
        const git_object *target = Git_ToObject(commit->Handle);

        return args->HandleGitError(this, git_reset(Handle, const_cast<git_object*>(target), mode,
                                    args->AllocCheckOutOptions(%pool)));
    }
    else
        throw gcnew InvalidOperationException();
}

#pragma region MERGE

GitCommitInfo::GitCommitInfo(GitRepository ^repository, String ^branchName, System::Uri ^url, GitId ^id)
{
    if (!repository)
        throw gcnew ArgumentNullException("repository");
    else if (String::IsNullOrEmpty(branchName))
        throw gcnew ArgumentNullException("BranchName");
    else if (!url)
        throw gcnew ArgumentNullException("url");
    else if (!id)
        throw gcnew ArgumentNullException("id");

    _repository = repository;
    _branchName = branchName;
    _reference = nullptr;
    _id = id;
    _uri = url;
}

GitCommitInfo::GitCommitInfo(GitReference ^reference)
{
    if (!reference)
        throw gcnew ArgumentNullException("reference");

    _reference = reference;
    _repository = reference->Repository;
    _id = reference->TargetId;
}

GitCommitInfo::GitCommitInfo(GitRepository ^repository, GitId ^id)
{
    if (!repository)
        throw gcnew ArgumentNullException("repository");
    else if (!id)
        throw gcnew ArgumentNullException("id");

    _repository = repository;
    _id = id;
}

git_annotated_commit *GitCommitInfo::Alloc(GitPool ^pool)
{
  git_annotated_commit *mh;

  if (BranchName)
    GIT_THROW(git_annotated_commit_from_fetchhead(&mh, _repository->Handle,
                                                  pool->AllocString(BranchName),
                                                  pool->AllocString(Uri->AbsoluteUri),
                                                  &Id->AsOid()));
  else if (Reference)
    GIT_THROW(git_annotated_commit_from_ref(&mh, _repository->Handle, _reference->Handle));
  else
    GIT_THROW(git_annotated_commit_lookup(&mh, _repository->Handle, &Id->AsOid()));

  return mh;
}

ref class GitFetchHeadInfo
{
public:
    GitRepository ^Repository;
    List<GitCommitInfo^>^ List;
};

static int __cdecl foreach_fetchhead(const char *ref_name, const char *remote_url, const git_oid *oid, unsigned int is_merge, void *payload)
{
    if (!is_merge)
        return 0;

    GitRoot<GitFetchHeadInfo^> root(payload);
    GitBranch ^branch;

    root->List->Add(gcnew GitCommitInfo(root->Repository, GitBase::Utf8_PtrToString(ref_name), gcnew System::Uri(GitBase::Utf8_PtrToString(remote_url), UriKind::Absolute), gcnew GitId(oid)));

    return 0;
}

ICollection<GitCommitInfo^> ^ GitRepository::LastFetchResult::get()
{
    GitFetchHeadInfo ^fhi = gcnew GitFetchHeadInfo();
    fhi->List = gcnew List<GitCommitInfo^>();
    fhi->Repository = this;


    GitRoot<GitFetchHeadInfo^> root(fhi);

    GIT_THROW(git_repository_fetchhead_foreach(Handle, foreach_fetchhead, root.GetBatonValue()));

    return fhi->List->AsReadOnly();
}

bool GitRepository::Merge(ICollection<GitCommitInfo^> ^descriptions, GitMergeArgs ^args, [Out] GitMergeResult ^%mergeResult)
{
    GitPool pool(Pool);

    mergeResult = nullptr;

    const git_annotated_commit ** heads = (const git_annotated_commit **)pool.Alloc(sizeof(const git_annotated_commit*) * descriptions->Count);
    int headsAlloced = 0;
    try
    {
        for each(GitCommitInfo ^d in descriptions)
        {
            git_annotated_commit *mh = d->Alloc(%pool);

            if (!mh)
                throw gcnew InvalidOperationException();

            heads[headsAlloced++] = mh;
        }

        GIT_THROW(git_merge(Handle, heads, descriptions->Count, args->AllocMergeOptions(%pool), args->AllocCheckOutOptions(%pool)));
    }
    finally
    {
        while(headsAlloced)
        {
            git_annotated_commit_free(const_cast<git_annotated_commit*>(heads[--headsAlloced]));
        }
    }

    return true;
}

bool GitRepository::MergeAnalysis(ICollection<GitCommitInfo^> ^descriptions, GitMergeArgs ^args, [Out] GitMergeAnalysis ^%mergeAnalysis)
{
    GitPool pool(Pool);

    mergeAnalysis = nullptr;

    const git_annotated_commit ** heads = (const git_annotated_commit **)pool.Alloc(sizeof(const git_annotated_commit*) * descriptions->Count);
    int headsAlloced = 0;
    try
    {
        for each(GitCommitInfo ^d in descriptions)
        {
            git_annotated_commit *mh = d->Alloc(%pool);

            if (!mh)
                throw gcnew InvalidOperationException();

            heads[headsAlloced++] = mh;
        }

        git_merge_analysis_t analysis;
        git_merge_preference_t preference;
        GIT_THROW(git_merge_analysis(&analysis, &preference, Handle, heads, descriptions->Count));

        mergeAnalysis = gcnew GitMergeAnalysis(analysis, preference);
    }
    finally
    {
        while(headsAlloced)
        {
            git_annotated_commit_free(const_cast<git_annotated_commit*>(heads[--headsAlloced]));
        }
    }

    return true;
}
#pragma endregion MERGE

#include "UnmanagedStructs.h"
