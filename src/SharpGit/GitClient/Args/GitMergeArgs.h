#pragma once
#include "GitClientContext.h"
#include "../GitCheckOut.h"

namespace SharpGit {

  public ref class GitMergeArgs : public GitCheckOutArgs
  {
  internal:
      git_merge_options *AllocMergeOptions(GitPool ^pool);
  };

  public ref class GitMergeResult : public GitEventArgs
  {
  };
}
