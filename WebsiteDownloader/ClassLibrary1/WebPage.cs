using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using HtmlAgilityPack;
using System.IO;
using System.Text.RegularExpressions;

namespace WebsiteDownloaderLib {
    public class WebPage {
        public static bool StopWorking = false;
        public static List<string> ListSavedPaths = new List<string>();
        
        public static string _savepath;

        public static string SavePath {
            get { return _savepath; }
            set {
                if (!value.EndsWith("\\")) {
                    value += "\\";
                }

                _savepath = value;
            }
        }
        public static string Encoding;

        public string html = "";
        string _url;
        string Host;
        public string Url {
            get { return _url; }
            set {
                Uri uri = new Uri(value);
                Host = uri.Host;
                if (!Host.StartsWith("www.")) {
                    Host = "www." + Host;
                }
                _url = value;
            }
        }
        public List<string> ListAnchorHrefs = new List<string>();
        List<string> ListCssUrls = new List<string>();

        public void Download() {
            this.html = WebArchiveCom.Download(Url, WebPage.Encoding);
            this.html = WebArchiveCom.RemoveWayBackBar(this.html);
            this.html = WebArchiveCom.RemoveWayBackComments(this.html);
            this.html = WebArchiveCom.RemoveJS(this.html);
        }

        static public bool IsPathSaved(string path) {
            int index = WebPage.ListSavedPaths.BinarySearch(path);
            if (index < 0) {
                WebPage.ListSavedPaths.Insert(~index, path);
                return false;
            }

            return true;
        }

        static public string GetSavePathFromUrl(string url) {
            return WebPage.GetSavePathFromUrl(url, true, true);
        }

        static public string GetSavePathFromUrl(string url, bool createDir, bool changeExtension, string extension = "html") {
            Uri uri = new Uri(url);
            string a = MakeValidHrefOrSrc(uri.PathAndQuery, changeExtension, extension);
            if (a.StartsWith("/")) {
                a = a.Substring(1);
            }

            a = a.Replace('/', '\\');
            
            string savePath = SavePath + a;
            if (createDir) {
                string savePathDir = null;
                try {
                    savePathDir = Path.GetDirectoryName(savePath);
                } catch (Exception) { return null; }
                if (!Directory.Exists(savePathDir)) {
                    //Console.WriteLine("creating dir {0}", savePathDir);
                    Directory.CreateDirectory(savePathDir);
                }
            }

            return savePath;
        }

        public static string MakeValidFileName(string name) {
            name = name.Replace(':', '_');
            name = name.Replace('?', '_');
            name = name.Replace('<', '_');
            name = name.Replace('>', '_');
            name = name.Replace('|', '_');
            name = name.Replace('*', '_');
            name = name.Replace('"', '_');
            return name;
        }

        static public string GetUrlFromPath(string cUrl, string path) {
            Uri uri = new Uri(cUrl);
            string host = uri.Host;

            string url = null;
            if (path.StartsWith("/")) {
                url = "http://" + host + path;
            } else {
                string uriUrlPath = uri.AbsolutePath;
                string d = "";
                if (uriUrlPath.Length > 1) {
                    try {
                        d = Path.GetDirectoryName(uriUrlPath).Replace("\\", "/");
                    } catch (Exception) {
                        return null;
                    }
                } else {
                    d = uriUrlPath;
                }

                if (!d.EndsWith("/")) {
                    d += "/";
                }

                if (!d.StartsWith("/")) {
                    d = "/" + d;
                }
                url = "http://" + host + d + path;
            }

            return url;
        }

        static public string MakeValidHrefOrSrc(string input, bool changeExtension = false, string extension = "html") {
            input = input.Replace('\\', '/');
            if (input.StartsWith("/")) {
                input = input.Substring(1);
            }

            Uri uri = new Uri("http://www.google.com/"+input);
            string path = MakeValidFileName(uri.AbsolutePath);
            string query = uri.Query;
            string newHref = "";
            if (query.Length > 0) {
                query = MakeValidFileName(query); //System.Web.HttpUtility.UrlEncode(query);
                path = path.Replace('.', '_');
                newHref = path + query + "/index." + extension;
            } else {
                string dir = Path.GetDirectoryName(path);
                if (dir == null) {
                    dir = "";
                }

                if (dir.Length > 0) {
                    dir = dir.Replace('\\', '/');
                }

                if (!dir.EndsWith("/")) {
                    dir += "/";
                }

                dir = dir.Replace('.', '_');

                string fileName = Path.GetFileName(path);
                if (!fileName.Contains('.')) {
                    fileName = fileName + "index.html";
                } else {
                    if (changeExtension) {
                        fileName = Path.ChangeExtension(fileName, "html");
                    }
                }

                newHref = dir + fileName;
            }

            return newHref;
        }

        public void Save() {
            if(html.Trim().Length == 0) return;

            string savePath = WebPage.GetSavePathFromUrl(Url);
            if(null == savePath) return;
            //Console.WriteLine("saving {0} to {1}...", Url, savePath);

            do {
                try {
                    File.WriteAllText(savePath, html);
                    break;
                } catch (IOException) { continue; }
            } while(true);
        }

