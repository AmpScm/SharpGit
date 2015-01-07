using System;
using System.Collections.Generic;
using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace SharpGit.Tests
{
    [TestClass]
    public class StatusTests : TestBase
    {
        [TestMethod]
        public void FullStatus()
        {
            using(GitClient client = new GitClient())
            {
                string dir = GitTools.GetTruePath(GetTempPath(), true);
                client.Init(dir);

                BuildGreek(dir);

                foreach (string[] k in GreekTree())
                {
                    string p = Path.Combine(dir, k[0]);
                    if (File.Exists(p))
                        client.Stage(p);
                }

                string A_dir = Path.Combine(dir, "A\\B");
                GitStatusArgs sa = new GitStatusArgs();
                sa.GenerateVersionedDirs = true;
                sa.IncludeUnmodified = true;
                sa.IncludeIgnored = true; // Directories with 0 files are ignored

                {
                    int n = 0;
                    client.Status(A_dir, sa,
                        delegate(object sender, GitStatusEventArgs e)
                        {
                            Assert.IsTrue(Path.IsPathRooted(e.FullPath));
                            Assert.IsTrue(e.FullPath.StartsWith(A_dir + "\\") || e.FullPath == A_dir);
                            n++;
                        });
                    Assert.AreEqual(6, n);
                }

                {
                    List<string> paths = new List<string>();
                    int n = 0;
                    foreach (string[] s in GreekTree())
                        paths.Add(GitTools.GetNormalizedFullPath(Path.Combine(dir, s[0])));

                    paths.Add(dir);

                    n = 0;
                    client.Status(dir, sa,
                        delegate(object sender, GitStatusEventArgs e)
                        {
                            Assert.IsTrue(Path.IsPathRooted(e.FullPath));
                            Assert.IsTrue(paths.Remove(e.FullPath), "Can remove {0}", e.FullPath);
                            n++;
                        });
                    Assert.AreEqual(21, n);
                    Assert.AreEqual(0, paths.Count);
                }

                client.Commit(dir);

                {
                    List<string> paths = new List<string>();
                    int n = 0;
                    foreach (string[] s in GreekTree())
                        paths.Add(GitTools.GetNormalizedFullPath(Path.Combine(dir, s[0])));

                    paths.Add(dir);

                    n = 0;
                    client.Status(dir, sa,
                        delegate(object sender, GitStatusEventArgs e)
                        {
                            Assert.IsTrue(Path.IsPathRooted(e.FullPath));
                            Assert.IsTrue(paths.Remove(e.FullPath), "Can remove {0}", e.FullPath);
                            n++;
                        });
                    Assert.AreEqual(21, n);
                    Assert.AreEqual(0, paths.Count);
                }
            }
        }
    }
}
