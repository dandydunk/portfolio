require 'test_helper'

class UserControllerTest < ActionDispatch::IntegrationTest
  test "should get Index" do
    get user_Index_url
    assert_response :success
  end

end
