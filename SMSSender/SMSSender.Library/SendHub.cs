using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Runtime.Serialization.Json;
using System.IO;

namespace SMSSender.Library {
    public class SendHub {
		public string Api_Key {get; set;}
		public string User_Name {get; set;}

		public bool Validate() {
			using(var web_client = new WebClient()) {
				try {
					web_client.DownloadString("https://api.sendhub.com/v1/contacts/?username="+User_Name+"&api_key="+Api_Key);
				} catch(WebException ex) {
					return false;
				}
			}

			return true;
		}

		public string SendSMS(List<string> list_numbers, string text) {
			List<string> list_n = new List<string>();
			foreach(var i in list_numbers) {
				list_n.Add("+1"+i);
			}

			SMSJsonObject sms_json_obj = new SMSJsonObject();
			sms_json_obj.contacts = list_n;
			sms_json_obj.text = text;

			DataContractJsonSerializer json_serializer = new DataContractJsonSerializer(typeof(SMSJsonObject));
			MemoryStream ms = new MemoryStream();
			json_serializer.WriteObject(ms, sms_json_obj);
			ms.Seek(0, SeekOrigin.Begin);
			var sr = new StreamReader(ms);
			string json_post_data = sr.ReadToEnd();
			//Console.WriteLine("posting ["+json_post_data+"]");
			using(var web_client = new WebClient()) {
				web_client.Headers["Content-Type"] = "application/json";
				return web_client.UploadString("https://api.sendhub.com/v1/messages/?username="+User_Name+"&api_key="+Api_Key, "POST", json_post_data);
			}
		}
    }
}
