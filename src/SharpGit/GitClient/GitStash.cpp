#include "pch.h"
#include "GitClient.h"
#include "GitStashArgs.h"

#include "Plumbing/GitRepository.h"
#include "Plumbing/GitIndex.h"

using namespace SharpGit;
using namespace SharpGit::Plumbing;

bool GitClient::Stash(String ^localRepository)
{
    GitId ^stashId;

    return GitClient::Stash(localRepository, gcnew GitStashArgs(), stashId);
}

bool GitClient::Stash(String ^localRepository, GitStashArgs ^args)
{
    GitId ^stashId;

    return GitClient::Stash(localRepository, args, stashId);
}

bool GitClient::Stash(String ^localRepository, [Out] GitId ^%stashId)
{
    return GitClient::Stash(localRepository, gcnew GitStashArgs(), stashId);
}


bool GitClient::Stash(String ^localRepository, GitStashArgs ^args, [Out] GitId ^%stashId)
{
    GitRepository repo;

    if (!repo.Locate(localRepository, args))
        return false;

    GitPool pool;

    return repo.Stash(args, stashId);
}