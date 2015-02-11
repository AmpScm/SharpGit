using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NUnit.Framework;
using SharpGit.Diff;
using Assert = NUnit.Framework.Assert;

namespace SharpGit.Tests
{
    [TestClass]
    public class AuthTests : TestBase
    {
        [TestMethod]
        public void TestAuthHttps2()
        {
            using (GitClient git = new GitClient())
            {
                string local = GetTempPath();
                int n = 0;

                git.Certificate += delegate(object sender, GitCertificateEventArgs e)
                {
                    GC.KeepAlive(e);
                };

                git.Authentication.Credentials += delegate(object sender, GitCredentialEventArgs e)
                {
                    switch (n)
                    {
                        case 0:
                            e.SetUsernamePassword("A", "B-invalid");
                            break;
                        default:
                            break;
                    }
                    GC.KeepAlive(e);
                    n++;
                };

                git.Clone(new Uri("https://secure.vsoft.nl/qq"), local);
                Assert.That(n, Is.EqualTo(0));
            }
        }


        [TestMethod]
        public void TestAuthHttps()
        {
            using (GitClient git = new GitClient())
            {
                string local = GetTempPath();
                int n = 0;

                git.Certificate += delegate(object sender, GitCertificateEventArgs e)
                {
                    throw new NotImplementedException();
                };

                git.Authentication.Credentials += delegate(object sender, GitCredentialEventArgs e)
                {
                    switch (n)
                    {
                        case 0:
                            e.SetUsernamePassword("A", "B-invalid");
                            break;
                        default:
                            break;
                    }
                    GC.KeepAlive(e);
                    n++;
                };

                git.Clone(new Uri("https://github.com/rhuijben/fruitmachine.git"), local);
                Assert.That(n, Is.EqualTo(0));
                //git.Authentication.Handlers +=

                //try
                {
                    GitPushArgs pa = new GitPushArgs();
                    pa.Mode = GitPushMode.All;

                    git.Push(local, pa);

                    Assert.Fail();
                }
                /*catch(Exception e)
                {
                    GC.KeepAlive(e);
                }*/

                Assert.That(n, Is.GreaterThan(0));
            }
        }

        [TestMethod]
        public void TestAuthSsh()
        {
            using (GitClient git = new GitClient())
            {
                string local = GetTempPath();
                int n = 0;

                git.Authentication.Credentials += delegate(object sender, GitCredentialEventArgs e)
                {
                    switch (n)
                    {
                        case 0:
                            if (e.AllowUsernamePassword)
                                e.SetUsernamePassword("bert", "Wmj3=woP");
                            else if (e.AllowUsername)
                                e.SetUsername("bert");
                            break;
                        default:
                            break;
                    }
                    GC.KeepAlive(e);
                    n++;
                };

                git.Clone(new Uri("ssh://vip/~bert/gittest"), local);
                Assert.That(n, Is.EqualTo(0));
                //git.Authentication.Handlers +=

                //try
                {
                    GitPushArgs pa = new GitPushArgs();
                    pa.Mode = GitPushMode.All;

                    git.Push(local, pa);

                    Assert.Fail();
                }
                /*catch(Exception e)
                {
                    GC.KeepAlive(e);
                }*/

                Assert.That(n, Is.GreaterThan(0));
            }
        }
    }
}
