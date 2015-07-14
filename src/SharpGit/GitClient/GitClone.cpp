#include "stdafx.h"
#include "GitClient.h"
#include "GitCloneArgs.h"

#include "GitClient/GitInitArgs.h"
#include "Plumbing/GitRepository.h"
#include "Plumbing/GitIndex.h"

using namespace SharpGit;
using namespace SharpGit::Implementation;
using namespace SharpGit::Plumbing;

GitCloneArgs::GitCloneArgs()
{
    _initArgs = gcnew GitInitArgs();
}

const git_clone_options * GitCloneArgs::MakeCloneOptions(GitPool ^pool)
{
    if (! pool)
        throw gcnew ArgumentNullException("pool");

    git_clone_options *opts = (git_clone_options *)pool->Alloc(sizeof(*opts));

    git_clone_init_options(opts, GIT_CLONE_OPTIONS_VERSION);

    const git_checkout_options *coo = AllocCheckOutOptions(pool);
    opts->checkout_opts = *coo;

    opts->checkout_branch = BranchName ? pool->AllocString(BranchName) : nullptr;

    // Remote and Callbacks are handled in caller

    return opts;
}

bool GitClient::Clone(Uri ^remoteRepository, String ^path)
{
    return Clone(remoteRepository, path, gcnew GitCloneArgs());
}

bool GitClient::Clone(String ^localRepository, String ^path)
{
    return Clone(localRepository, path, gcnew GitCloneArgs());
}

bool GitClient::Clone(Uri ^remoteRepository, String ^path, GitCloneArgs ^args)
{
    if (! remoteRepository)
        throw gcnew ArgumentNullException("remoteRepository");
    else if (String::IsNullOrEmpty(path))
        throw gcnew ArgumentNullException("path");
    else if (! args)
        throw gcnew ArgumentNullException("args");

    GitPool pool(_pool);

    return CloneInternal(svn_uri_canonicalize(pool.AllocString(remoteRepository->AbsoluteUri), pool.Handle), path, args, %pool);
}

bool GitClient::Clone(String ^localRepository, String ^path, GitCloneArgs ^args)
{
    if (String::IsNullOrEmpty(localRepository))
        throw gcnew ArgumentNullException("remoteRepository");
    else if (String::IsNullOrEmpty(path))
        throw gcnew ArgumentNullException("path");
    else if (! args)
        throw gcnew ArgumentNullException("args");

    if (localRepository->LastIndexOf(':') > 1)
        throw gcnew ArgumentOutOfRangeException("Use Clone(Uri,...) to pass urls", "localRepository");

    GitPool pool(_pool);
    return CloneInternal(pool.AllocDirent(localRepository), path, args, %pool);
}

struct create_repository
{
  gcroot<GitPool^> _pool;
  gcroot<GitClient^> _client;
  gcroot<GitCloneArgs^> _args;

  create_repository(GitClient^ client, GitCloneArgs^ args, GitPool^ pool)
    : _client(client), _args(args), _pool(pool)
  {}

  static int __cdecl repository_create_cb(git_repository **out, const char *path, int bare, void *payload)
  {
      create_repository *cr = (create_repository *)payload;

      try
      {
          cr->_client->Init(GitBase::StringFromDirent(path, cr->_pool), cr->_args->InitArgs);


          return git_repository_open(out, path);
      }
      catch(GitException ^e)
      {
          return GitBase::WrapError(e);
      }
  }
};

struct create_remote
{
  gcroot<GitPool^> _pool;
  gcroot<GitClient^> _client;
  gcroot<GitCloneArgs^> _args;

  create_remote(GitClient^ client, GitCloneArgs^ args, GitPool^ pool)
    : _client(client), _args(args), _pool(pool)
  {}

  static int __cdecl create_remote_cb(git_remote **out, git_repository *repo, const char *name, const char *url, void *payload)
  {
      create_remote *cr = (create_remote *)payload;

      try
      {
          //cr->_client->Init(GitBase::StringFromDirent(path, cr->_pool), cr->_args->InitArgs);

          return git_remote_create(out, repo, name, url);
      }
      catch(GitException ^e)
      {
          return GitBase::WrapError(e);
      }
  }
};

bool GitClient::CloneInternal(const char *rawRepository, String ^path, GitCloneArgs ^args, GitPool ^pool)
{
    git_repository *repository;

    int r;

    if (args->Synchronous)
    {
        create_repository c_repository(this, args, pool);
        create_remote c_remote(this, args, pool);
        git_clone_options *opts = const_cast<git_clone_options *>(args->MakeCloneOptions(pool));

        opts->repository_cb = create_repository::repository_create_cb;
        opts->repository_cb_payload = &c_repository;

        opts->remote_cb = create_remote::create_remote_cb;
        opts->remote_cb_payload = &c_remote;
        

        r = git_clone(&repository, rawRepository, pool->AllocDirent(path), opts);
    }
    else
        throw gcnew NotImplementedException();


    return args->HandleGitError(this, r);
}
