select restaurant.name, food.name, orders.quantity, orders.date, restaurant.rate from
 restaurant, food, orders where
  food.restaurant_id = restaurant.id and
  orders.food_id = food.id and
  orders.date > '2017-01-01';
  
select restaurant.name, restaurant.rate from
 restaurant, orders, food where
 food.restaurant_id = restaurant.id and
 orders.food_id = food.id and
 orders.date > '2017/01/01';