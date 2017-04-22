using System;
using System.Collections.Generic;
using System.Collections;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Threading;
using WebsiteDownloaderLib;

namespace WpfApplication2 {
    public partial class MainWindow : Window, System.Windows.Forms.IWin32Window {
        string Url;
        string Timestamp;
        string SavePath;
        Queue Queue = Queue.Synchronized(new Queue());
        List<string> listPaths = new List<string>();
        int CountThreadsWaiting = 0;
        bool StopThreads = false;
        int CountConnections;
        object _locker = new object();

        public IntPtr Handle {
            get {
                var interopHelper = new System.Windows.Interop.WindowInteropHelper(this);
                return interopHelper.Handle;
            }
        }

        public MainWindow() {
            InitializeComponent();
            WebArchiveCom.FailedDownloadHandler = FailedDownloadHandler;
        }

        void FailedDownloadHandler(string path, System.Net.HttpStatusCode statusCode) {
            //Console.WriteLine("failed [{0}] downloading {1}", statusCode, path);
            App.Log(String.Format("failed [{0}] downloading {1}", statusCode, path));
            PagesFailed(1);
        }

        private void Button_Browse_Click(object sender, RoutedEventArgs e) {
            System.Windows.Forms.FolderBrowserDialog f = new System.Windows.Forms.FolderBrowserDialog();
            f.ShowDialog(this);
            TextBox_SavePath.Text = f.SelectedPath;
        }

        private void Button_Download_Click(object sender, RoutedEventArgs e) {
            Button_Download.IsEnabled = false;
            WebPage.StopWorking = false;
            if (TextBox_SavePath.Text.Trim().Length == 0) {
                MessageBox.Show("The save path is empty.");
                TextBox_SavePath.Focus();
                Button_Download.IsEnabled = true;
                return;
            }

            if (TextBox_Timestamp.Text.Trim().Length == 0) {
                MessageBox.Show("The time stamp is empty.");
                TextBox_Timestamp.Focus();
                Button_Download.IsEnabled = true;
                return;
            }

            if (TextBox_Url.Text.Trim().Length == 0) {
                MessageBox.Show("The url is empty.");
                TextBox_Url.Focus();
                Button_Download.IsEnabled = true;
                return;
            }

            try {
                new Uri(TextBox_Url.Text.Trim());
            } catch (Exception) {
                MessageBox.Show("invalid url, did you forget the http://?");
                TextBox_Url.Focus();
                Button_Download.IsEnabled = true;
                return;
            }

            
            int countConnections;
            if (!Int32.TryParse(TextBox_Count_Connections.Text.Trim(), out countConnections)) {
                countConnections = 1;
            }

            if (countConnections <= 0) {
                countConnections = 1;
            }

            CountConnections = countConnections;
            CountThreadsWaiting = 0;

            Url = TextBox_Url.Text.Trim();
            SavePath = TextBox_SavePath.Text.Trim();
            Timestamp = TextBox_Timestamp.Text.Trim();

            WebArchiveCom.Timestamp = Timestamp;
            WebPage.SavePath = SavePath;
            WebPage.Encoding = ComboBox_Encoding.SelectedValue.ToString();

            Queue.Clear();
            listPaths.Clear();
            WebPage.ListSavedPaths.Clear();

            Label_Count_Downloaded_Pages.Content = 0;
            Label_Count_Queued_Pages.Content = 0;
            Label_Count_Failed_Downloads.Content = 0;

            Thread tt = new Thread(new ThreadStart(ThreadC));
            tt.Start();
        }

        void ThreadC() {
            if (!WebArchiveCom.IsArchived(Url, Timestamp)) {
                MessageBox.Show("That url and time stamp combination is not archived.");

                Dispatcher.Invoke((Action)delegate{
                    Button_Download.IsEnabled = true;
                });

                return;
            }

            StopThreads = false;
            for (int i = 0; i < CountConnections; i++) {
                Thread t = new Thread(new ThreadStart(ThreadA));
                t.Start();
            }

            Thread tt = new Thread(new ThreadStart(ThreadB));
            tt.Start();

            WebPage webpage = new WebPage();
            webpage.Url = Url;
            Queue.Enqueue(webpage);
            PagesQueued(1);

            Dispatcher.Invoke((Action)delegate{
                Button_Stop_Download.IsEnabled = true;
            });
        }

        void DownloadCompleted() {
            Dispatcher.Invoke((Action)delegate{
                Button_Download.IsEnabled = true;
                Button_Stop_Download.IsEnabled = false;
            });
        }

        void Stop() {
            StopThreads = true;
            Thread tt = new Thread(new ThreadStart(ThreadAAA));
            tt.Start();
            WebPage.StopWorking = true;
        }

        void ThreadAAA() {
            while (CountConnections > 0) {
                System.Threading.Thread.Sleep(1000);
            }

            Dispatcher.Invoke((Action)delegate{
                //Button_Stop_Download.IsEnabled = false;
            });
        }

