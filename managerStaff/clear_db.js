var mongoose = require('mongoose');
mongoose.connect('mongodb://localhost/staff_manager');

var db = mongoose.connection;
db.on('error', console.error.bind(console, 'connection error:'));

console.log("connecting to database...")
db.once('open', function (callback) {
  console.log("connected to database...");

  var User = require("./models/user.js");
  var time_card = require("./models/time_card.js");

  console.log("deleting time cards...");
  time_card.remove({}, function(){
  	console.log("deleting users...");
  	User.remove({}, function(){
  		console.log("finished...");
  		process.exit();
  	})
  })
});