import * as $ from 'jquery';

class YouTookUser {
	userName : string;
	password : string;
	passwordConfirm : string;
	email : string;
	
	Login() {
		let ajax = $.ajax({url:'/users/login/'});
				
		ajax.done(function(data : any){
			alert(data);
		});
		
		ajax.fail(function(){
			alert("failed");
		});
	}
	
	Register() {
		let ajax = $.ajax({url:'/users/register/'});
				
		ajax.done(function(data : any){
			alert(data);
		});
		
		ajax.fail(function(){
			alert("failed");
		});
	}
}