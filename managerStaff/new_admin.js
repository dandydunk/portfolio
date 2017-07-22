var args = process.argv;
if(!args[2] || !args[2].length) {
	console.log("password is missing.");
	process.exit();
}

var Pass_Word = args[2];

var mongoose = require('mongoose');
mongoose.connect('mongodb://localhost/staff_manager');

var db = mongoose.connection;
db.on('error', console.error.bind(console, 'connection error:'));

console.log("connecting to database...")
db.once('open', function (callback) {
  console.log("connected to database...");

  var User = require("./models/user.js");
  console.log("making admin user with password:", Pass_Word);
  User.findOne({password:Pass_Word}, function(error, user){
  	if(error) {
  		console.log("there was a database error, finding your password", error);
  	}

  	else {
  		if(user) {
  			console.log("your password exists; choose another one.");
  			process.exit();
  		}

		var admin = new User({password:Pass_Word, first_name:"Change", last_name:"Me", role:"admin"})

		admin.save(function(error, user_){
		  	if(error) {
		  		console.log("database error saving the admin", error);
		  	}

		  	else {
		  		if(!user_) {
		  			console.log("couldn't create the user...");
		  		}
		  	}

		  	process.exit();
	 	});
  	}
  });
  

});