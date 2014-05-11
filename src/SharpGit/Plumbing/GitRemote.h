#pragma once
#include "GitClientContext.h"
#include "GitRepository.h"

namespace SharpGit {
    namespace Plumbing {

        public ref class GitRemote : public Implementation::GitBase
        {
            initonly GitRepository ^_repository;
            git_remote *_remote;
            String^ _name;

        private:
            GitRemote(GitRepository ^repository, git_remote *remote);

            ~GitRemote();
            !GitRemote();

        public:
            property String^ Name
            {
                String ^ get();
            }
        };

        public ref class GitRemoteCollection sealed : public Implementation::GitBase,
                                                      public System::Collections::Generic::IEnumerable<GitRemote^>
        {
            initonly GitRepository ^_repository;

        internal:
            GitRemoteCollection(GitRepository^ repository)
            {
                if (!repository)
                    throw gcnew ArgumentNullException("repository");

                _repository = repository;
            }

        public:
            virtual System::Collections::Generic::IEnumerator<GitRemote^>^ GetEnumerator();

        private:
            virtual System::Collections::IEnumerator^ GetObjectEnumerator() sealed
                = System::Collections::IEnumerable::GetEnumerator
            {
                return GetEnumerator();
            }
        };
    }
}