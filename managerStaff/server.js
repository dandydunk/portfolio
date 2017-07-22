var express = require('express');
var app = express();

app.set('views', './views/');
app.set('view engine', 'jade');

var body_parser = require('body-parser')
app.use(body_parser.urlencoded({ extended: false }))

app.use(express.static('public'));

app.locals.basedir = app.get('views');



/* */
var multer  = require('multer');
app.use(multer({ 
	dest: './public/uploads/',
	rename: function (fieldname, filename, req) {
		return filename+Date.now();
	},
	onFileUploadStart: function (file) {
		console.log(file.originalname + ' is starting ...')
	},
	onFileUploadComplete: function (file) {
		console.log(file.fieldname + ' uploaded to  ' + file.path)
	}
}));



/*  */
var mongoose = require('mongoose');
mongoose.connect('mongodb://localhost/staff_manager');

var db = mongoose.connection;
db.on('error', console.error.bind(console, 'connection error:'));

console.log("connecting to database...")
db.once('open', function (callback) {
  console.log("connected to database...");

  var user = require("./models/user.js");
  var time_card = require("./models/time_card.js");


  LoadSessions();
  var routes = require('./routes.js');
  routes(app);

  StartServer();
});

/* */
function LoadSessions() {
	var session = require('express-session');
	var MongoStore = require('connect-mongo')(session);
	 
	app.use(session({
	    secret: 'foowwvfvv',
	    store: new MongoStore({ mongooseConnection: mongoose.connection })
	}));

	app.use(function(req, response, next){
		console.log("the path", req.path);
		//console.log("user", req.session.user);
		response.locals.request = req;
		if(req.path != "/account/login" && !req.session.user) {
			response.redirect("/account/login");
			return;
		}

		next();
	});
}

function StartServer() {
	var server = app.listen(300, function () {

	  var host = server.address().address;
	  var port = server.address().port;

	  console.log('listening at http://%s:%s', host, port);

	});
}