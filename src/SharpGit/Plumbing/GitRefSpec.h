#pragma once
namespace SharpGit {
    namespace Plumbing {
        using System::String;

        public ref class GitRefSpec sealed
              : public System::IComparable<GitRefSpec^>,
                public System::IEquatable<GitRefSpec^>
        {
            bool _updateAlways;
            String ^_src;
            String ^_dst;

        public:
            GitRefSpec(String ^source, String ^destination, bool updateAlways);
            GitRefSpec(String ^source, String ^destination);

        internal:
            GitRefSpec(const git_refspec &refspec);

        public:
            property String ^ Source
            {
                String ^ get()
                {
                    return _src;
                }
            }

            property String ^ Destination
            {
                String ^ get()
                {
                    return _dst;
                }
            }

            property bool UpdateAlways
            {
                bool get()
                {
                    return _updateAlways;
                }
            }

        public:
            static GitRefSpec ^ Parse(String ^refspec);
            static bool TryParse(String ^refspec, [Out]GitRefSpec ^%value);

        public:
            virtual String ^ ToString() override
            {
                return UpdateAlways ? String::Concat("+", Source, ":", Destination)
                                    : String::Concat(Source, ":", Destination);
            }

        public:
            virtual bool Equals(Object ^other) override
            {
                return Equals(dynamic_cast<GitRefSpec^>(other));
            }

            virtual bool Equals(GitRefSpec ^other)
            {
                if (!(Object^)other)
                    return false;

                return ToString()->Equals(other->ToString());
            }

            virtual int CompareTo(GitRefSpec ^other)
            {
                int n = Source->CompareTo(other->Source);

                if (!n)
                    n = Destination->CompareTo(other->Destination);

                if (n)
                    return n;

                if (UpdateAlways == other->UpdateAlways)
                    return 0;
                else if (UpdateAlways)
                    return -1;
                else
                    return 1;
            }
        };
    }
}