#pragma once

namespace SharpGit {

      public ref class GitInitArgs : GitClientArgs
        {
        internal:
            git_repository_init_options * MakeInitOptions(GitPool ^pool);

        private:
            bool _bare;
            bool _noCreateDir;
            String ^_description;
            System::Uri ^_originUrl;
        public:
            property bool CreateBareRepository
            {
                bool get()
                {
                    return _bare;
                }
                void set(bool value)
                {
                    _bare = value;
                }
            }

            property bool CreateDirectory
            {
                bool get()
                {
                    return !_noCreateDir;
                }
                void set(bool value)
                {
                    _noCreateDir = !value;
                }
            }

            property String ^ Description
            {
                String ^ get()
                {
                    return _description;
                }

                void set(String ^ value)
                {
                    _description = value;
                }
            }

            property System::Uri ^ Origin
            {
                System::Uri ^ get()
                {
                    return _originUrl;
                }

                void set(System::Uri ^ value)
                {
                    if (value && ! value->IsAbsoluteUri)
                        throw gcnew InvalidOperationException();

                    _originUrl = value;
                }
            }
        };
}
