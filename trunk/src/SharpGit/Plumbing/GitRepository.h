#pragma once
#include "GitClientContext.h"
#include "../GitClient/GitClientEventArgs.h"

namespace SharpGit {
    ref class GitInitArgs;
    ref class GitStatusArgs;
    ref class GitCommitArgs;
    ref class GitCheckOutArgs;
    ref class GitMergeArgs;
    ref class GitMergeResult;
    ref class GitMergeAnalysis;
    ref class GitStashArgs;
    ref class GitResetArgs;

    namespace Plumbing {
        ref class GitBranch;

        ref class GitConfiguration;
        ref class GitIndex;
        ref class GitObjectDatabase;
        ref class GitTree;
        ref class GitCommit;
        ref class GitReference;
        ref class GitObject;
        ref class GitBranchCollection;
        ref class GitReferenceCollection;
        ref class GitRemoteCollection;

        public enum class GitRepositoryState
        {
            None = 0, // GIT_REPOSITORY_STATE_NONE
            Merge = GIT_REPOSITORY_STATE_MERGE,
            Revert = GIT_REPOSITORY_STATE_REVERT,
            CherryPick = GIT_REPOSITORY_STATE_CHERRY_PICK,
            Bisect = GIT_REPOSITORY_STATE_BISECT,
            Rebase = GIT_REPOSITORY_STATE_REBASE,
            RebaseInteractive = GIT_REPOSITORY_STATE_REBASE_INTERACTIVE,
            RebaseMerge = GIT_REPOSITORY_STATE_REBASE_MERGE,
            MailBox = GIT_REPOSITORY_STATE_APPLY_MAILBOX,
            MailBoxOrRebase = GIT_REPOSITORY_STATE_APPLY_MAILBOX_OR_REBASE
        };

        public ref class GitMergeDescription : public Implementation::GitBase
        {
        internal:
            git_merge_head *Alloc(GitPool ^pool);

            initonly GitRepository ^_repository;
            initonly String ^_branchName;
            initonly GitReference ^_reference;
            initonly GitId ^_id;
            initonly System::Uri ^_uri;

        internal:
            GitMergeDescription(GitReference ^reference);
            GitMergeDescription(GitRepository ^repository, String ^branchName, System::Uri ^url, GitId ^id);
            GitMergeDescription(GitRepository ^repository, GitId ^id);

        public:
            property GitId ^ Id
            {
                GitId^ get()
                {
                    return _id;
                }
            }

            property String ^ BranchName
            {
                String ^ get()
                {
                    return _branchName;
                }
            }

            property GitReference ^Reference
            {
                GitReference ^ get()
                {
                    return _reference;
                }
            }

            property System::Uri ^ Uri
            {
                System::Uri ^ get()
                {
                    return _uri;
                }
            }
        };

