Turns Metatrader5 into REST API server. 


## Commands

POST /sub - setup web hook. options: callback_url and callback_format (json or x-form)

GET /info - get account details, number of  orders, number of positions

GET /positions - returns list of positions

GET /transactions - returns list of deals/transactions

GET /balance - returns balance, margin, free margin

GET /orders - returns list of orders

POST /trade - open position, details in POST body

# Example of POST body for trade command

## Open Buy
```json
{ 
  "symbol":"EURUSD", 
  "actionType": "ORDER_TYPE_BUY", 
  "volume": 0.1, 
  "stoploss": 1.3455, 
  "takeprofit": 1.33333, 
  "comment": "test buy"
}
```

## Open Sell
```json
{ 
  "symbol":"EURUSD", 
  "actionType": "ORDER_TYPE_SELL", 
  "volume": 0.1, 
  "stoploss": 1.3455, 
  "takeprofit": 1.33333, 
  "comment": "test buy"
}
```

## Open Buy Limit
```json
{ 
  "symbol":"EURUSD", 
  "actionType": "ORDER_TYPE_BUY_LIMIT", 
  "price": 1.4444, 
  "volume": 0.1, 
  "stoploss": 1.3455, 
  "takeprofit": 1.33333, 
  "comment": "test buy limit"
}
```

## Open Sell Limit
```json
{ 
  "symbol":"EURUSD", 
  "actionType": "ORDER_TYPE_SELL_LIMIT", 
  "price": 1.4444,
  "volume": 0.1, 
  "stoploss": 1.3455, 
  "takeprofit": 1.33333, 
  "comment": "test sell limit"
}
```

## Open Buy Stop
```json
{ 
  "symbol":"EURUSD", 
  "actionType": 
  "ORDER_TYPE_BUY_STOP", 
  "price": 1.4444, 
  "volume": 0.1, 
  "stoploss": 1.3455, 
  "takeprofit": 1.33333, 
  "comment": "test buy stop"
}
```

## Open Sell Stop
```json
{ 
  "symbol":"EURUSD", 
  "actionType": "ORDER_TYPE_SELL_STOP", 
  "price": 1.4444, 
  "volume": 0.1, 
  "stoploss": 1.3455, 
  "takeprofit": 1.33333, 
  "comment": "test sell stop"
}  
```

## Position Close by ID
```json
{ "actionType": "POSITION_CLOSE_ID", "id": 1212121 }
```

## Order Cancel
```json
{ "actionType": "ORDER_CANCEL", "id": 1212121 }
```

# Examples of /trade output
```json
{
    "error": 10018,
    "desription": "TRADE_RETCODE_MARKET_CLOSED",
    "order": 0,
    "volume": 0,
    "price": 0,
    "bid": 0,
    "ask": 0,
    "function": "CRestApi::tradingModule"
}
```
```json
{
    "error": 10009,
    "desription": "TRADE_RETCODE_DONE",
    "order": 405895526,
    "volume": 0.1,
    "price": 1.13047,
    "bid": 1.13038,
    "ask": 1.13047,
    "function": "CRestApi::tradingModule"
}
```
