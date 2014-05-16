#pragma once
#include "GitClientContext.h"

namespace SharpGit {
    namespace Plumbing {


        public ref class GitReference : public Implementation::GitBase
        {
        private:
            initonly GitRepository ^_repository;
            git_reference *_reference;
            String^ _name;

            !GitReference()
            {
                if (_reference)
                {
                    try
                    {
                        git_reference_free(_reference);
                    }
                    finally
                    {
                        _reference = nullptr;
                        _name = nullptr;
                    }
                }
            }

            ~GitReference()
            {
                try
                    {
                        git_reference_free(_reference);
                    }
                    finally
                    {
                        _reference = nullptr;
                        _name = nullptr;
                    }
            }

        internal:
            GitReference(GitRepository ^repository, git_reference *handle)
            {
                if (! repository)
                    throw gcnew ArgumentNullException("repository");
                if (! handle)
                    throw gcnew ArgumentNullException("handle");

                _repository = repository;
                _reference = handle;
            }

            GitReference(GitRepository ^repository, String^ name)
            {
                if (! repository)
                    throw gcnew ArgumentNullException("repository");
                else if (String::IsNullOrEmpty(name))
                    throw gcnew ArgumentNullException("name");

                _repository = repository;
                _name = name;
            }

            property const git_reference* Handle
            {
                const git_reference* get();
            }

        private:


        public:
            property bool IsDisposed
            {
                bool get();
            }

            property String^ Name
            {
                String^ get();
            }

            property bool HasLog
            {
                bool get();
            }

            property bool IsBranch
            {
                bool get()
                {
                    return git_reference_is_branch(Handle) != 0;
                }
            }

            property bool IsRemoteTrackingBranch
            {
                bool get()
                {
                    return git_reference_is_remote(Handle) != 0;
                }
            }

            property bool IsNote
            {
                bool get()
                {
                    return git_reference_is_note(Handle) != 0;
                }
            }

            property bool IsTag
            {
                bool get()
                {
                    return git_reference_is_tag(Handle) != 0;
                }
            }

            property String ^ ShortName
            {
                String ^ get()
                {
                    return GitBase::Utf8_PtrToString(git_reference_shorthand(Handle));
                }
            }

            bool EnsureLog();

            bool Delete();
            bool Delete(GitArgs ^args);
        };

        public ref class GitReferenceCollection sealed : public Implementation::GitBase,
                                                                                   public System::Collections::Generic::IEnumerable<GitReference^>
        {
            initonly GitRepository ^_repository;
        internal:
            GitReferenceCollection(GitRepository ^repository)
            {
                if (!repository)
                    throw gcnew ArgumentNullException("repository");

                _repository = repository;
            }

        public:
            virtual System::Collections::Generic::IEnumerator<GitReference^>^ GetEnumerator();

        private:
            IEnumerable<GitReference^>^ GetEnumerable();

        private:
            virtual System::Collections::IEnumerator^ GetObjectEnumerator() sealed = System::Collections::IEnumerable::GetEnumerator
            {
                return GetEnumerator();
            }
        };
    }
}
