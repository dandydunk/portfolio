var mongoose = require('mongoose');
var Schema = mongoose.Schema;

var schema = new Schema({
  user: [{ type: Schema.Types.ObjectId, ref: 'user' }],
  admin: [{ type: Schema.Types.ObjectId, ref: 'user' }],
  change_type : String,
  old_value : String, 
  date_changed : Date
});

module.exports = mongoose.model('user_change_history', schema);