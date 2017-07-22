var mongoose = require('mongoose');
var Schema = mongoose.Schema;

var schema = new Schema({
  user_id: [{ type: Schema.Types.ObjectId, ref: 'user' }],
  name: String
});

module.exports = mongoose.model('system', schema);