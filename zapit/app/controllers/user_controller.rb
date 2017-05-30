class UserController < ApplicationController
  def login
	render template:"user/login", layout:"usersession"
  end
  
  def register
	render template:"user/register", layout:"usersession"
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
