#!/bin/bash

for i in {1..30}
do
    echo $i
    (echo start bitstamp.net live_orders_btcusd; echo start bitstamp.net live_orders_btcjpy; echo start bitstamp.net live_orders_btccad; sleep 10; echo quit;)| valgrind --show-reachable=yes --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./bin/ticker
done
