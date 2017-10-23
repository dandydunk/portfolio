function YouTookUser() {
	this.email = "";
	this.userName = "";
	this.password = "";
	this.passwordConfirm = "";
}

YouTookUser.prototype.Login = function() {
	let ajax = $.ajax({ url: '/users/login/' });
	ajax.done(function (data) {
		alert(data);
	});
	ajax.fail(function () {
		alert("failed");
	});
}