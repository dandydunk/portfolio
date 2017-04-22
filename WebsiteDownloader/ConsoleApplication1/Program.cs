using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using WebsiteDownloaderLib;
using System.Net.Http;
using System.Text.RegularExpressions;
using System.IO;
using System.Web;
using System.Net;

namespace ConsoleApplication1 {
    class Program {
        static async void A() {
            Console.WriteLine("HERE");
            HttpClient client = new HttpClient();
            HttpResponseMessage r = client.GetAsync("http://web.archive.org/web/20150607/http://stackoverflow.com/", 
                                                                HttpCompletionOption.ResponseHeadersRead).Result;
            
            //response.EnsureSuccessStatusCode();
            string responseUri =  r.RequestMessage.RequestUri.ToString();
            Console.WriteLine("URL = "+responseUri);
        }
        //11111111 11111111 11111111 11111111
        static public void Main(string[] args) {
            if (WebArchiveCom.IsArchived("111google.com", "2012")) {
                Console.WriteLine("yes");
            } else { 
                Console.WriteLine("no");
            }
        }
    }
}
