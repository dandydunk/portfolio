class CreateRentals < ActiveRecord::Migration[5.0]
  def change
    create_table :rentals do |t|
      t.integer :inventories_id
      t.integer :users_id
      t.integer :dueDate
      t.timestamps
    end
  end
end
