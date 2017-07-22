var express = require('express');
var router = express.Router();
var pages = require('../pages.js');
var user_systems_access = require("../models/user_systems_access.js");
var time_card = require("../models/time_card.js");
var system = require("../models/system.js");
var User = require("../models/user.js");
var UserChangeHistory = require("../models/user_change_history.js")
var Async = require("async");

router.use(function (req, res, next) {
  if(req.session.user.role != "admin") {
  	req.session.error = "access denied";
  	return res.redirect("/error");
  }

  next();
});

router.get('/', function (req, res) {
	return res.redirect("/");
});

router.post('/picture', function (req, res) {
	console.log("files", req.files);
	if(!req.body.user_id || !req.body.user_id.length) {
		req.session.error = "there isn't a user id";
		return res.redirect("/error");
	}

	if(!req.files || !req.files.picture || !req.files.picture.name || !req.files.picture.name.length) {
		req.session.error ="the picture didn't upload; try again";
		return res.redirect("/error");
	}

	User.findOne({_id:req.body.user_id}).exec(function(error, user){
		if(error) {
			req.session.error = "error finding the user.";
			return res.redirect("/error");
		}

		if(!user) {
			req.session.error ="user id not found";
			return res.redirect("/error");
		}

		var conditions = { _id: user._id };
		var update = {
					picture : req.files.picture.name
					};
		var options ={};//{ multi: true };

		console.log("updating user picture", update);
		User.update(conditions, update, options, function (err, numAffected) {
			if(err) {
				req.session.error ="there was a problem updating the user's picture";
				return res.redirect("/error");
			}

			else {
				console.log("updated picture");
				return res.redirect("/employee/"+req.body.user_id);
			}
		});
	});
});

router.get('/delete/:id', function (req, res) {
	if(req.session.user.role != "admin") {
		return res.json({error:"access denied"});
	}

	User.findOne({_id:req.params.id}).exec(function(error, user){
		if(error) {
			return res.json({error:"error finding the user."})
		}

		if(!user) {
			return res.json({error:"user id not found"})
		}
		
		console.log("removing user ", user);
		user.remove(function(){
			console.log("removed..");
			return res.json({success:true});
		});
	});
});

router.get('/paid_time_card/:id', function (req, res) {
	if(req.session.user.role != "admin") {
		return res.json({error:"access denied"});
	}

	time_card.findOne({_id:req.params.id}).populate('user', 'pay_rate').exec(function(error, timecard){
		if(error) {
			return res.json({error:"error finding the user."})
		}

		if(!timecard) {
			return res.json({error:"time card id not found"})
		}
		console.log(timecard);
		var conditions = { _id: timecard._id };
		var update = { paid:timecard.user[0].pay_rate
					};
		var options ={};//{ multi: true };

		console.log("updating", update);
		time_card.update(conditions, update, options, function (err, numAffected) {
			if(err) {
				return res.json({error:"there was a problem updating the time card"});
			}

			else {
				console.log("updated");
				return res.json({success:true});
			}
		});
	});
});

router.get('/update_admin_status/:id', function (req, res) {
	if(req.session.user.role != "admin") {
		return res.json({error:"access denied"});
	}

	User.findOne({_id:req.params.id}).exec(function(error, user){
		if(error) {
			return res.json({error:"error finding the user."})
		}

		if(!user) {
			return res.json({error:"user id not found"})
		}

		var conditions = { _id: user._id };
		var update = {
					role : (user.role != "admin") ? "admin":""
					};
		var options ={};//{ multi: true };

		console.log("updating user admin status", update);
		User.update(conditions, update, options, function (err, numAffected) {
			if(err) {
				return res.json({error:"there was a problem updating the user status"});
			}

			else {
				console.log("updated");
				return res.json({success:true});
			}
		});
	});
});

router.get('/save', function (req, res) {
	var bread_crumbs = pages.MakeBreadCrumb("save_employee");

	return res.render('save_employee', {page:{title:"Save Employee"}, list_bread_crumbs:bread_crumbs, user:{}});
});

