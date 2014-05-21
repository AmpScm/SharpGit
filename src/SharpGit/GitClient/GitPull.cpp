#include "stdafx.h"
#include "GitClient.h"
#include "GitDelete.h"

#include "Args/GitPullArgs.h"

#include "Plumbing/GitRepository.h"
#include "Plumbing/GitRemote.h"
#include "Plumbing/GitBranch.h"

using namespace System;
using namespace SharpGit;
using namespace SharpGit::Plumbing;

bool GitClient::Pull(String ^localRepository)
{
    return Pull(localRepository, gcnew GitPullArgs());
}

bool GitClient::Pull(String ^localRepository, GitPullArgs ^args)
{
    GitRepository repo;

    if (!repo.Locate(localRepository, args))
        return false;

    GitBranch ^current = repo.HeadBranch;
    if (! current || !current->IsTracking)
        throw gcnew InvalidOperationException("There is no tracking information for the current branch.");

    GitRemote ^rm;
    if (! current->RemoteName
        || !repo.Remotes->TryGet(current->RemoteName, rm))
    {
        throw gcnew InvalidOperationException("No upstream remote for the current branch.");
    }

    try
    {
        rm->SetCallbacks(get_callbacks());
        rm->Connect(true, args->FetchArgs);
        rm->Download(args->FetchArgs);
        rm->UpdateTips(args->FetchArgs);
        rm->Disconnect(args->FetchArgs);
    }
    finally
    {
        delete rm;
    }

    GitMergeResult ^mr;
    if (!repo.Merge(repo.LastFetchResult, args->MergeArgs, mr))
        return false;
}