        public ref class GitRepository : public Implementation::GitBase
        {
        private:
            git_repository *_repository;
            GitPool _pool;
            GitBranchCollection^ _branches;
            GitReferenceCollection^ _references;
            GitRemoteCollection^ _remotes;

            GitRepository(git_repository *repository)
            {
                if (! repository)
                    throw gcnew ArgumentNullException("repository");

                _repository = repository;
            }

            !GitRepository()
            {
                if (_repository)
                {
                    try
                    {
                        ClearReferences();
                        git_repository_free(_repository);
                    }
                    finally
                    {
                        _repository = nullptr;
                    }
                }
            }

            ~GitRepository()
            {
                try
                {
                    ClearReferences();
                    git_repository_free(_repository);
                }
                finally
                {
                    _repository = nullptr;
                }
            }

        internal:
            bool Locate(String ^path, GitArgs ^args);
            bool Open(String ^repositoryPath, GitArgs ^args);

            void AssertNotOpen()
            {
                if (_repository)
                    throw gcnew InvalidOperationException();
            }

            void AssertOpen()
            {
                if (! _repository)
                    throw gcnew ObjectDisposedException("repository");
            }

            property git_repository* Handle
            {
                git_repository* get()
                {
                    if (IsDisposed)
                        throw gcnew ObjectDisposedException("repository");

                    return _repository;
                }
            }

            property GitPool^ Pool
            {
                GitPool^ get()
                {
                    return %_pool;
                }
            }

        public:
            property bool IsDisposed
            {
                bool get()
                {
                    return !_repository;
                }
            }

        public:
            /// <summary>Creates an unopened repository</summary>
            GitRepository() {}

            /// <summary>Creates an unopened repository and then calls Open(REPOSITORYPATH)</summary>
            GitRepository(String^ repositoryPath)
            {
                if (!Open(repositoryPath))
                    throw gcnew InvalidOperationException();
            }

        public:
            /// <summary>Opens the repository containing PATH</summary>
            bool Locate(String ^path);

            /// <summary>Opens the repository at REPOSITORYPATH</summary>
            bool Open(String ^repositoryPath);

            bool CreateAndOpen(String ^repositoryPath);
            bool CreateAndOpen(String ^repositoryPath, GitInitArgs ^args);

            static GitRepository^ Create(String ^repositoryPath);
            static GitRepository^ Create(String ^repositoryPath, GitInitArgs ^args);

        public:
            generic<typename T> where T : GitObject
            bool Lookup(GitId ^id, [Out] T% object);
            generic<typename T> where T : GitObject
            bool Lookup(GitId ^id, GitArgs ^args, [Out] T% object);

            bool Lookup(GitId ^id, GitObjectKind kind, [Out] GitObject^% object);
            bool Lookup(GitId ^id, GitObjectKind kind, GitArgs ^args, [Out] GitObject^% object);

            bool LookupViaPrefix(String ^idPrefix, [Out] GitId^% id);
            bool LookupViaPrefix(String ^idPrefix, [Out] GitObject^% object);

            bool Commit(GitTree ^tree, ICollection<GitCommit^> ^parents, GitCommitArgs ^args);
            bool Commit(GitTree ^tree, ICollection<GitCommit^> ^parents, GitCommitArgs ^args, [Out] GitId^% id);

            bool Stash(GitStashArgs ^args);
            bool Stash(GitStashArgs ^args, [Out] GitId ^%stashId);

            bool Reset(GitResetArgs ^args);

            bool CheckOut(GitTree^ tree);
            bool CheckOut(GitTree^ tree, GitCheckOutArgs^ args);

            /// <summary>Remove all the metadata associated with an ongoing command like merge, revert, cherry-pick, etc.</summary>
            bool CleanupState();
            /// <summary>Remove all the metadata associated with an ongoing command like merge, revert, cherry-pick, etc.</summary>
            bool CleanupState(GitArgs ^args);


            bool Merge(ICollection<GitMergeDescription^> ^description, GitMergeArgs ^args, [Out] GitMergeResult ^%mergeResult);

            bool MergeAnalysis(ICollection<GitMergeDescription^> ^description, GitMergeArgs ^args, [Out] GitMergeAnalysis ^%mergeAnalysis);

        public:
            property bool IsEmpty
            {
                bool get();
            }

            property bool IsBare
            {
                bool get();
            }

            property bool IsShallow
            {
                bool get();
            }

            property String^ RepositoryDirectory
            {
                String^ get();
            }

            property String^ WorkingCopyDirectory
            {
                String^ get();
                void set(String ^value);
            }

        public:
            property GitReference^ HeadReference
            {
                GitReference^ get();
            }

            property GitBranch^ HeadBranch
            {
                GitBranch^ get();
            }

            property bool IsHeadDetached
            {
                bool get()
                {
                    if (IsDisposed)
                        return false;

                    return (git_repository_head_detached(_repository) == 1);
                }
            }

            property bool IsHeadUnborn
            {
                bool get()
                {
                    if (IsDisposed)
                        return false;

                    return (git_repository_head_unborn(_repository) == 1);
                }
            }

            property GitBranchCollection^ Branches
            {
                GitBranchCollection^ get();
            }

            property GitReferenceCollection^ References
            {
                GitReferenceCollection^ get();
            }

            property GitRemoteCollection^ Remotes
            {
                GitRemoteCollection^ get();
            }

        public:
            bool ResolveReference(String^ referenceName, [Out] GitId ^%id);
            bool ResolveReference(GitReference^ reference, [Out] GitId ^%id);

        internal:
            const char *MakeRelpath(String ^path, GitPool ^pool);

            property ICollection<GitMergeDescription^> ^ LastFetchResult
            {
                ICollection<GitMergeDescription^> ^ get();
            }


        public:
            String ^MakeRelativePath(String ^path);

        private:
            GitConfiguration ^_configuration;
            GitIndex ^_indexRef;
            GitObjectDatabase ^_dbRef;
            void ClearReferences();

        public:
            property GitConfiguration^ Configuration
            {
                GitConfiguration^ get();
            }

            property GitIndex^ Index
            {
                GitIndex^ get()
                {
                    if (!_indexRef && _repository)
                        _indexRef = GetIndexInstance();

                    return _indexRef;
                }
            }

            property GitObjectDatabase^ ObjectDatabase
            {
                GitObjectDatabase^ get()
                {
                    if (!_dbRef && _repository)
                        _dbRef = GetObjectDatabaseInstance();

                    return _dbRef;
                }
            }

            property GitRepositoryState RepositoryState
            {
                GitRepositoryState get()
                {
                    if (IsDisposed)
                        return GitRepositoryState::None;

                    return (GitRepositoryState)git_repository_state(_repository);
                }
            }

            property IEnumerable<GitId^>^ MergeHeads
            {
                IEnumerable<GitId^>^ get();
            }

        public:
            bool OpenTree(GitId^ id, [Out] GitTree ^%tree);
            bool OpenTree(GitId^ id, GitArgs^ args, [Out] GitTree ^%tree);

        private:
            GitIndex^ GetIndexInstance();
            GitObjectDatabase^ GetObjectDatabaseInstance();

        public:
            bool Status(String ^path, GitStatusArgs ^args, EventHandler<GitStatusEventArgs^>^ handler);
        };


    };
}
