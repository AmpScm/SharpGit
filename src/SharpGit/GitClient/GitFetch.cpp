#include "pch.h"
#include "GitClient.h"
#include "GitDeleteArgs.h"
#include "GitFetchArgs.h"

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
        GitPool pool(repo.Pool);

        try
        {
            rm->Fetch(nullptr, args, this);
        }
        finally
        {
            delete rm;
        }
    }
    return true;
}
