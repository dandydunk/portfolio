var express = require('express');
var router = express.Router();
var pages = require('../pages.js');
var User = require("../models/user.js");
var Job = require("../utils/job.js");
var time_card = require("../models/time_card.js");

router.get('/login', function (req, res) {
	res.render('login');
});

router.post('/login', function(req, res){
	if(req.session.user) {
		if(req.session.user.role == "admin") return res.json({redirect:"/"});
	}

	else {
		console.log("workfpoj");
		User.findOne({password:req.body.pass_code}).select('role first_name last_name').exec(function(err, user){
			if(err) {
				console.log("login error");
				return res.json({error:"login error"});
			}

			else {
				if(!user) {
					console.log("login failed");
					return res.json({error:"login failed"});
				}

				console.log("logged in as", user);

				if(user.role == "admin") {
					req.session.user = user;
					return res.json({redirect:"/"});
				}

				time_card.find({user:user._id}, function(error, timecards){
					if(error) {
						return res.json({error:error});
					}

					var timecard = null;
					var clock_out = false;
					for(var i = 0; i < timecards.length; i++) {
						timecard = timecards[i];
						if(timecard.clock_out && timecard.clock_out.date_time) {
							continue;
						}

						else {
							clock_out = true;
							break;
						}
					}

					if(clock_out) {
						console.log("clocking out...")
						Job.ClockOut(timecard, req.body, function(error){
							if(error) {
								return res.json({error:error});
							}

							console.log("clocked out.");
							return res.json({clocked_out:true, name:user.first_name+" "+user.last_name});
						});
					}

					else {
						console.log("clocking in", req.body);
						Job.ClockIn(user, req.body, function(error, tc){
							if(error) {
								console.log("error", error);
								return res.json({error:error});
							}

							console.log("clocked in.");
							return res.json({clocked_in:true, name:user.first_name+" "+user.last_name});
						});
					}
				})
			}
		});
	}
});

router.get('/logout', function (req, res) {
	req.session.destroy(function(err) {
		if(err) {
			console.log("error", err);
		}
			res.redirect('/');
	})
});


module.exports = router;