        public void Save(string path, string bytes) {
            do {
                try {
                    File.WriteAllText(path, bytes);
                    break;
                } catch (IOException) { continue; }
            } while(true);
        }

        public void Save(string path, byte[] bytes) {
            do {
                try {
                    File.WriteAllBytes(path, bytes);
                    break;
                } catch (IOException) { continue; }
            } while(true);
        }

        public void GetCssUrls() {
            GetCssUrls(ref html, Url);
        }

        public void GetCssUrls(ref string text, string cssUrl) {
            MatchCollection mc = Regex.Matches(text, "url\\(['\"]?(.+?)['\"]?\\)");
            foreach (Match m in mc) {
                if(StopWorking) return;
                string href = m.Groups[1].Value;
                string cpHref = href;

                href = WebArchiveCom.GetRealUrl(href);

                text = text.Replace(cpHref, href);

                if (href.StartsWith("//")) {
                   continue;
                }

                try {
                    Uri uri = new Uri(href);
                    string host = uri.Host;
                    if (!host.StartsWith("www.")) {
                        host = "www." + host;
                    }

                    if (!host.Equals(Host)) {
                        continue;
                    }

                    href = uri.PathAndQuery;
                } catch (Exception) { }

                if (href.Length >= 255) continue;

                string chref = MakeValidHrefOrSrc(href, false, "css");
                text = text.Replace(href, chref);

                if(WebPage.IsPathSaved(href)) continue;

                string urlHref = GetUrlFromPath(cssUrl, href);
                if(urlHref == null) continue;

                string savePath = GetSavePathFromUrl(urlHref, true, false, "css");
                if(savePath == null) continue;
                if(File.Exists(savePath)) continue;

                byte[] bytes = WebArchiveCom.DownloadData(urlHref);
                if (bytes != null) {
                    //Console.WriteLine("saving css url {0} to {1}", href, savePath);
                    Save(savePath, bytes);
                }
            }
        }

        public void GetCss() {
            if(html.Trim().Length == 0) return;

            HtmlDocument htmlDocument = new HtmlDocument();
            htmlDocument.LoadHtml(this.html);
            IEnumerable<HtmlNode> styles = htmlDocument.DocumentNode.Descendants("link").Where(x => x.Attributes.Contains("href"));
            
            foreach (HtmlNode style in styles) {
                if(StopWorking) return;
                HtmlAttribute att = style.Attributes["href"];

                string href = style.Attributes["href"].Value;
                href = WebArchiveCom.GetRealUrl(href);

                att.Value = href;
                //Console.WriteLine("checking if {0} is a valid script...", src);

                if (href.StartsWith("//")) {
                    continue;
                }

               try {
                    Uri uri = new Uri(href);
                    string host = uri.Host;
                    if (!host.StartsWith("www.")) {
                        host = "www." + host;
                    }

                    if (!host.Equals(Host)) {
                        continue;
                    }

                    href = uri.PathAndQuery;
                } catch (Exception) { }

                if (href.Length >= 255) continue;

                
                att.Value = MakeValidHrefOrSrc(href, false, "css");

                if(WebPage.IsPathSaved(href)) continue;

                string urlHref = GetUrlFromPath(Url, href);
                if(urlHref == null) continue;
                string savePath = GetSavePathFromUrl(urlHref, true, false, "css");
                if(savePath == null) continue;

                if(File.Exists(savePath)) continue;

                string bytes = WebArchiveCom.Download(urlHref, WebPage.Encoding);
                if (bytes.Length > 0) {
                    //Console.WriteLine("saving css {0} to {1}", href, savePath);
                    GetCssUrls(ref bytes, urlHref);
                    Save(savePath, bytes);
                }
            }

            this.html = htmlDocument.DocumentNode.OuterHtml;
        }

        public void GetScripts() {
            if(html.Trim().Length == 0) return;

            HtmlDocument htmlDocument = new HtmlDocument();
            htmlDocument.LoadHtml(this.html);
            IEnumerable<HtmlNode> scripts = htmlDocument.DocumentNode.Descendants("script").Where(x => x.Attributes.Contains("src"));
            
            foreach (HtmlNode script in scripts) {
                if(StopWorking) return;
                HtmlAttribute att = script.Attributes["src"];

                string src = script.Attributes["src"].Value;
                //Console.WriteLine("checking if {0} is a valid script...", src);

                src = WebArchiveCom.GetRealUrl(src);
                att.Value = src;

                if (src.StartsWith("//")) {
                    continue;
                }

                try {
                    Uri uri = new Uri(src);
                    string host = uri.Host;
                    if (!host.StartsWith("www.")) {
                        host = "www." + host;
                    }

                    if (!host.Equals(Host)) {
                        continue;
                    }

                    src = uri.PathAndQuery;
                } catch (Exception) { }

                if (src.Length >= 255) continue;

                
                att.Value = MakeValidHrefOrSrc(src, false, "js");

                if(WebPage.IsPathSaved(src)) continue;

                string urlHref = GetUrlFromPath(Url, src);
                if(null == urlHref) continue;
                string savePath = GetSavePathFromUrl(urlHref, true, false, "js");
                if(null == savePath) continue;

                if(File.Exists(savePath)) continue;

                string bytes = WebArchiveCom.Download(urlHref, WebPage.Encoding);
                if (bytes.Length > 0) {
                    //Console.WriteLine("saving script {0} to {1}", src, savePath);
                    Save(savePath, bytes);
                }
            }

            this.html = htmlDocument.DocumentNode.OuterHtml;
        }

