class UserController < ApplicationController
  def index
    @user = User.find(params[:id])
    @rentals = Rental.joins(:inventories).all.select("inventories.title")
    render
  end

  def register
    if params.has_key?("email") != true
      render json: {"error" => "the email is missing"}
      return
    end

    if params.has_key?("password") != true
      render json: {"error" => "the password is missing"}
      return
    end

    if params.has_key?("passwordConfirm") != true
      render json: {"error" => "the password confirm is missing"}
      return
    end

    password = params[:password].strip
    email = params[:email].strip
    passwordConfirm = params[:passwordConfirm].strip

    if password.length == 0
      render json: {"error" => "the password is missing"}
      return
    end

    if passwordConfirm.length == 0
      render json: {"error" => "the password confirm is missing"}
      return
    end

    if password != passwordConfirm
      render json: {"error" => "the passwords don't match"}
      return
    end

    user = User.find_by_email(email)
    if user != nil
      render json: {"error" => "that email already exists."}
      return
    end

    user = User.new()
    user.email = params[:email]
    user.password = params[:password]
    user.save()
    render json: {"success" => true}
  end

  def login
    
  end

  def test
    email = params[:email]
    email = email.strip
    render plain: email
  end
end
