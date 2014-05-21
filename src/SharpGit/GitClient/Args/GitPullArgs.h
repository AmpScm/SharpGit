#pragma once
#include "GitClientContext.h"
#include "GitFetchArgs.h"
#include "GitMergeArgs.h"

namespace SharpGit {

    public ref class GitPullArgs : public GitClientArgs
    {
        initonly GitFetchArgs ^_fetchArgs;
        initonly GitMergeArgs ^_mergeArgs;

    public:
        GitPullArgs(GitFetchArgs ^fetchArgs, GitMergeArgs ^mergeArgs)
        {
            _fetchArgs = fetchArgs ? fetchArgs : gcnew GitFetchArgs();
            _mergeArgs = mergeArgs ? mergeArgs : gcnew GitMergeArgs();
        }

        GitPullArgs()
        {
            _fetchArgs = gcnew GitFetchArgs();
            _mergeArgs = gcnew GitMergeArgs();
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


    };


}