        public void GetImages() {
            if(html.Trim().Length == 0) return;

            HtmlDocument htmlDocument = new HtmlDocument();
            htmlDocument.LoadHtml(this.html);
            IEnumerable<HtmlNode> imgs = htmlDocument.DocumentNode.Descendants("img").Where(x => x.Attributes.Contains("src"));
            
            List<HtmlNode> listNodes = new List<HtmlNode>();
            foreach (HtmlNode img in imgs) {
                if(StopWorking) return;
                HtmlAttribute att = img.Attributes["src"];
                string src = img.Attributes["src"].Value;

                src = WebArchiveCom.GetRealUrl(src);
                att.Value = src;
                //Console.WriteLine("checking if {0} is a valid image...", src);

                if (src.StartsWith("//")) {
                    continue;
                }

                try {
                    Uri uri = new Uri(src);
                    string host = uri.Host;
                    if (!host.StartsWith("www.")) {
                        host = "www." + host;
                    }

                    if (!host.Equals(Host)) {
                        continue;
                    }

                    src = uri.PathAndQuery;
                } catch (Exception) { }

                if (src.Length >= 255) continue;

               
                att.Value = MakeValidHrefOrSrc(src);

                if(WebPage.IsPathSaved(src)) continue;

                string urlHref = GetUrlFromPath(Url, src);
                if(urlHref == null) continue;
                string savePath = GetSavePathFromUrl(urlHref, true, false);
                if(savePath == null) continue;

                byte[] bytes = WebArchiveCom.DownloadData(urlHref);
                if (bytes != null) {
                    //Console.WriteLine("saving image {0} to {1}", src, savePath);
                    Save(savePath, bytes);
                } else {
                    //img.ParentNode.RemoveChild(img);
                    listNodes.Add(img);
                }
            }

            foreach (HtmlNode node in listNodes) {
                //node.Remove();
                try {
                    htmlDocument.DocumentNode.SelectSingleNode(node.XPath).Remove();
                } catch (System.Xml.XPath.XPathException ex) { }
            }

            this.html = htmlDocument.DocumentNode.OuterHtml;
        }

        public void GetAnchors() {
            if(html.Trim().Length == 0) return;

            HtmlDocument htmlDocument = new HtmlDocument();
            htmlDocument.LoadHtml(this.html);
            IEnumerable<HtmlNode> links = htmlDocument.DocumentNode.Descendants("a").Where(x => x.Attributes.Contains("href"));
            
            foreach (HtmlNode link in links) {
                if(StopWorking) return;

                HtmlAttribute att = link.Attributes["href"];

                string href = link.Attributes["href"].Value.Trim();

                href = WebArchiveCom.GetRealUrl(href);
                att.Value = href;

                //Console.WriteLine("checking if {0} is a valid anchor...", href);
                if(href.StartsWith("mailto:", StringComparison.OrdinalIgnoreCase)) continue;

                if(href.StartsWith("#")) continue;

                if (href.StartsWith("//")) continue;

                try {
                    Uri uri = new Uri(href);
                    string host = uri.Host;
                    if (!host.StartsWith("www.")) {
                        host = "www." + host;
                    }

                    if (!host.Equals(Host)) {
                        continue;
                    }

                    href = uri.PathAndQuery;
                } catch (Exception) { }

                if (href.Length >= 255) continue;


                
                att.Value = MakeValidHrefOrSrc(href, true);

                if(WebPage.IsPathSaved(href)) continue;
                
                /* */
                string urlHref = GetUrlFromPath(Url, href);
                if(null == urlHref) continue;


                Uri uriUrlHref = new Uri(urlHref);
                if (uriUrlHref.Fragment.Length > 0) continue;

                string ex = "exe pdf txt gif jpeg png zip gz tar mp3 mov avi jpg bmp xml 7z doc docx";
                string fileEx = Path.GetExtension(uriUrlHref.AbsolutePath);
                if (fileEx.Length > 0) {
                    if(ex.IndexOf(fileEx) != -1) continue;
                }

                //Console.WriteLine("adding anchor {0} to list as {1}...", href, urlHref);
                ListAnchorHrefs.Add(urlHref);
            }

            this.html = htmlDocument.DocumentNode.OuterHtml;
        }
    }
}
