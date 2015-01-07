#include "stdafx.h"
#include "GitClient.h"
#include "GitCheckOutArgs.h"

#include "Plumbing/GitRepository.h"
#include "Plumbing/GitIndex.h"

using namespace SharpGit;
using namespace SharpGit::Plumbing;

struct checkout_cb_info_t
{
  gcroot<GitCheckOutArgs^> Args;
  apr_pool_t *pool;

public:
  checkout_cb_info_t(GitCheckOutArgs ^args, apr_pool_t *pl)
    : Args(args), pool(pl)
    {}
};

static apr_status_t __cdecl cleanup_notify(void *baton)
{
  checkout_cb_info_t *notify = reinterpret_cast<checkout_cb_info_t *>(baton);

  delete notify;
  return 0;
}

static int __cdecl Git_checkout_notify(git_checkout_notify_t why, const char *path, const git_diff_file *baseline, const git_diff_file *target, const git_diff_file *workdir, void *payload)
{
  checkout_cb_info_t *notify = reinterpret_cast<checkout_cb_info_t *>(payload);
  GitPool pool(notify->pool);
  GitCheckOutNotifyEventArgs ^e = nullptr;

  try
  {
      e = gcnew GitCheckOutNotifyEventArgs(why, path, baseline, target, workdir, %pool);

      notify->Args->InvokeCheckOutNotify(e);
      return 0;
  }
  catch(Exception ^e)
  {
      return GitBase::WrapError(e);
  }
  finally
  {
      if (e)
          e->Detach(false);
  }
}

static void __cdecl Git_checkout_progress(const char *path, size_t completed_steps, size_t total_steps, void *payload)
{
  checkout_cb_info_t *notify = reinterpret_cast<checkout_cb_info_t *>(payload);

  GitCheckOutProgressEventArgs ^ee = gcnew GitCheckOutProgressEventArgs(path, completed_steps, total_steps);
  try
  {
      notify->Args->InvokeCheckOutProgress(ee);
  }
  catch(Exception ^)
  {
      //return GitBase::WrapError(e);
  }
  finally
  {
      ee->Detach(false);
  }
}

git_checkout_options * GitCheckOutArgs::AllocCheckOutOptions(GitPool ^pool)
{
    git_checkout_options *opts = (git_checkout_options *)pool->Alloc(sizeof(*opts));

    git_checkout_init_options(opts, GIT_CHECKOUT_OPTIONS_VERSION);

    if (! DryRun)
    {
        opts->checkout_strategy |= GIT_CHECKOUT_SAFE;

        if (! SkipMissingFiles)
            opts->checkout_strategy |= GIT_CHECKOUT_SAFE_CREATE;

        if (OverwriteExisting)
            opts->checkout_strategy |= GIT_CHECKOUT_FORCE;

        if (! FailOnConflicts)
            opts->checkout_strategy |= GIT_CHECKOUT_ALLOW_CONFLICTS;

        if (RemoveUnversioned)
            opts->checkout_strategy |= GIT_CHECKOUT_REMOVE_UNTRACKED;

        if (RemoveIgnored)
            opts->checkout_strategy |= GIT_CHECKOUT_REMOVE_IGNORED;

        if (UpdateOnly)
            opts->checkout_strategy |= GIT_CHECKOUT_UPDATE_ONLY;

        // GIT_CHECKOUT_DISABLE_PATHSPEC_MATCH

        // Defined but unimplemented:
        // GIT_CHECKOUT_SKIP_UNMERGED
        // GIT_CHECKOUT_UPDATE_SUBMODULES
        // GIT_CHECKOUT_UPDATE_SUBMODULES_IF_CHANGED

        switch (ConflictStrategy)
        {
            case GitCheckOutConflictStrategy::UseOurs:
                opts->checkout_strategy |= GIT_CHECKOUT_USE_OURS;
                break;
            case GitCheckOutConflictStrategy::UseTheirs:
                opts->checkout_strategy |= GIT_CHECKOUT_USE_THEIRS;
                break;

            case GitCheckOutConflictStrategy::Diff3:
                opts->checkout_strategy |= GIT_CHECKOUT_CONFLICT_STYLE_DIFF3;
                break;

            case GitCheckOutConflictStrategy::Merge:
                opts->checkout_strategy |= GIT_CHECKOUT_CONFLICT_STYLE_MERGE;
                break;

            default:
                break; // Will be retrieved from config.
        }
    }

    if (this->NoCacheUpdate)
        opts->checkout_strategy |= GIT_CHECKOUT_DONT_UPDATE_INDEX;

    if (this->NoRefresh)
        opts->checkout_strategy |= GIT_CHECKOUT_NO_REFRESH;

    checkout_cb_info_t *payload = new checkout_cb_info_t(this, pool->Handle);
    apr_pool_cleanup_register(pool->Handle, payload, cleanup_notify, apr_pool_cleanup_null);

    opts->notify_payload = payload;
    opts->progress_payload = payload;

    opts->notify_flags = GIT_CHECKOUT_NOTIFY_ALL;
    opts->notify_cb = Git_checkout_notify;

    opts->progress_cb = Git_checkout_progress;

    return opts;
}
