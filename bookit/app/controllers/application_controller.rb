class ApplicationController < ActionController::Base
  protect_from_forgery with: :exception
  helper_method :isLoggedIn?

  def isLoggedIn?
  	session[:userId]
  end
end
