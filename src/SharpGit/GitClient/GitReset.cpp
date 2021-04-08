#include "pch.h"
#include "GitClient.h"

#include "Plumbing/GitRepository.h"
#include "GitResetArgs.h"

using namespace SharpGit;
using namespace SharpGit::Plumbing;


bool GitClient::Reset(String ^localRepository)
{
    return Reset(localRepository, gcnew GitResetArgs());
}

bool GitClient::Reset(String ^localRepository, GitResetArgs ^args)
{
    if (String::IsNullOrEmpty(localRepository))
        throw gcnew ArgumentNullException("localRepository");
    else if (! args)
        throw gcnew ArgumentNullException("args");

    GitRepository repo;

    if (!repo.Locate(localRepository, args))
        return false;

    return repo.Reset(args);
}
