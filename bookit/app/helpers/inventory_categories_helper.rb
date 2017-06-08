module InventoryCategoriesHelper
  def List(parentId = 0, level = 0, &block)
    if parentId == 0
      cats = InventoryCategory.where("parentId = 0 or parentId is null")
    else
      cats = InventoryCategory.where("parentId = #{parentId}")
    end

    cats.each do |cat|
      yield cat, level
      List(cat.id, level+1, &block)
    end if cats.length > 0
  end
end