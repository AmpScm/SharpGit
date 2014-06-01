#include "stdafx.h"
#include "GitClient.h"

#include "GitPullArgs.h"

#include "Plumbing/GitRepository.h"
#include "Plumbing/GitCommit.h"
#include "Plumbing/GitConfiguration.h"
#include "Plumbing/GitIndex.h"
#include "Plumbing/GitRemote.h"
#include "Plumbing/GitBranch.h"
#include "Plumbing/GitReference.h"

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
    String ^defaultBranch;

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
        // ### From which remote should we fetch ### config?

        if (!repo.Remotes->TryGet("origin", rm))
        {
            throw gcnew InvalidOperationException("No origin found.");
        }

        initialCheckout = true;
    }

    IList<GitRemoteHead^> ^heads = nullptr;

    try
    {
        rm->SetCallbacks(get_callbacks());
        rm->Connect(true, args->FetchArgs);
        rm->Download(args->FetchArgs);
        rm->UpdateTips(args->FetchArgs);

        if (initialCheckout)
        {
            heads = rm->GetHeads();
            defaultBranch = rm->DefaultBranch;
        }

        rm->Disconnect(args->FetchArgs);
    }
    finally
    {
        delete rm;
    }

    /*if (initialCheckout)
    {
        
    }*/

    GitMergeAnalysis ^an;
    if (repo.MergeAnalysis(repo.LastFetchResult, args->MergeArgs, an))
    {
        if (an->IsUpToDate)
            return true;
        else if (an->IsUnborn)
        {
            if (!initialCheckout)
                throw gcnew NotImplementedException();

            GitCommit ^commit;
            GitBranch ^branch;

            // The first reported head is HEAD
            if (!repo.Lookup(heads[0]->Id, commit))
                throw gcnew InvalidOperationException("Not synced?");

            if (!repo.Branches->Create(commit, defaultBranch ? defaultBranch : "refs/heads/master", args->FetchArgs, branch))
                throw gcnew InvalidOperationException("Failed to create local branch");

            branch->RecordAsHeadBranch(args->FetchArgs);

            GitPool pool(repo.Pool);
            GIT_THROW(git_checkout_head(repo.Handle, args->MergeArgs->MakeCheckOutOptions(%pool)));
            //repo.CheckOut(commit->Tree, args->MergeArgs);
            return true;
        }
        else
        {
            GitMergeResult ^mr;

            if (repo.Merge(repo.LastFetchResult, args->MergeArgs, mr))
            {
                if (repo.Index->HasConflicts)
                {
                }
                else
                {
                    if (args->CommitOnSuccess)
                    {
                        GitId ^result;
                        Commit(repo.WorkingCopyDirectory, args->CommitArgs, result);
                    }
                }

                return true;
            }
        }
    }

    return false;
}
