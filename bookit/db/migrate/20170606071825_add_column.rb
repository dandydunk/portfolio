class AddColumn < ActiveRecord::Migration[5.0]
  def change
    add_column :inventories, :amount_in_stock, :integer
  end
end
