#include "stdafx.h"
#include "GitClient.h"
#include "GitDelete.h"

#include "Args/GitPullArgs.h"

#include "Plumbing/GitRepository.h"
#include "Plumbing/GitConfiguration.h"
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
    bool initialCheckout = false;

    if (!repo.Locate(localRepository, args))
        return false;

    GitRemote ^rm;
    if (!repo.IsEmpty)
    {
        GitBranch ^current = repo.HeadBranch;
        if (! current || !current->IsTracking)
            throw gcnew InvalidOperationException("There is no tracking information for the current branch.");

        if (! current->RemoteName
            || !repo.Remotes->TryGet(current->RemoteName, rm))
        {
            throw gcnew InvalidOperationException("No upstream remote for the current branch.");
        }
    }
    else
    {
        // ### From which remote should we fatch
        for each(GitRemote ^r in repo.Remotes)
          {
            rm = r;
            break;
          }

        if (!rm)
            throw gcnew InvalidOperationException("No origin found.");

        initialCheckout = true;
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

    if (initialCheckout)
    {
        git_checkout_options op = GIT_CHECKOUT_OPTIONS_INIT;

        GIT_THROW(git_checkout_head(repo.Handle, &op));
        return true;
    }

    GitMergeResult ^mr;
    if (!repo.Merge(repo.LastFetchResult, args->MergeArgs, mr))
        return false;
}
