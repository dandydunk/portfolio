var mongoose = require('mongoose');
var Schema = mongoose.Schema;

var schema = new Schema({
  user: [{ type: Schema.Types.ObjectId, ref: 'user' }],
  system: [{ type: Schema.Types.ObjectId, ref: 'system' }]
});

module.exports = mongoose.model('user_systems_access', schema);