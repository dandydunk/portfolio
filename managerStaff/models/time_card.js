var mongoose = require('mongoose');
var Schema = mongoose.Schema;

var schema = new Schema({
  user: [{ type: Schema.Types.ObjectId, ref: 'user' }],
  clock_in:{
  	date_time: Date,
  	latitude: Number,
  	longitude: Number,
  	city:String,
  	state:String,
  	street_number:String,
  	street_name:String,
  	zip_code:String
  },
  clock_out:{
  	date_time: Date,
  	latitude: Number,
  	longitude: Number,
  	city:String,
  	state:String,
  	street_number:String,
  	street_name:String,
  	zip_code: String
  },
  paid:Boolean
});

module.exports = mongoose.model('time_card', schema);