using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.Serialization.Json;
using System.IO;
using System.Net;

namespace ConsoleApplication1 {
	public class JsonRootObject {
		public List<string> contacts { get; set; }
		public string text { get; set; }
	}


	class Program {
		Stack<object> stack_objects = new Stack<object>();
		void GetP(Object obj) {
			stack_objects.Push(obj);

			while(stack_objects.Count > 0) {
				Object o = stack_objects.Pop();
				foreach(System.Reflection.PropertyInfo pi in o.GetType().GetProperties()) {
					Console.WriteLine(pi.Name);
					stack_objects.Push(pi.GetValue(o));
				}
			}
		}

		static void Main(string[] args) {
			/*JsonRootObject js = new JsonRootObject();
			js.contacts = new List<string>();
			js.contacts.Add("+number1");
			js.contacts.Add("+number2");
			js.contacts.Add("+number3");
			js.text = "my text";

			DataContractJsonSerializer d = new DataContractJsonSerializer(typeof(JsonRootObject));
			MemoryStream ms = new MemoryStream();
			d.WriteObject(ms, js);
			ms.Seek(0, SeekOrigin.Begin);
			var sr = new StreamReader(ms);
			Console.WriteLine(sr.ReadToEnd());*/

			/**HttpWebRequest r = (HttpWebRequest)WebRequest.Create("https://api.sendhub.com/v1/contacts/?username=112066816615&api_key=1230cd847431e6ecac2290daa5cbd729727efbe51");
			//var s = r.GetRequestStream();
			HttpWebResponse ss = (HttpWebResponse)r.GetResponse();
			Console.WriteLine("status = " + ss.StatusCode);*/
			DateTime a = new DateTime();
			foreach(System.Reflection.PropertyInfo pi in a.GetType().GetProperties()) {
				Console.WriteLine(pi.Name);
			}
		}
	}
}