router.get('/save/:id', function (req, res) {
	var bread_crumbs = pages.MakeBreadCrumb("save_employee");

	User.findOne({_id:req.params.id}, function(error, user){
		console.log("user", user)
		return res.render('save_employee', {page:{title:"Update Employee"}, list_bread_crumbs:bread_crumbs,
							user:user});
	});
});

function Validate(email, pass, id, cb) {
	Async.series([
		/* check email is unique */
		function(callback){
			User.findOne({email:email}, function(err, result){
				if(err) {
					callback(err, null);
				}
				else {
					if(result) {
						if((id && result._id != id) || !id) {
							callback("email exists", null);
						}
						else {
							callback(null);
						}
					}
					else {
						callback(null);
					}
				}
			})
		},

		/* check password is unique */
		function(callback){
			User.findOne({password:pass}, function(err, result){
				if(err) {
					callback(err, null);
				}
				else {
					if(result) {
						if((id && result._id != id) || !id) {
							callback("password exists", null);
						}
						else {
							callback(null);
						}
					}
					else {
						callback(null);
					}
				}
			})
		}
	], function(err, results){
		cb(err);
	});
}

router.post('/save', function (req, res) {
	//res.render('login');
	console.log("post", req.body);
	if(!req.body._id) {
		console.log("saving...");
		if(!req.body.first_name || req.body.first_name.length == 0) {
			req.session.error = "The first name is missing.";
			return res.redirect("/error");
		}

		if(!req.body.last_name || req.body.last_name.length == 0) {
			req.session.error = "The last name is missing.";
			return res.redirect("/error");
		}

		if(!req.body.pay_rate || req.body.pay_rate.length == 0) {
			req.session.error = "The pay rate is missing.";
			return res.redirect("/error");
		}

		if(!req.body.password || req.body.pay_rate.password == 0) {
			req.session.error = "The password is missing.";
			return res.redirect("/error");
		}
				
		Validate(req.body.email, req.body.password, null, function(err){
			if(err) {
				req.session.error = err;
				return res.redirect("/error");
			}

			else {
				

				var user = new User(req.body);

				user.save(function(err){
					console.log("redirecting...");
					return res.redirect("/employee/"+user._id);
				});
			}
		})
	}

	else {
		User.findOne({_id: req.body._id}, function(error, db_user){
			Validate(req.body.email, req.body.password, req.body._id, function(err){
				if(err) {
					req.session.error = err;
					return res.redirect("/error");
				}

				else {
					var conditions = { _id: req.body._id };
					var update = req.body;
					var options = { multi: true };
					console.log("updating employee", update);
					User.update(conditions, update, options, function (err, count_updated) {
						if(err) {
							req.session.error = err;
							return res.redirect("/error");
						}

						else {
							var funcs = new Array();
							for(var i in req.body) {
								if(!db_user[i] || db_user[i] == req.body[i]) continue;
								
								var f = (function(_db_user, ii){
									return function(callback){
										setTimeout(function(){

											var a = new UserChangeHistory({
																user:_db_user._id,
																admin:req.session.user,
																date_changed:new Date(),
																change_type:ii,
																old_value:_db_user[ii]
															});

											a.save(function(error){
												if(error) {
													callback(error, null);
												}

												else {
													console.log("updated", ii);
													callback(null, null);
												}
											});
										}, 100);
									};
								})(db_user, i);


								funcs.push(f);
							}

							Async.parallel(funcs, function(){
								console.log("finished updating...")
							})

							return res.redirect("/employee/"+req.body._id);
						}
					});
				}
			});
		});

		
	}
});

router.get('/get_info_changes/:user_id', function(req, res){
	UserChangeHistory.
	find({user:req.params.user_id}).
	populate("admin").
	exec(function(error, results){
		if(error) {
			return res.json({error:error});
		}

		var list_results = new Array();
		console.log("results", results);
		for(var i = 0; i < results.length; i++) {
			var result = results[i];
			if(result["change_type"] == "pay_rate") continue;
			var d = result["date_changed"];
			list_results.push({
				date_changed:d.getMonth()+"/"+d.getDay()+"/"+d.getFullYear(),
				change_type:result["change_type"].replace("_", " "),
				changed_by:result["admin"][0]["first_name"]+" "+result["admin"][0]["last_name"],
				old_value:result["old_value"]
			});
		}

		return res.json({total:1, page:1, records:list_results.length, rows:list_results});
	});
});

