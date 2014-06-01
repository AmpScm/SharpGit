#pragma once
#include "GitClientContext.h"
#include "GitCheckOutArgs.h"

namespace SharpGit {

  public ref class GitMergeArgs : public GitCheckOutArgs
  {
      int _renameTreshold;
      int _renameTargetLimit;
      bool _skipFindingRenames;

  public:
      const int DefaultRenameTreshold = 50;
      const int DefaultRenameTargetLimit = 200;

      GitMergeArgs()
      {
          _renameTreshold = DefaultRenameTreshold;
          _renameTargetLimit = DefaultRenameTargetLimit;
      }

  internal:
      git_merge_options *AllocMergeOptions(GitPool ^pool);


  public:
      property int RenameTreshold
      {
          int get()
          {
              return _renameTreshold;
          }
          void set(int value)
          {
              _renameTreshold = (value >= 0) ? value : DefaultRenameTreshold;
          }
      }

      property int RenameTargetLimit
      {
          int get()
          {
              return _renameTargetLimit;
          }
          void set(int value)
          {
              _renameTargetLimit = (value >= 0) ? value : DefaultRenameTargetLimit;
          }
      }

      property bool SkipFindingRenames
      {
          bool get()
          {
              return _skipFindingRenames;
          }
          void set(bool value)
          {
              _skipFindingRenames = value;
          }
      }
  };

  public ref class GitMergeResult : public GitEventArgs
  {
  };

  public ref class GitMergeAnalysis : public GitEventArgs
  {
      initonly git_merge_analysis_t _analysis;
      initonly git_merge_preference_t _preference;

  internal:
      GitMergeAnalysis(git_merge_analysis_t analysis, git_merge_preference_t preference)
      {
        _analysis = analysis;
        _preference = preference;
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
