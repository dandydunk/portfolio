var express = require('express');
var router = express.Router();
var pages = require('../pages.js');
var user_systems_access = require("../models/user_systems_access.js");
var time_card = require("../models/time_card.js");
var system = require("../models/system.js");
var User = require("../models/user.js");
var UserChangeHistory = require("../models/user_change_history.js")
var Async = require("async");
var url = require('url') ;

router.use(function (req, res, next) {
  if(req.session.user.role != "admin") {
  	req.session.error = "access denied";
  	return res.redirect("/error");
  }

  next();
});

router.get('/', function (req, res) {
  	var bread_crumbs = pages.MakeBreadCrumb("staff");

  	User.find({}, function(err, results){
  		if(err) {
  			console.log("database error");
  			req.session.error = "database error";
  			return res.redirect("/error");
  		}

  		else {

  			res.render('staff', {page:{title:"Staff"}, list_bread_crumbs:bread_crumbs, list_staff:results});
  		}
  	});
});

router.get('/reports', function (req, res) {
  	var bread_crumbs = pages.MakeBreadCrumb("staff");

  	User.find({}, function(err, results){
  		if(err) {
  			console.log("database error");
  			req.session.error = "database error";
  			return res.redirect("/error");
  		}

  		else {
  			console.log("users", results);
  			res.render('reports', {page:{title:"Staff Reports"}, list_bread_crumbs:bread_crumbs, list_staff:results});
  		}
  	});
});

router.get('/reports_single', function(req, res){
	var query = url.parse(req.url,true).query;

	if(!query.start_date || !query.start_date.length) {
		return res.json({error:"the start date is missing."});
	}

	if(!query.end_date || !query.end_date.length) {
		return res.json({error:"the end date is missing."});
	}

	if(new Date(query.end_date) < new Date(query.start_date)) {
		return res.json({error:"the start date must be before the end date."});
	}


	var matchs = {};
	if(query.find_user && query.find_user.length) {
		console.log("finding user", query.find_user);
		matchs.user = query.find_user;
	}

	if((query.start_date && query.start_date.length) &&
		(query.end_date && query.end_date.length)) {
		matchs["clock_in.date_time"] = {$gte:query.start_date};
		matchs["clock_out.date_time"] = {$lte:query.end_date};
	}

	else if(query.start_date && query.start_date.length) {
		matchs["clock_in.date_time"] = {$gte:query.start_date};
	}

	console.log("match", matchs);

	time_card.find(matchs).populate('user', 'first_name last_name pay_rate').exec(function(error, db_results){
		if(error) {
			return res.json({error:error});
		}

		var results = new Array();
		var o_users = new Array();
		console.log("time cards", db_results);
		for(var i = 0; i < db_results.length; i++) {
			var db_result = db_results[i];
			if(!db_result['user'][0]) continue;

			if(!db_result.clock_out.date_time || !db_result.clock_in.date_time) continue;
			var tc_date = new Date(db_result.clock_in.date_time);
			
			var user_id = db_result['user'][0]['_id'];
			if(!o_users[user_id]) {
				o_users[user_id] = {employee:db_result['user'][0]["first_name"]+" "+db_result['user'][0]["last_name"],
									wage:db_result['user'][0]['pay_rate'],
									total:0};
			}

			var shift_length = (db_result.clock_out.date_time - db_result.clock_in.date_time) / 1000 / 60 / 60;
			var tc_start_date = (tc_date.getMonth()+1)+"/"+tc_date.getDate()+"/"+tc_date.getFullYear();
			if(!o_users[user_id][tc_start_date]) {
				o_users[user_id][tc_start_date] = 0;
			}
			o_users[user_id][tc_start_date] += shift_length;
			o_users[user_id]['total'] += parseFloat(db_result['user'][0]['pay_rate']) * shift_length;
		}

		for(var user in o_users) {
			results.push(o_users[user]);
		}

		var start = new Date(query.start_date);
		var end = new Date(query.end_date);
		var model_days = [];
		var model_names = [];
		model_days.push({name:"employee"});
		model_names.push("Employee");
		for (var d = start; d <= end; d.setDate(d.getDate() + 1)) {
			var this_d = new Date(d);
			var n = (this_d.getMonth()+1)+"/"+this_d.getDate()+"/"+this_d.getFullYear();
		    model_days.push({name:n});
		    model_names.push(n);
		}

		model_days.push({name:"wage"});
		model_names.push("Wage");

		model_days.push({name:"total"});
		model_names.push("Total");

		console.log("results", results);

		return res.json({model:model_days, model_names:model_names, data:results});
		//return res.json({total:1, page:1, records:results.length, rows:results});
	});
});


