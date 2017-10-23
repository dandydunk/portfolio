class UserController < ApplicationController
  def login
	render template:"user/login", layout:"usersession"
  end
  
  def register
	render template:"user/register", layout:"usersession"
  end
  
  def save
	params.each do |key, value|
		params[key] = params[key].strip if not params[key].empty?
	end
	
	user = User.new
	user.email = params[:email]
	if user.email == nil or user.email.empty? then
		render json:{"error" => "the email is missing"}
		return
	end
	
	userTemp = User.find_by email:user.email
	if not userTemp == nil then
		render json:{"error" => "the email already exists"}
		return
	end
	
	user.password = params[:password]
	if user.password == nil or user.password.empty? then
		render json:{"error" => "the password is missing"}
		return
	end
	
	user.username = params[:username]
	if user.username == nil or user.username.empty? then
		render json:{"error" => "the user name is missing"}
		return
	end
	
	userTemp = User.find_by username:user.username
	if not userTemp == nil then
		render json:{"error" => "the user name already exists"}
		return
	end
	
	user.save
	
	render json:{"success":true}
  end
  
  def logout
	render template:"user/logout", layout:"usersession"
  end
  
  def profile
	render template:"user/profile", layout:"usersession"
  end
  
  def editprofile
	render template:"user/editprofile", layout:"usersession"
  end
end