router.get('/get_pay_rates_history/:user_id', function(req, res){
	UserChangeHistory.
	find({user:req.params.user_id}).
	populate("admin").
	exec(function(error, results){
		if(error) {
			return res.json({error:error});
		}

		var list_results = new Array();
		console.log("results", results);
		for(var i = 0; i < results.length; i++) {
			var result = results[i];
			if(result["change_type"] != "pay_rate") continue;
			var d = result["date_changed"];
			list_results.push({
				date_changed:d.getMonth()+"/"+d.getDay()+"/"+d.getFullYear(),
				changed_by:result["admin"][0]["first_name"]+" "+result["admin"][0]["last_name"],
				old_value:result["old_value"]
			});
		}

		return res.json({total:1, page:1, records:list_results.length, rows:list_results});
	});
});

router.get('/get_open_time_cards', function(req, res){
	time_card.find({user:req.session.user._id}, function(error, db_results){
		if(error) {
			return res.json({error:error});
		}

		var results = new Array();
		for(var i = 0; i < db_results.length; i++) {
			var db_result = db_results[i];
			if(db_result['clock_out_time_date']) continue;
			var d = db_result['clock_in_time_date'];
			var ds = d.getMonth() + "/" + d.getDay() + "/" + d.getYear();
			ds += " " + d.getHours() + ":" + d.getMinutes()
			results.push({location:db_result['location'], clock_in_time_date:ds});
		}

		return res.json({total:1, page:1, records:results.length, rows:results});
	});
});

router.get('/get_shift_history', function(req, res){
	time_card.find({user:req.session.user._id}, function(error, db_results){
		if(error) {
			return res.json({error:error});
		}

		var results = new Array();
		for(var i = 0; i < db_results.length; i++) {
			var db_result = db_results[i];
			if(!db_result['clock_out_time_date'] || !db_result['clock_in_time_date']) continue;
			var d = db_result['clock_in_time_date'];
			var ds = d.getMonth() + "/" + d.getDay() + "/" + d.getYear();
			ds += " " + d.getHours() + ":" + d.getMinutes();

			d = db_result['clock_out_time_date'];
			var ds2 = d.getMonth() + "/" + d.getDay() + "/" + d.getYear();
			ds += " " + d.getHours() + ":" + d.getMinutes()
			
			results.push({location:db_result['location'], clock_in_time_date:ds, clock_out_time_date:ds2});
		}

		return res.send({total:1, page:1, records:results.length, rows:results});
	});
});

router.get('/get_total_hours_by_location', function(req, res){
	time_card.aggregate([{
		$group:{_id:"$location"}
	}], function(err, results){
			console.log("# results", results);

			var cb = function(results, callback){
				var location = results["_id"];
				time_card.find({user:req.session.user._id, location:location}, function(error, db_results){
					if(error) {
						callback(error, null);
					}

					else {
						var c = 0;
						for(var i = 0; i < db_results.length; i++) {
							var db_result = db_results[i];
							if(!db_result["clock_out_time_date"] || !db_result["clock_in_time_date"]) continue;
							var a = Math.abs(db_result["clock_out_time_date"] - db_result["clock_in_time_date"]);
							c += a / 1000 / 60 / 60;
						}

						callback(null, {location:location, total_hours:c});
					}
				})
			};

			Async.map(results, cb, function (error, new_results) {
			  return res.json(new_results);
			});
	});
});


router.get('/:id', function (req, res) {
	var bread_crumbs = pages.MakeBreadCrumb("employee");

	console.log("id", req.params.id);

	User.findOne({_id:req.params.id}, function(error, user){
		console.log("user", user)
		res.render('employee', {page:{title:"Employee "+user.first_name+" "+user.last_name}, list_bread_crumbs:bread_crumbs,
							user:user});
	});
});
module.exports = router;