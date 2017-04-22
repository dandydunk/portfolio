using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;

namespace SMSSender.Library {
	public class WebSocket {
		static public string
		DownloadDataPost(string url, string data) {
			using(var web_client = new WebClient()) {
				return web_client.UploadString(url, "POST", data);
			}
		}
	}
}