router.get('/get_open_time_cards', function(req, res){
	var query = url.parse(req.url,true).query;

	var matchs = {};
	if(query.find_user && query.find_user.length) {
		console.log("finding user", query.find_user);
		matchs.user = query.find_user;
	}

	if((query.start_date && query.start_date.length) &&
		(query.end_date && query.end_date.length)) {
		matchs["clock_in.date_time"] = {$gte:query.start_date};
		matchs["clock_out.date_time"] = {$lte:query.end_date};
	}

	else if(query.start_date && query.start_date.length) {
		matchs["clock_in.date_time"] = {$gte:query.start_date};
	}

	console.log("match", matchs);
	time_card.find(matchs).populate('user').exec(function(error, db_results){
		if(error) {
			return res.json({error:error});
		}

		console.log("open time cards", db_results);
		var results = new Array();
		for(var i = 0; i < db_results.length; i++) {
			var db_result = db_results[i];
			if(db_result.clock_out.date_time) continue;
			if(!db_result['user'] || !db_result['user'][0]) continue;
			var d = db_result.clock_in.date_time;
			var ds = d.getMonth() + "/" + d.getDay() + "/" + d.getYear();
			ds += " " + d.getHours() + ":" + d.getMinutes();
			results.push({location:db_result.clock_in.street_name+" "+
									db_result.clock_in.street_number+", "+
									db_result.clock_in.city+", "+
									db_result.clock_in.state, 
						clock_in_time_date:db_result.clock_in.date_time,
						first_name:db_result['user'][0]['first_name'],
						last_name:db_result['user'][0]['last_name'],
						user_id:db_result['user'][0]['_id']});
		}

		results.sort(function(a, b){
			if(a[query.sidx] == b[query.sidx]) {
				return 0;
			}
			if(a[query.sidx] < b[query.sidx]) {
				return (query.sord == "asc") ? -1:1;
			}

			return (query.sord == "asc") ? 1:-1;
		});

		return res.json({total:1, page:1, records:results.length, rows:results});
	});
});

router.get('/get_shift_history', function(req, res){
	var query = url.parse(req.url,true).query;

	var matchs = {};
	if(query.find_user && query.find_user.length) {
		console.log("finding user", query.find_user);
		matchs.user = query.find_user;
	}

	if((query.start_date && query.start_date.length) &&
		(query.end_date && query.end_date.length)) {
		matchs["clock_in.date_time"] = {$gte:query.start_date};
		matchs["clock_out.date_time"] = {$lte:query.end_date};
	}

	else if(query.start_date && query.start_date.length) {
		matchs["clock_in.date_time"] = {$gte:query.start_date};
	}

	console.log("match", matchs);
	time_card.find(matchs).populate('user', 'first_name last_name').exec(function(error, db_results){
		if(error) {
			return res.json({error:error});
		}

		var results = new Array();
		for(var i = 0; i < db_results.length; i++) {
			var db_result = db_results[i];
			if(!db_result['user'][0]) continue;

			if(!query.show_paid)
				if(db_result.paid) continue;

			console.log("result", db_result);

			if(!db_result.clock_out.date_time || !db_result.clock_in.date_time) continue;
			
			results.push({
				id:db_result._id,
				clock_in_location:db_result.clock_in.street_name+" "+
									db_result.clock_in.street_number+", "+
									db_result.clock_in.city+", "+
									db_result.clock_in.state,
				clock_in_time_date:db_result.clock_in.date_time,
				first_name:db_result['user'][0]['first_name'],
				last_name:db_result['user'][0]['last_name'],
				user_id:db_result['user'][0]['_id']});

			results.push({
				id:db_result._id,
				clock_out_location:db_result.clock_out.street_name+" "+
									db_result.clock_out.street_number+", "+
									db_result.clock_out.city+", "+
									db_result.clock_out.state,
				clock_out_time_date:db_result.clock_out.date_time,
				first_name:db_result['user'][0]['first_name'],
				last_name:db_result['user'][0]['last_name'],
				user_id:db_result['user'][0]['_id']});
		}

		if(query.sidx && query.sidx.length) {
			results.sort(function(a, b){
				if(a[query.sidx] == b[query.sidx]) {
					return 0;
				}
				if(a[query.sidx] < b[query.sidx]) {
					return (query.sord == "asc") ? -1:1;
				}

				return (query.sord == "asc") ? 1:-1;
			});
		}
		return res.json({total:1, page:1, records:results.length, rows:results});
	});
});

