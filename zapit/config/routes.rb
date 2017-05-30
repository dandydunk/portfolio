Rails.application.routes.draw do
  get 'user/register'

  get 'user/login'
  get 'user/register'
  get 'user/logout'
  get 'user/profile'
  get 'user/editprofile'

  get 'home/index'
  root 'home#index'
end