        void PagesDownloading(int x) {
            lock (_locker) {
                Dispatcher.Invoke((Action)delegate{
                    int a;

                    if (!Int32.TryParse(Label_Count_Downloading_Pages.Content.ToString().Trim(), out a)) {
                        a = 0;
                    }

                    Label_Count_Downloading_Pages.Content = a + x;
                });
            }
        }

        void PagesDownloaded(int x) {
            lock (_locker) {
                Dispatcher.Invoke((Action)delegate{
                    int a;

                    if (!Int32.TryParse(Label_Count_Downloaded_Pages.Content.ToString().Trim(), out a)) {
                        a = 0;
                    }

                    Label_Count_Downloaded_Pages.Content = a + x;
                });
            }
        }

        void PagesQueued(int x) {
            lock (_locker) {
                Dispatcher.Invoke((Action)delegate{
                    int a;

                    if (!Int32.TryParse(Label_Count_Queued_Pages.Content.ToString().Trim(), out a)) {
                        a = 0;
                    }

                    Label_Count_Queued_Pages.Content = a + x;
                });
            }
        }

        void PagesFailed(int x) {
            lock (_locker) {
                Dispatcher.Invoke((Action)delegate{
                    int a;

                    if (!Int32.TryParse(Label_Count_Failed_Downloads.Content.ToString().Trim(), out a)) {
                        a = 0;
                    }

                    Label_Count_Failed_Downloads.Content = a + x;
                });
            }
        }

        private void ThreadB() {
            DateTime startTime = DateTime.Now;
            bool waiting = false;
            do {
                if (CountThreadsWaiting == CountConnections) {
                    if (!waiting) {
                        startTime = DateTime.Now;
                        waiting = true;
                    } else { 
                        if (DateTime.Now.Subtract(startTime).Seconds > 1) {
                            StopThreads = true;
                            while (CountConnections > 0) {
                                Thread.Sleep(1000);
                            }

                            DownloadCompleted();
                            break;
                        }
                    }
                } else {
                    if (waiting) {
                        waiting = false;
                    }
                    
                    Thread.Sleep(100);
                }
            } while(true);
        }

        private void ThreadA() {
            bool updatedStatus = false;
            WebPage cpWebPage = null;
            do {
                lock (Queue) {
                    if (Queue.Count == 0) {
                        if (!updatedStatus) {
                            Interlocked.Increment(ref CountThreadsWaiting);
                            updatedStatus = true;
                        }
                        Thread.Sleep(10);
                        continue;
                    }

                    cpWebPage = Queue.Dequeue() as WebPage;
                    PagesQueued(-1);
                }

                if (updatedStatus) {
                    updatedStatus = false;
                    Interlocked.Decrement(ref CountThreadsWaiting);
                }

                cpWebPage.Download();
                if (StopThreads) break;
                cpWebPage.GetAnchors();
                int countAnchors = cpWebPage.ListAnchorHrefs.Count;
                if (countAnchors > 0) {
                    for(int i = 0; i < countAnchors; i++) {
                        WebPage wp = new WebPage();
                        wp.Url = cpWebPage.ListAnchorHrefs[i];

                        Uri wpUri = new Uri(wp.Url);
                        string p = wpUri.PathAndQuery;
                        int index = listPaths.BinarySearch(p);
                        if (index < 0) {
                            listPaths.Insert(~index, p);
                        } else {
                            continue;
                        }

                        Queue.Enqueue(wp);
                        PagesQueued(1);
                    }
                }
                
                PagesDownloading(1);
                if (StopThreads) break;
                cpWebPage.GetImages();
                if (StopThreads) break;
                cpWebPage.GetScripts();
                if (StopThreads) break;
                cpWebPage.GetCss();
                if (StopThreads) break;
                cpWebPage.GetCssUrls();
                if (StopThreads) break;
                cpWebPage.Save();
                PagesDownloaded(1);
                PagesDownloading(-1);
                
            } while(!StopThreads);

            Interlocked.Decrement(ref CountConnections);
        }

        private void Button_View_Log_Click(object sender, RoutedEventArgs e) {
            System.Diagnostics.Process.Start(App.AppDir+"log.txt");
        }

        private void Button_Stop_Download_Click(object sender, RoutedEventArgs e) {
            Button_Stop_Download.IsEnabled = false;
            Stop();
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e) {
            if (!Button_Download.IsEnabled) {
                e.Cancel = true;
                MessageBox.Show("Stop the downloads first!");
            }
        }

        private void TextBox_Count_Connections_TextChanged(object sender, TextChangedEventArgs e) {

        }

        private void Window_Loaded(object sender, RoutedEventArgs e) {
            foreach( EncodingInfo ei in Encoding.GetEncodings() )  {
                ComboBox_Encoding.Items.Add(ei.Name);
            }
            ComboBox_Encoding.SelectedValue = "utf-8";
        }
    }
}
