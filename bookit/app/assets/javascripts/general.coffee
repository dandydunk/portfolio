window.Reserve = (id, title)->
	$("#reserveMovieTitle").html(title)
	$("#modalReserve").modal()

$(document).ready(()->
	#$("#datePicker").datepicker()
	$("#buttonReserve").click(()->
		$(this).attr("disabled", "disabled")
		self = $(this)
		data = {}
		ajax = $.post("/inventory/newPost", data, (data)->
			if data.error
				alert(data.error)

			$(self).removeAttr("disabled")
		, "json")
			
		ajax.fail(()->
			alert("ajax failed.")
			$(self).removeAttr("disabled")
		);
	)
	
	$("#buttonRegister").click(()->
		
		$(this).attr("disabled", "disabled")
		self = $(this)
		data = 
			email:$("#emailRegister").val()
			username:$("#usernameRegister").val()
			password:$("#passwordRegister").val()
			passwordConfirm:$("#passwordConfirm").val()
		console.log(data);
		ajax = $.post("/user/register/", data, (d)->
			if d.error
				alert(d.error)

			$(self).removeAttr("disabled")
		, "json")
			
		ajax.fail(()->
			alert("ajax failed.")
			$(self).removeAttr("disabled")
		);
	)
)
