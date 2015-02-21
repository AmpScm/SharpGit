#pragma once
#include "GitClientContext.h"
#include "GitCommitArgs.h"
#include "GitFetchArgs.h"
#include "GitMergeArgs.h"

namespace SharpGit {

    public ref class GitPullArgs : public GitClientArgs
    {
        initonly GitFetchArgs ^_fetchArgs;
        initonly GitMergeArgs ^_mergeArgs;
        initonly GitCommitArgs ^_commitArgs;
        bool _noCommitOnSuccess;

    public:
        GitPullArgs(GitFetchArgs ^fetchArgs, GitMergeArgs ^mergeArgs, GitCommitArgs ^commitArgs)
        {
            _fetchArgs = fetchArgs ? fetchArgs : gcnew GitFetchArgs();
            _mergeArgs = mergeArgs ? mergeArgs : gcnew GitMergeArgs();
            _commitArgs = commitArgs ? commitArgs : gcnew GitCommitArgs();
        }

        GitPullArgs()
        {
            _fetchArgs = gcnew GitFetchArgs();
            _mergeArgs = gcnew GitMergeArgs();
            _commitArgs = gcnew GitCommitArgs();
        }

    public:
        property GitFetchArgs ^ FetchArgs
        {
            GitFetchArgs ^ get() { return _fetchArgs; }
        }

        property GitMergeArgs ^ MergeArgs
        {
            GitMergeArgs ^ get() { return _mergeArgs; }
        }

        property GitCommitArgs ^ CommitArgs
        {
            GitCommitArgs ^ get() { return _commitArgs; }
        }

        property bool CommitOnSuccess
        {
            bool get()
            {
                return !_noCommitOnSuccess;
            }
            void set(bool value)
            {
                _noCommitOnSuccess = !value;
            }
        }


    };


}