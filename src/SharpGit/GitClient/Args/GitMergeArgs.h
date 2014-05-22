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

  public ref class GitMergeAnalysis : public GitEventArgs
  {
      initonly git_merge_analysis_t _analysis;

  internal:
      GitMergeAnalysis(const git_merge_analysis_t analysis)
      {
        _analysis = analysis;
      }

  public:
      /// <summary>
      /// A "normal" merge; both HEAD and the given merge input have diverged
      /// from their common ancestor.  The divergent commits must be merged.
      /// </summary>
      property bool CanMergeNormal
      {
          bool get()
          {
              return (0 != (_analysis & GIT_MERGE_ANALYSIS_NORMAL));
          }
      }

      /// <summary>
      /// All given merge inputs are reachable from HEAD, meaning the
	  /// repository is up-to-date and no merge needs to be performed.
      /// </summary>
      property bool IsUpToDate
      {
          bool get()
          {
              return (0 != (_analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE));
          }
      }

      /// <summary>
      /// The given merge input is a fast-forward from HEAD and no merge
      /// needs to be performed.  Instead, the client can check out the
      /// given merge input.
      /// </summary>
      property bool CanFastForward
      {
          bool get()
          {
              return (0 != (_analysis & GIT_MERGE_ANALYSIS_FASTFORWARD));
          }
      }

      /// <summary>
      /// The HEAD of the current repository is "unborn" and does not point to
      /// a valid commit.  No merge can be performed, but the caller may wish
      /// to simply set HEAD to the target commit(s).
      /// </summary>
      property bool IsUnborn
      {
          bool get()
          {
              return (0 != (_analysis & GIT_MERGE_ANALYSIS_UNBORN));
          }
      }
  };
}
