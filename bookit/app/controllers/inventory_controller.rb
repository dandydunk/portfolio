class InventoryController < ApplicationController
  helper InventoryCategoriesHelper
  def new
    @inventories = Inventory.all
  end

  def getPictures
    o = []
    InventoryPicture.where("inventoriesId = #{params[:id]}").each do |i|
      o += [{"path" => i["path"], "id" => i["id"]}]
    end
    render json:o
  end

  def show
    @inventory = Inventory.joins("INNER JOIN inventory_pictures ON inventory_pictures.inventoriesId = Inventories.id")
                     .where("inventories.id = #{params[:id]}")
        .select("inventory_pictures.path AS picturePath, inventories.name, inventories.description, inventories.id, inventories.amount_in_stock")
         .first
  end

  def reserve
    inventory = Inventory.find_by id: params[:inventoryId]
    if inventory == nil
      render json:{"error" => "invalid inventory id"}
      return
    end

    reserve = Reserve.new
    reserve.users_id = 0
    reserve.inventories_id = params[:inventoryId]
    reserve.dueDate = params[:dueDate]
    reserve.save

    inventory.amount_in_stock = inventory.amount_in_stock - 1
    inventory.save

    render json:{"success":true}
  end
end
