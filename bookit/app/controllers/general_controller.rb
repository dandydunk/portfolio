class GeneralController < ApplicationController
  def index
    @inventories = Inventory.joins("INNER JOIN inventory_pictures ON inventory_pictures.inventoriesId = Inventories.id")
                    .select("inventory_pictures.path AS picturePath, inventories.name, inventories.id, inventories.amount_in_stock").distinct.all
  end
end
