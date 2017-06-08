class CreateInventoryPictures < ActiveRecord::Migration[5.0]
  def change
    create_table :inventory_pictures do |t|
      t.integer :inventoriesId
      t.text :path

      t.timestamps
    end
  end
end
