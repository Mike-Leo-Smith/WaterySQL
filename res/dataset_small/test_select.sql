select restaurant.name, food.name, orders.quantity, orders.date from
 restaurant, food, orders where
  food.restaurant_id = restaurant.id and
  orders.food_id = food.id and
  orders.date > '2017-01-01';