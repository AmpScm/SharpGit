using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace SharpGit.Tests
{
    public abstract class TestBase
    {
        protected string GetTempPath()
        {
            string rest = Guid.NewGuid().ToString("N");
            int restBaseLen = 0;

            string start;
            if (TestContext != null)
            {
                start = Path.Combine(TestContext.TestDir, "gittest");
                rest = TestContext.TestName + "-" + rest;
                restBaseLen = rest.Length - 32;
            }
            else
                start = Path.Combine(Path.GetTempPath(), "gittest");

            Directory.CreateDirectory(start); // No error when exists

            for (int n = 4; n < rest.Length; n++)
            {
                string name = Path.Combine(start, rest.Substring(0, n + restBaseLen));

                if (!Directory.Exists(name))
                    return name;
            }

            return Path.Combine(Path.GetTempPath(), "gittest\\" + Guid.NewGuid().ToString("N"));
        }


        Microsoft.VisualStudio.TestTools.UnitTesting.TestContext _tcx;
        public Microsoft.VisualStudio.TestTools.UnitTesting.TestContext TestContext
        {
            get { return _tcx; }
            set
            {
                _tcx = value;
            }
        }

        protected void BuildGreek(string path)
        {
            path = Path.GetFullPath(path);

            foreach (string[] kv in GreekTree())
            {
                string fullPath = Path.Combine(path, kv[0]);

                if (kv[1] == null)
                    Directory.CreateDirectory(fullPath);
                else
                    File.WriteAllText(fullPath, kv[1]);
            }
        }

        protected string[][] GreekTree()
        {
            return new string[][] { 
                            new string[] { "iota",         "This is the file 'iota'.\n" },
                            new string[] { "A",            null },
                            new string[] { "A/mu",         "This is the file 'mu'.\n" },
                            new string[] { "A/B",          null },
                            new string[] { "A/B/lambda",   "This is the file 'lambda'.\n" },
                            new string[] { "A/B/E",        null },
                            new string[] { "A/B/E/alpha",  "This is the file 'alpha'.\n" },
                            new string[] { "A/B/E/beta",   "This is the file 'beta'.\n" },
                            new string[] { "A/B/F",        null },
                            new string[] { "A/C",          null },
                            new string[] { "A/D",          null },
                            new string[] { "A/D/gamma",    "This is the file 'gamma'.\n" },
                            new string[] { "A/D/G",        null },
                            new string[] { "A/D/G/pi",     "This is the file 'pi'.\n" },
                            new string[] { "A/D/G/rho",    "This is the file 'rho'.\n" },
                            new string[] { "A/D/G/tau",    "This is the file 'tau'.\n" },
                            new string[] { "A/D/H",        null },
                            new string[] { "A/D/H/chi",    "This is the file 'chi'.\n" },
                            new string[] { "A/D/H/psi",    "This is the file 'psi'.\n" },
                            new string[] { "A/D/H/omega",  "This is the file 'omega'.\n" }};
        }
    }
}
