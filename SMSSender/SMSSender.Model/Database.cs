using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Data.SqlClient;

namespace SMSSender.Model {
	public class Appointment {
		public Physician Physician {get; set;}
		public int Count;
		public DateTime First_Case_Start_Time;
		public DateTime Last_Case_Start_Time;
		public DateTime Last_Case_End_Time;
		public DateTime Date;
	}

	public class Physician {
		public int Id {get; set;}
		public string First_Name {get; set;}
		public string Last_Name {get; set;}
		public string Mobile_Number_1 {get; set;}
		public string Mobile_Number_2 {get; set;}
		public bool NextDaySx {get; set;}
	}

	public class ConfigOptions {
		public string User_Name {get; set;}
		public string Api_Key {get; set;}
		public int Days_Ahead {get; set;}
		public bool Enabled {get; set;}
		public string Next_Day_Sx_Msg {get; set;}
		public bool Next_Day_Sx {get; set;}
		public string Facility_Name {get; set;}
	}

    public class Database {
		public string Conn_Str {get; set;}

		static public string BuildSMSMessage(Appointment appointment, ConfigOptions options) {
			string sms_msg = options.Next_Day_Sx_Msg;
			sms_msg = sms_msg.Replace("[FIRSTNAME]", appointment.Physician.First_Name);
			sms_msg = sms_msg.Replace("[LASTNAME]", appointment.Physician.Last_Name);
			sms_msg = sms_msg.Replace("[FACILITYNAME]", options.Facility_Name);
			sms_msg = sms_msg.Replace("[FIRSTCASESTART]", appointment.First_Case_Start_Time.ToString("hh:mm tt"));
			sms_msg = sms_msg.Replace("[LASTCASESTART]", appointment.Last_Case_Start_Time.ToString("hh:mm tt"));
			sms_msg = sms_msg.Replace("[LASTCASEEND]", appointment.Last_Case_End_Time.ToString("hh:mm tt"));
			sms_msg = sms_msg.Replace("[SXDATE]", appointment.Date.ToString("d"));
			sms_msg = sms_msg.Replace("[CASECOUNT]", appointment.Count.ToString());

			return sms_msg;
		}

		public ConfigOptions
		GetConfigOptions() {
			using(SqlConnection sql_conn = new SqlConnection(Conn_Str)) {
				sql_conn.Open();
				SqlCommand sql_cmd = sql_conn.CreateCommand();
				sql_cmd.CommandText = @"SELECT shUsername, shApi_key, NextDaySxMsg, DaysAhead, Enabled, NextDaySx, FacilityName
										FROM Modules_SMS_Config";
				SqlDataReader sql_reader = sql_cmd.ExecuteReader();
				if(!sql_reader.Read()) {
					return null;
				}

				ConfigOptions co = new ConfigOptions();
				if(!sql_reader.IsDBNull(0)) co.User_Name = sql_reader.GetString(0);
				if(!sql_reader.IsDBNull(1)) co.Api_Key = sql_reader.GetString(1);
				if(!sql_reader.IsDBNull(2)) co.Next_Day_Sx_Msg = sql_reader.GetString(2);
				if(!sql_reader.IsDBNull(3)) co.Days_Ahead = sql_reader.GetInt32(3);
				if(!sql_reader.IsDBNull(4)) co.Enabled = sql_reader.GetSqlBoolean(4).IsTrue  ? true:false;
				if(!sql_reader.IsDBNull(5)) co.Next_Day_Sx = sql_reader.GetSqlBoolean(5).IsTrue ? true:false;
				if(!sql_reader.IsDBNull(6)) co.Facility_Name = sql_reader.GetString(6);

				return co;
			}
		}
		
		public void UpdateStatus(int days_ahead, int sms_sent, int sms_failed) {
			using(SqlConnection sql_conn = new SqlConnection(Conn_Str)) {
				sql_conn.Open();
				SqlCommand sql_cmd = sql_conn.CreateCommand();

				var today_date = DateTime.Now;
				var target_date = today_date.AddDays(days_ahead);
				sql_cmd.CommandText = "UPDATE Modules_SMS_Config SET NextDaySxStatus = @data";
				//Last run 1/1/0001 12:00:00 AM. Target date: 1/2/00
				sql_cmd.Parameters.Add(new SqlParameter("@data", System.Data.SqlDbType.Text, 200){ Value = "Last run "+today_date.ToString()+". Target date: "+target_date.ToString("d")+". "+sms_sent+" SMS Messages Sent. "+sms_failed+" SMS Messages Failed."});

				sql_cmd.Prepare();
				sql_cmd.ExecuteNonQuery();
			}
		}

