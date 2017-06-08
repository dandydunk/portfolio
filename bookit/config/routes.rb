Rails.application.routes.draw do
  get 'user/index/:id', to: 'user#index'
  get 'user/register'
  get 'user/test'
  post 'user/registerPost', to: 'user#registerPost'
  get 'general/index'
  get 'inventory/new'
  get 'inventory/show/:id', to: 'inventory#show'
  get 'inventory/getPictures/:id', to: 'inventory#getPictures'
  root 'general#index'
  # For details on the DSL available within this file, see http://guides.rubyonrails.org/routing.html
end
