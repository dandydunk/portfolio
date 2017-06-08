Todo:
*controller
**user
***index
***login
***rentals
***index
**rentals
***index

*login
**email txt field
**password text field
**submit button
*register
**email txt field
**password txt field
**confirm password txt field
**submit button

***home page**
**list products
***name
***picture
***button reserve if in stock
***button cancel reserve if session user has reserved

*product information page
**name
**# in stock
**picture
**description
**reviews
***new
***read post
****userId
****will rent again?
****date
****post


*your rentals page
**table
***start date
***end date
***inventory item

*model
**users
***id
***email
***password
**subscriptions
***userId
***subscriptionType
***startDate
***endDate
**scriptionTypes
***id
***name
***price
**inventory
***id
***name
***description
***inventoryCategoryId
**inventoryCategory
***id
**rentals
***id
***userId
***inventoryId
***date
***dueDate