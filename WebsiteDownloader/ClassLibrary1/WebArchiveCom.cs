using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Http;
using System.IO;
using System.Net;

namespace WebsiteDownloaderLib {
    public delegate void DelFailedDownload(string path, HttpStatusCode statusCode);

    public class WebArchiveCom {
        public static string Timestamp;
        public static DelFailedDownload FailedDownloadHandler = null;

        public static bool IsArchived(string url, string timestamp) {
            WebClient wc = new WebClient();

            string dUrl = String.Format("http://archive.org/wayback/available?url={0}&timestamp={1}", url, timestamp);
            string html = "";
            int retries = 0;
            do {
                try {
                    html = wc.DownloadString(dUrl);
                    break;
                } catch (WebException ex) {
                    HttpWebResponse r = ex.Response as HttpWebResponse;
                    
                    if (r != null) {
                        if (r.StatusCode == HttpStatusCode.NotFound || 
                            r.StatusCode == HttpStatusCode.BadRequest ||
                            r.StatusCode == HttpStatusCode.Forbidden ||
                            r.StatusCode == HttpStatusCode.Gone) {
                            throw ex;
                        }
                    } else {
                        throw ex;
                    }

                    if (retries++ > 5) {
                        throw ex;
                    } else {
                        System.Threading.Thread.Sleep(5000);
                    }
                }
            } while(true);

            return html.IndexOf("\"available\":true") == -1 ? false:true;
        }

        public static string GetNewUrl(string failedUrl) {
            WebClient wc = new WebClient();

            string dUrl = String.Format("http://web.archive.org/cdx/search/cdx?url={0}", failedUrl);

            int retries = 0;
            string html = "";
            do {
                try {
                    //Console.WriteLine("connecting to website {0}", dUrl);
                    html = wc.DownloadString(dUrl);
                    break;
                } catch (WebException ex) {
                    HttpWebResponse r = ex.Response as HttpWebResponse;
                    
                    if (r != null) {
                        //Console.WriteLine("failed [{0}] downloading {1}...", r.StatusCode, url);
                        if (r.StatusCode == HttpStatusCode.NotFound || 
                            r.StatusCode == HttpStatusCode.BadRequest ||
                            r.StatusCode == HttpStatusCode.Forbidden ||
                            r.StatusCode == HttpStatusCode.Gone) {
                            
                            break;
                        }
                    } else {
                        //Console.WriteLine("failed downloading {0}...", url);
                       
                    }

                    if (retries++ > 5) {
                        break;
                    } else {
                        System.Threading.Thread.Sleep(5000);
                    }
                }
            } while(true);

            if (0 == html.Length) {
                return "";
            }

            string[] s = html.Split(new string[]{"\r\n"}, StringSplitOptions.RemoveEmptyEntries);
            string newUrl = "";
            foreach (var line in s) {
                string[] ss = line.Split(new string[]{" "}, StringSplitOptions.RemoveEmptyEntries);
                string timeStamp = ss[1];
                newUrl = String.Format("http://web.archive.org/web/{0}id_/{1}", timeStamp, failedUrl);
            }

            return newUrl;
        }

        public static string GetRealUrl(string url) {
            if(!url.StartsWith("/web/")) return url;
            string needle = "/web/";
            int index = needle.Length;

            needle = "/";
            index = url.IndexOf(needle, index);
            if(index == -1) return url;

            index += needle.Length;

            return url.Substring(index);
        }

        public static string RemoveWayBackBar(string html) {
            string needle = "<!-- BEGIN WAYBACK TOOLBAR INSERT -->";
            int index = html.IndexOf(needle);
            if(index == -1) return html;

            int start_extract = index;

            index += needle.Length;

            needle = "<!-- END WAYBACK TOOLBAR INSERT -->";
            index = html.IndexOf(needle, index);
            index += needle.Length;

            string cpHtml = html.Substring(0, start_extract);
            cpHtml += html.Substring(index);

            return cpHtml;
        }

        public static string RemoveJS(string html) {
            html = html.Replace("<script type=\"text/javascript\" src=\"/static/js/analytics.js\"></script>", "");
            html = html.Replace("<link type=\"text/css\" rel=\"stylesheet\" href=\"/static/css/banner-styles.css\"/>", "");
            
            int index = 0;
            do {
                string needle = "<script type=\"text/javascript\">";
                index = html.IndexOf(needle, index);
                if(index == -1) return html;

                int startExtract = index;
                index += needle.Length;

                needle = "</script>";
                index = html.IndexOf(needle, index);
                if(index == -1) return html;

                index += needle.Length;
                int endExtract = index;

                string p = html.Substring(startExtract, endExtract - startExtract);
                if (p.IndexOf("archive_analytics.values.server_name") > -1) {
                    string c = html.Substring(0, startExtract);
                    c += html.Substring(endExtract);
                    return c;
                }
            } while(true);
        }

