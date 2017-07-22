var mongoose = require('mongoose');
var Schema = mongoose.Schema;

var schema = new Schema({
  first_name: String,
  last_name: String,
  city: String,
  address: String,
  state: String,
  phone: String,
  email: String,
  password:String,
  picture:String,
  pay_rate:String,
  date_hired:String,
  role:String
});

schema.statics.All = function (cb) {
  return this.model('user').aggregate([{
    $group:{_id:{id:"$_id", first_name:"$first_name", last_name:"$last_name"}}
  }], function(error, results){
    cb(error, results);
  });
};

module.exports = mongoose.model('user', schema);