#include "stdafx.h"
#include "GitClient.h"


#include "Plumbing/GitRepository.h"
#include "Plumbing/GitIndex.h"
#include "GitInitArgs.h"

using namespace SharpGit;
using namespace SharpGit::Plumbing;


git_repository_init_options * GitInitArgs::MakeInitOptions(GitPool ^pool)
{
    git_repository_init_options *opts = (git_repository_init_options *)pool->Alloc(sizeof(*opts));

    git_repository_init_init_options(opts, GIT_REPOSITORY_INIT_OPTIONS_VERSION);

    if (CreateBareRepository)
        opts->flags |= GIT_REPOSITORY_INIT_BARE;
    if (CreateDirectory)
        opts->flags |= GIT_REPOSITORY_INIT_MKDIR;

    if (Description)
        opts->description = pool->AllocString(Description);

    if (Origin)
        opts->origin_url = pool->AllocString(Origin->AbsoluteUri);

    return opts;
}

bool GitClient::Init(String ^localRepository)
{
    return Init(localRepository, gcnew GitInitArgs());
}

bool GitClient::Init(String ^localRepository, GitInitArgs ^args)
{
    GitRepository rp;
    
    return rp.CreateAndOpen(localRepository, args);
}
