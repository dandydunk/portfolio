var pages = require('./pages.js');
var user_systems_access = require("./models/user_systems_access.js");
var time_card = require("./models/time_card.js");
var system = require("./models/system.js");
var User = require("./models/user.js");
var UserChangeHistory = require("./models/user_change_history.js")
var Async = require("async");
var Account = require("./controllers/account.js");
var Employee = require("./controllers/employee.js");
var Staff = require("./controllers/staff.js");

module.exports = function(app) {
	
	app.get('/', function (req, res) {
		if(req.session.user.role != "admin") {
			req.session.error = "access denied";
			return res.redirect("/error");
		}

		var bread_crumbs = pages.MakeBreadCrumb("/");

		User.All(function(error, users){
			if(error) {
				console.log("error", error);
				req.session.error = "database error getting the users..";
				return res.redirect("/error");
			}
			else {
				console.log("all users", users);
				return res.render('index', {page:{title:"Dash Board"}, list_bread_crumbs:bread_crumbs, users:users});
			}
		});
	});

	app.use("/staff", Staff);
	app.use("/account", Account);
	app.use("/employee", Employee);
	app.get("/error", function(req, res){
		if(!req.session.error) {
			return res.send("no error has been set!");
		}

		else {
			var err = req.session.error;
			req.session.error = null;
			return res.send(err);
		}
	});	
}