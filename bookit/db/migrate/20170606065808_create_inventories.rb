class CreateInventories < ActiveRecord::Migration[5.0]
  def change
    create_table :inventories do |t|
      t.string :name
      t.text :description
      t.integer :inventory_category_id

      t.timestamps
    end
  end
end
