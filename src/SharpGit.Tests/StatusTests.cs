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

                GitStatusArgs sa = new GitStatusArgs();
                sa.GenerateVersionedDirs = true;
                sa.IncludeUnmodified = true;
                sa.IncludeIgnored = true; // Directories with 0 files are ignored

                BuildGreek(dir);

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
                            if (e.NodeKind == GitNodeKind.File)
                            {
                                Assert.AreEqual(GitStatus.None, e.IndexStatus, "Invalid index status on {0}", e.RelativePath);
                                Assert.AreEqual(GitStatus.New, e.WorkingDirectoryStatus, "Invalid working status on {0}", e.RelativePath);
                                Assert.IsFalse(e.Ignored);
                            }
                            else if (e.FullPath == dir)
                            {
                                Assert.AreEqual(GitStatus.None, e.WorkingDirectoryStatus);
                                Assert.IsFalse(e.Ignored);
                            }
                            else if (Path.GetFileName(e.FullPath) == "F" || Path.GetFileName(e.FullPath) == "C")
                            {   // Empty directory
                                Assert.AreEqual(GitStatus.Normal, e.WorkingDirectoryStatus, "Invalid status on {0}", e.RelativePath);
                                Assert.AreEqual(GitStatus.None, e.IndexStatus, "Invalid status on {0}", e.RelativePath);
                                Assert.IsTrue(e.Ignored);
                            }
                            else
                            {
                                Assert.AreEqual(GitStatus.None, e.IndexStatus, "Invalid status on {0}", e.RelativePath);
                                Assert.AreEqual(GitStatus.None, e.WorkingDirectoryStatus, "Invalid status on {0}", e.RelativePath);
                                Assert.IsFalse(e.Ignored);
                            }
                            n++;
                        });
                    Assert.AreEqual(21, n);
                    Assert.AreEqual(0, paths.Count);
                }

                foreach (string[] k in GreekTree())
                {
                    string p = Path.Combine(dir, k[0]);
                    if (File.Exists(p))
                        client.Stage(p);
                }

                string A_dir = Path.Combine(dir, "A\\B");

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
                            if (e.NodeKind == GitNodeKind.File)
                            {
                                Assert.AreEqual(GitStatus.Added, e.IndexStatus, "Invalid index status on {0}", e.RelativePath);
                                Assert.AreEqual(GitStatus.Normal, e.WorkingDirectoryStatus, "Invalid working status on {0}", e.RelativePath);
                                Assert.IsFalse(e.Ignored);
                            }
                            else if (e.FullPath == dir)
                            {
                                Assert.AreEqual(GitStatus.None, e.WorkingDirectoryStatus);
                                Assert.IsFalse(e.Ignored);
                            }
                            else if (Path.GetFileName(e.FullPath) == "F" || Path.GetFileName(e.FullPath) == "C")
                            {   // Empty directory
                                Assert.AreEqual(GitStatus.Normal, e.WorkingDirectoryStatus, "Invalid status on {0}", e.RelativePath);
                                Assert.AreEqual(GitStatus.None, e.IndexStatus, "Invalid status on {0}", e.RelativePath);
                                Assert.IsTrue(e.Ignored);
                            }
                            else
                            {
                                Assert.AreEqual(GitStatus.None, e.IndexStatus, "Invalid status on {0}", e.RelativePath);
                                Assert.AreEqual(GitStatus.None, e.WorkingDirectoryStatus, "Invalid status on {0}", e.RelativePath);
                                Assert.IsFalse(e.Ignored);
                            }
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
                            Assert.AreEqual(e.NodeKind, File.Exists(e.FullPath) ? GitNodeKind.File : GitNodeKind.Directory);
                            Assert.IsFalse(e.IndexModified);
                            Assert.IsFalse(e.WorkingDirectoryModified);
                            n++;
                        });
                    Assert.AreEqual(21, n);
                    Assert.AreEqual(0, paths.Count);
                }
            }
        }
    }
}