		public void AddLog(int physician_id, string mobile, string sms_content, string send_hub_response) {
			using(SqlConnection sql_conn = new SqlConnection(Conn_Str)) {
				sql_conn.Open();
				SqlCommand sql_cmd = sql_conn.CreateCommand();
				sql_cmd.CommandText = @"INSERT INTO Modules_SMS_Log(PhysicianID, MobileNumber, SendConfirmation, SMSContent, shDataResponse, SentDateTime, Job)
										VALUES(@id, @mobile, 1, @sms_content, @send_hub_response, GetDate(), 'NDSN')";
				
				sql_cmd.Parameters.Add(new SqlParameter("@id", System.Data.SqlDbType.Int){Value = physician_id});
				sql_cmd.Parameters.Add(new SqlParameter("@mobile", System.Data.SqlDbType.Text, 20){ Value = mobile});
				sql_cmd.Parameters.Add(new SqlParameter("@sms_content", System.Data.SqlDbType.Text, 450){ Value = sms_content});
				sql_cmd.Parameters.Add(new SqlParameter("@send_hub_response", System.Data.SqlDbType.Text, 250){ Value = send_hub_response});

				sql_cmd.Prepare();
				sql_cmd.ExecuteNonQuery();
			}
		}

		public delegate void CallBack_GetAppointments(Appointment appointment);
		public void
		GetAppointments(CallBack_GetAppointments cb_get_appointments, int days_ahead = 1) {
			using(SqlConnection sql_conn = new SqlConnection(Conn_Str)) {
				sql_conn.Open();
				SqlCommand sql_cmd = sql_conn.CreateCommand();
				sql_cmd.CommandText = @"SELECT
										Physician.PhysicianId, Physician.FirstName, Physician.LastName, Physician.MobileNumber1, Physician.MobileNumber2, Physician.NextDaySx, 
										Appointments.Date,
										COUNT(Appointments.Date) AS CaseCount,
										MIN(CAST(Appointments.ApptStartTime AS DateTime)) AS FirstCaseStart,
										MAX(CAST(Appointments.ApptStartTime AS DateTime)) AS LastCaseStart,
										MAX(CAST(Appointments.ApptEndTime AS DateTime)) AS LastCaseEnd
										FROM Physician INNER JOIN Appointments ON Physician.PhysicianId = Appointments.PhysicianId
										WHERE (Physician.NextDaySx = 1) AND 
										((DATEPART(year, Appointments.Date) = DATEPART(year, DATEADD(day, "+days_ahead+@", GetDate()))) AND
										(DATEPART(month, Appointments.Date) = DATEPART(month, DATEADD(day, "+days_ahead+@", GetDate()))) AND
										(DATEPART(day, Appointments.Date) = DATEPART(day, DATEADD(day, "+days_ahead+@", GetDate()))))
										GROUP BY Physician.FirstName, Physician.LastName, Physician.PhysicianId, Appointments.Date, Physician.MobileNumber1,
										Physician.MobileNumber2, Physician.NextDaySx
										ORDER BY Appointments.Date";
				SqlDataReader sql_reader = sql_cmd.ExecuteReader();
			
				while(sql_reader.Read()) {
					Physician p = new Physician();
					if(!sql_reader.IsDBNull(0)) p.Id = sql_reader.GetInt32(0);
					if(!sql_reader.IsDBNull(1)) p.First_Name = sql_reader.GetString(1);
					if(!sql_reader.IsDBNull(2)) p.Last_Name = sql_reader.GetString(2);
					if(!sql_reader.IsDBNull(3)) p.Mobile_Number_1 = sql_reader.GetString(3);
					if(!sql_reader.IsDBNull(4)) p.Mobile_Number_2 = sql_reader.GetString(4);
					if(!sql_reader.IsDBNull(5)) p.NextDaySx = sql_reader.GetSqlBoolean(5).IsTrue ? true:false;

					Appointment a = new Appointment();
					a.Physician = p;
					if(!sql_reader.IsDBNull(6)) a.Date = sql_reader.GetDateTime(6);
					if(!sql_reader.IsDBNull(7)) a.Count = sql_reader.GetInt32(7);
					if(!sql_reader.IsDBNull(8)) a.First_Case_Start_Time = sql_reader.GetDateTime(8);
					if(!sql_reader.IsDBNull(9)) a.Last_Case_Start_Time = sql_reader.GetDateTime(9);
					if(!sql_reader.IsDBNull(10)) a.Last_Case_End_Time = sql_reader.GetDateTime(10);

					cb_get_appointments(a);
				}
			}
		}
    }
}
