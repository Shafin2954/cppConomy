Act 1 — Orient the audience (30 sec)
status
consumers
farmers
"We have a small economy: 3 consumers, 2 farmers, 2 laborers, 3 firms, and 8 active markets."
Act 2 — Inspect the rice market (45 sec)
select_market(Rice)
market_details
"Here's the rice market — you can see the aggregate demand curve, supply curve, equilibrium price, and whether there's excess demand. This is Walras' tâtonnement in action."
Act 3 — Zoom into a farmer (45 sec)
select_farmer(Shafin)
farmer_details
farmer_crops
farmer_supply(Rice, 60)
"Shafin has 5 acres at 60% tech. At a market price of 60 Tk/kg, this is how much he'll supply — his supply curve accounts for weather, tax, and land scarcity."
Act 4 — The policy shock (60 sec — the money shot)
farmer_upgrade(0.9)
pass_day
"We upgrade Shafin's technology. Watch what happens across the entire economy when we pass a day — markets re-clear, consumers adjust spending, firms re-optimize."
Act 5 — Show firm efficiency (30 sec)
select_consumer(Rahim)
firm_details
firm_mp
firm_efficiency
"Rahim's garment firm compares MPL/wage vs MPK/rental rate. If these ratios aren't equal, the firm is wasting money — it should hire labor or add capital until the Isoquant is tangent to the Isocost."
Act 6 — Consumer micro (30 sec)
select_consumer(Priom)
consumer_details
consumer_surplus(Rice)
consumer_demand_curve(Rice)