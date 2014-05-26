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

            string start;
            if (TestContext != null)
                start = Path.Combine(TestContext.TestDir, "gittest");
            else
                start = Path.Combine(Path.GetTempPath(), "gittest");

            Directory.CreateDirectory(start); // No error when exists

            for (int n = 4; n < rest.Length; n++)
            {
                string name = Path.Combine(start, rest.Substring(0, n));

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
    }
}
