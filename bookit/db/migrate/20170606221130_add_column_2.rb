class AddColumn2 < ActiveRecord::Migration[5.0]
  def change
    add_column :inventories, :number, :integer
  end
end
