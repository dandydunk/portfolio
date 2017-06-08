class CreateUsers < ActiveRecord::Migration[5.0]
  def change
    create_table :users do |t|
      t.text :email
      t.text :password
      t.datetime :dateCreated
    end
  end
end
