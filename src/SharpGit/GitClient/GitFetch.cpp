#include "stdafx.h"
#include "GitClient.h"
#include "GitDelete.h"

#include "Args/GitFetchArgs.h"

#include "Plumbing/GitRepository.h"
#include "Plumbing/GitRemote.h"

using namespace System;
using namespace SharpGit;
using namespace SharpGit::Plumbing;

bool GitClient::Fetch(String ^localRepository)
{
    return GitClient::Fetch(localRepository, gcnew GitFetchArgs());
}

bool GitClient::Fetch(String ^localRepository, GitFetchArgs ^ args)
{
    GitRepository repo;

    if (!repo.Locate(localRepository, args))
        return false;

    if (!args->All)
        throw gcnew NotImplementedException();

    for each (GitRemote ^rm in repo.Remotes)
    {
        try
        {
            rm->SetCallbacks(get_callbacks());
            rm->Connect(true, args);
            rm->Download(args);
            rm->UpdateTips(args);
            rm->Disconnect(args);
        }
        finally
        {
            delete rm;
        }
    }
    return true;
}
