create index food(name);

select restaurant.name, restaurant.phone, food.price from
 restaurant, food where
  restaurant.id = food.restaurant_id and food.name = "hot pot";
