var pages = require('../pages.js');
var user_systems_access = require("../models/user_systems_access.js");
var time_card = require("../models/time_card.js");
var system = require("../models/system.js");
var User = require("../models/user.js");
var UserChangeHistory = require("../models/user_change_history.js")
var Async = require("async");

var geocoderProvider = 'google';
var httpAdapter = 'https';
// optionnal
var extra = {
    apiKey: 'AIzaSyAVC66Ca3DqVo16pZZ_h-8zQ-V4277R4WU', // for Mapquest, OpenCage, Google Premier
    formatter: null         // 'gpx', 'string', ...
};

var geocoder = require('node-geocoder')(geocoderProvider, httpAdapter, extra);

module.exports = {
	ClockIn:function(user, t, cb){
		if(isNaN(Date.parse(t["clock_time"]))) {
			cb("The clock in date is not in correct format: Month/Day/Year->06/04/2015");
		}

		if(!t["latitude"] || t["latitude"].length == 0) {
			cb("The latitude is not set.");
		}

		if(!t["longitude"] || t["longitude"].length == 0) {
			cb("The longitude is not set.");
		}

		geocoder.reverse({lat:t["latitude"], lon:t["longitude"]}, function(err, res) {
			if(err) {
				cb(err);
			}

		    else {
		    	var p = {
							user : user._id,
							clock_in: {
								latitude : t["latitude"],
								longitude : t["longitude"],
								date_time:t["clock_time"],
								city: res[0].city,
								state:res[0].state,
								street_number:res[0].streetNumber,
								street_name:res[0].streetName,
								zip_code:res[0].zipcode
							}
						}
				
				var tc = new time_card(p);
				tc.save(function(err, obj){
					if(err) {
						cb("database error clocking in, try again");
					}

					else {
						cb(null, obj);
					}
				});
		    }
		});

		/*
		*/
	},
	ClockOut:function(timecard, t, cb){
		if(!t["clock_time"] || isNaN(Date.parse(t["clock_time"]))) {
			cb("The clock out date is not in correct format: Month/Day/Year->06/04/2015");
		}

		if(!t["latitude"] || t["latitude"].length == 0) {
			cb("The latitude is not set.");
		}

		if(!t["longitude"] || t["longitude"].length == 0) {
			cb("The longitude is not set.");
		}

		geocoder.reverse({lat:t["latitude"], lon:t["longitude"]}, function(err, res) {
			if(err) {
				cb(err)
			}

			else {
				var conditions = { _id: timecard._id };
				var update = { 
								clock_out: {
								latitude : t["latitude"],
								longitude : t["longitude"],
								date_time:t["clock_time"],
								city: res[0].city,
								state:res[0].state,
								street_number:res[0].streetNumber,
								street_name:res[0].streetName,
								zip_code:res[0].zipcode
							}
							};
				var options = { multi: true };

				console.log("updating", update);
				time_card.update(conditions, update, options, function (err, numAffected) {
					if(err) {
						console.log("database error", err);
						cb("error updating the time card");
					}

					else {
						cb();
					}
				});
			}
		});
	}
};