router.get('/get_total_hours_by_location', function(req, res){
	var query = url.parse(req.url,true).query;

	var match_user = {};
	var ObjectId = require('mongoose').Types.ObjectId; 
	if(query.find_user) {
		console.log("finding user", query.find_user);
		match_user = {user:new ObjectId(query.find_user)};
	}

	var match_date = {};
	if((query.start_date && query.start_date.length) &&
		(query.end_date && query.end_date.length)) {
		matchs["clock_in.date_time"] = {$gte:query.start_date};
		matchs["clock_out.date_time"] = {$lte:query.end_date};
	}

	else if(query.start_date && query.start_date.length) {
		matchs["clock_in.date_time"] = {$gte:query.start_date};
	}
	
	var sort = {};
	if(query.sidx && query.sidx.length) {
		sort[query.sidx] = (query.sord == "desc") ? -1:1;
	}
	
	console.log("sorting by", sort);
	time_card.aggregate([
		{
			$match:match_user
		},
		{
			$match:match_date
		},
		{
		$group:{
				_id:{user:"$user", location:{city:"$clock_in.city", state:"$clock_in.state"}},
				total_hours:{$sum:{$subtract:["$clock_out.date_time", "$clock_in.date_time"]}}
			},
		}], function(err, results){
				if(err) {
					return res.json({error:err});
				}

				User.populate(results, {path:"_id.user", 
										model:"user"
										}, function(err, results){
					if(err) {
						return res.json({error:err});
					}
					
					var list_data = new Array();
					for(var i = 0; i < results.length; i++) {
						var result = results[i];
						console.log("total", result);
						if(!result["_id"]["user"]) continue;
						var user = result["_id"]["user"][0];
						if(!user) continue;
						list_data.push({user_id:user._id,
										first_name:user.first_name,
										last_name:user.last_name,
										location:result["_id"]["location"].city+", "+result["_id"]["location"].state,
										total_hours:Math.round((result["total_hours"] / 1000 / 60 / 60) * 100) / 100});
					}

					list_data.sort(function(a, b){
						if(a[query.sidx] == b[query.sidx]) {
							return 0;
						}
						if(a[query.sidx] < b[query.sidx]) {
							return (query.sord == "asc") ? -1:1;
						}

						return (query.sord == "asc") ? 1:-1;
					});

					return res.json(list_data);
				});
			});
});

router.get('/get_staff_time_cards', function (req, res) {
	var a = new Array();
  	time_card.find({}, function(err, results){
  		if(err) {
  			console.log("error gettin staff time cards", err);
  		}

  		else {
  			var time_cards = new Array();
  			for(var i = 0; i < results.length; i++) {
  				var result = results[i];
  				var user = result['user'][0];
  				if(!user) continue;

  				//console.log(result);
  				var tc = {first_name:result['user'][0]['first_name'],
  						  last_name:result['user'][0]['last_name'],
  						  location:"to be added",
  						  total_hours:0}
  				time_cards.push(tc);
  			}

  			res.json({total:1, page:1, records:results.length, rows:time_cards});
  		}
  	}).populate('user');

  	time_card.aggregate()
});


module.exports = router;