        public static string RemoveWayBackComments(string html) {
            int index = 0;
            do {
                string needle = "<!--";
                index = html.IndexOf(needle, index);
                if(index == -1) return html;

                int startExtract = index;
                index += needle.Length;

                needle = "-->";
                index = html.IndexOf(needle, index);
                if(index == -1) return html;

                index += needle.Length;
                int endExtract = index;

                string p = html.Substring(startExtract, endExtract - startExtract);
                if (p.IndexOf("FILE ARCHIVED") > -1) {
                    string c = html.Substring(0, startExtract);
                    c += html.Substring(endExtract);
                    return c;
                }
            } while(true);
        }

        public static string Download(string url, string encoding) {
            WebClient wc = new WebClient();
            wc.Encoding = Encoding.GetEncoding(encoding);
            
            string dUrl = String.Format("http://web.archive.org/web/{0}/{1}", Timestamp, url);

            int retries = 0;
            string html = "";
            bool triedNewUrl = false;
            
            do {
                try {
                    Console.WriteLine("downloading string {0}...", dUrl);
                    html = wc.DownloadString(dUrl);
                    break;
                } catch (WebException ex) {
                    HttpWebResponse r = ex.Response as HttpWebResponse;
                    
                    if (r != null) {
                        //Console.WriteLine("failed [{0}] downloading {1}...", r.StatusCode, url);
                        if (r.StatusCode == HttpStatusCode.NotFound || 
                            r.StatusCode == HttpStatusCode.BadRequest ||
                            r.StatusCode == HttpStatusCode.Forbidden ||
                            r.StatusCode == HttpStatusCode.Gone) {
                            if (triedNewUrl) {
                                if (FailedDownloadHandler != null) {
                                    FailedDownloadHandler(dUrl, r.StatusCode);
                                }

                                break;
                            }

                            triedNewUrl = true;
                            dUrl = WebArchiveCom.GetNewUrl(url);
                            if(dUrl.Length == 0) break;
                        }
                    }

                    if (retries++ > 5) {
                        if (triedNewUrl) {
                            if (FailedDownloadHandler != null) {
                                FailedDownloadHandler(dUrl, 0);
                            }
                            break;
                        }

                        triedNewUrl = true;
                        dUrl = WebArchiveCom.GetNewUrl(url);
                        if(dUrl.Length == 0) break;
                        retries = 0;
                    } else {
                        System.Threading.Thread.Sleep(5000);
                    }
                }
            } while(true);

            return html;
        }

        public static byte[] DownloadData(string url) {
            WebClient wc = new WebClient();

            string dUrl = String.Format("http://web.archive.org/web/{0}id_/{1}", Timestamp, url);

            int retries = 0;
            byte[] bytes = null;
            bool triedNewUrl = false;
            do {
                try {
                    Console.WriteLine("downloading data {0}...", dUrl);
                    bytes = wc.DownloadData(dUrl);
                    break;
                } catch (WebException ex) {
                    HttpWebResponse r = ex.Response as HttpWebResponse;
                    
                    if (r != null) {
                        //Console.WriteLine("failed [{0}] downloading {1}...", r.StatusCode, url);
                        if (r.StatusCode == HttpStatusCode.NotFound || 
                            r.StatusCode == HttpStatusCode.BadRequest ||
                            r.StatusCode == HttpStatusCode.Forbidden ||
                            r.StatusCode == HttpStatusCode.Gone) {
                            if (triedNewUrl) {
                                if (FailedDownloadHandler != null) {
                                    FailedDownloadHandler(dUrl, r.StatusCode);
                                }

                                break;
                            }

                            triedNewUrl = true;
                            dUrl = WebArchiveCom.GetNewUrl(url);
                            if(dUrl.Length == 0) break;
                        }
                    }

                    if (retries++ > 5) {
                        if (triedNewUrl) {
                            if (FailedDownloadHandler != null) {
                                FailedDownloadHandler(dUrl, 0);
                            }
                            break;
                        }

                        triedNewUrl = true;
                        dUrl = WebArchiveCom.GetNewUrl(url);
                        if(dUrl.Length == 0) break;
                        retries = 0;
                    } else {
                        System.Threading.Thread.Sleep(5000);
                    }
                }
            } while(true);

            return bytes;
        }
    }
}
