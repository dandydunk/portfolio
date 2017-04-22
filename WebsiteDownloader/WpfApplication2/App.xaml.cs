using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.IO;

namespace WpfApplication2 {
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application {
        static public string AppDir = "";
        static public Object objLock = new Object();

        private void Application_Startup(object sender, StartupEventArgs e) {
            AppDir = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
            if (!AppDir.EndsWith("\\")) {
                AppDir += "\\";
            }

            AppDir += System.IO.Path.GetFileNameWithoutExtension(System.AppDomain.CurrentDomain.FriendlyName) + "\\";
            if (!Directory.Exists(AppDir)) {
                Directory.CreateDirectory(AppDir);
            }

            //Console.WriteLine(AppDir);
            Log("application started");

            AppDomain currentDomain = AppDomain.CurrentDomain;
            currentDomain.UnhandledException += new UnhandledExceptionEventHandler(MyHandler);
        }

        static void MyHandler(object sender, UnhandledExceptionEventArgs args) {
            Exception e = (Exception) args.ExceptionObject;
            //Console.WriteLine("MyHandler caught : " + e.Message);
            //Console.WriteLine("Runtime terminating: {0}", args.IsTerminating);
            Log(String.Format("Unhandled exception [{0}]\r\n{1}", e.Message, e.StackTrace));
            MessageBox.Show(String.Format("Unhandled exception [{0}]", e.Message));
            Application.Current.Shutdown();
        }

        static public void Log(string t) {
            lock(objLock) {
                File.AppendAllText(AppDir + "log.txt", String.Format("[{0}] {1}\r\n", DateTime.Now, t));
            }
        }
    }
}
