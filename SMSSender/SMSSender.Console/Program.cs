using System;
using System.Collections.Generic;
using System.Collections;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using System.Net;

namespace SMSSender.Main {
	class Program {
		public static Mutex Mutex = null;
		public static Model.ConfigOptions Config_Options = null;
		public static Stack Stack_Appointments = null;
		public static Library.SendHub Send_Hub = new Library.SendHub();
		public static Model.Database Database = new Model.Database();
		public static log4net.ILog Logger = null;
		public static int Count_SMS_Failures = 0;

		static void DoWork() {
			while(Stack_Appointments.Count > 0) {
				Model.Appointment appointment = (Model.Appointment)Stack_Appointments.Pop();
				string sms_msg = Model.Database.BuildSMSMessage(appointment, Config_Options);

				List<string> list_numbers = new List<string>();
				if(!String.IsNullOrWhiteSpace(appointment.Physician.Mobile_Number_1)) {
					list_numbers.Clear();
					list_numbers.Add(appointment.Physician.Mobile_Number_1);
					string response = null;

					try {
						Print("Sending SMS to ["+appointment.Physician.Mobile_Number_1+"] MSG["+sms_msg+"]");
						response = Send_Hub.SendSMS(list_numbers, sms_msg);
					} catch(WebException ex) {
						Print("There was a problem sending a SMS to ["+appointment.Physician.Mobile_Number_1+"] ["+ex.Message+"]");
						Logger.Error("There was a problem sending a SMS to ["+appointment.Physician.Mobile_Number_1+"] ["+ex.Message+"]");
						Database.AddLog(appointment.Physician.Id, appointment.Physician.Mobile_Number_1, sms_msg, "FAILED ["+ex.Message+"]");

						Interlocked.Increment(ref Count_SMS_Failures);
						
						continue;
					}

					Database.AddLog(appointment.Physician.Id, appointment.Physician.Mobile_Number_1, sms_msg, response);

				}

				if(!String.IsNullOrWhiteSpace(appointment.Physician.Mobile_Number_2)) {
					list_numbers.Clear();
					list_numbers.Add(appointment.Physician.Mobile_Number_2);
					string response = null;

					try {
						Print("Sending SMS to ["+appointment.Physician.Mobile_Number_2+"] MSG["+sms_msg+"]");
						response = Send_Hub.SendSMS(list_numbers, sms_msg);
					} catch(WebException ex) {
						Print("There was a problem sending a SMS to ["+appointment.Physician.Mobile_Number_2+"] ["+ex.Message+"]");
						Logger.Error("There was a problem sending a SMS to ["+appointment.Physician.Mobile_Number_2+"] ["+ex.Message+"]");
						Database.AddLog(appointment.Physician.Id, appointment.Physician.Mobile_Number_2, sms_msg, "FAILED ["+ex.Message+"]");

						Interlocked.Increment(ref Count_SMS_Failures);
						
						continue;
					}

					Database.AddLog(appointment.Physician.Id, appointment.Physician.Mobile_Number_2, sms_msg, response);
				}
			}
		}

		static void Print(string txt) {
			Console.WriteLine(txt);
			if(null != Logger) Logger.Info(txt);
		}

		static void Main(string[] args) {
			if(args.Length < 1) {
				Print("The database name is missing.");
				return;
			}

			/* */
			try {
				Mutex = new Mutex(true, "riooeijiejrie");
			} catch(Exception ex) {
				Print("Program is already running...");
				return;
			}

			/* set up the log file */
			string log_path = Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData)+"\\SMSSender\\";
			if(!System.IO.Directory.Exists(log_path)) {
				Print("Setting up the log file...");
				try {
					System.IO.Directory.CreateDirectory(log_path);
				} catch(System.IO.IOException ex) {

				}
			}

			Print("Log path ["+log_path+"]");

			log4net.GlobalContext.Properties["LogFilePath"] = log_path;
			log4net.Config.XmlConfigurator.Configure();
            Logger = log4net.LogManager.GetLogger(typeof(Program));

			Logger.Info("App started.");

			/* */
			Stack_Appointments = Stack.Synchronized(new Stack());

			
			string database_username = System.Configuration.ConfigurationManager.AppSettings["database_username"];
			string database_password = System.Configuration.ConfigurationManager.AppSettings["database_password"];
			string database_host = System.Configuration.ConfigurationManager.AppSettings["database_host"];
			string database_name = args[0];
			Database.Conn_Str = @"server="+database_host+";database="+database_name+";uid="+database_username+";pwd="+database_password;

			Print("Getting the config options...");
			Config_Options = Database.GetConfigOptions();
			if(!Config_Options.Enabled) {
				Print("SMS is not enabled...");
				return;
			}

			if(!Config_Options.Next_Day_Sx) {
				Print("Next Day Sx is not enabled...");
				return;
			}
			
			Send_Hub.Api_Key = Config_Options.Api_Key;
			Send_Hub.User_Name = Config_Options.User_Name;

			Print("Validating send hub credentials...");
			if(!Send_Hub.Validate()) {
				Print("Send Hub credentials are incorrect.");
				return;
			}

			Print("Getting physicians appointments...");
			Database.GetAppointments((o)=>{
				Stack_Appointments.Push(o);
			}, Config_Options.Days_Ahead);

			int count_appointments = Stack_Appointments.Count;
			Print("Got ["+Stack_Appointments.Count+"] appointments.");

			int thread_count = (int)Math.Ceiling((double)Stack_Appointments.Count / 5);
			if(thread_count > 3) thread_count = 3;

			Print("Making ["+thread_count+"] threads");
			Thread[] threads = new Thread[thread_count];
			for(int i = 0; i < thread_count; i++) {
				threads[i] = new Thread(new ThreadStart(DoWork));
				threads[i].Start();
			}

			for(int i = 0; i < thread_count; i++) {
				threads[i].Join();
			}

			Database.UpdateStatus(Config_Options.Days_Ahead, count_appointments - Count_SMS_Failures, Count_SMS_Failures);
			Logger.Info("Finished.");
		}
	}
}
