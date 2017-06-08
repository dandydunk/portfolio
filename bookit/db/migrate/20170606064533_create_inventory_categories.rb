class CreateInventoryCategories < ActiveRecord::Migration[5.0]
  def change
    create_table :inventory_categories do |t|
      t.string :name
      t.integer :parentId

      t.timestamps
    end
  end
end
