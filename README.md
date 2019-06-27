Turns Metatrader5 into REST API server. 

Note: make sure you have VSredist: https://www.microsoft.com/en-ie/download/details.aspx?id=48145

time_setup, time is MQL5 datetime, check docs at: https://www.mql5.com/en/docs/basis/types/integer/datetime

check "error" for /trade command on that page: https://www.mql5.com/en/docs/constants/errorswarnings/enum_trade_return_codes

for enums check that page: https://www.mql5.com/en/docs/constants

## how to compile
clone repo to any folder on our PC.

Copy MQL5 folder to Data folder of mt5 (how to get where is Data folder is localted, open mt5, click File->Open Data Folder or just click Ctrl+Shft+D).
In MT5 Navigator (to show Naavigator click Ctrl+N) select Expert Advisors->Advisors. Right click on RestApi, select Modify. Editor will be opened, press F7 to compile. Done.

To compile C++ dll use Visual Studio 2017. Use .sln file to open solution in VS, select Realease x64 and press Ctrl+Shft+b to build dll file. Output is located under /x64 folder.


## Commands

POST /sub - setup web hook. options: callback_url and callback_format (json or x-form)

GET /symbols/{name} - get symbol informaiton including ask/bid prices.

GET /info - get account details, number of  orders, number of positions

GET /positions - returns list of positions

GET /positions/{id} - return position by id

GET /deals?offset={offset}&limit={limit} - returns list of deals/transactions 

GET /deals/{id} - return deal by id

GET /orders - returns list of orders

GET /orders/{id} - return order by id

GET /history - returns list of history orders

GET /history/{id} - return order history by id

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

## Order Cancel
```json
{ "actionType": "POSITION_PARTIAL", "id": 1212121, "volume": 0.1 }
```


# Examples of /trade output
```json
{
    "error": 10018,
    "desription": "TRADE_RETCODE_MARKET_CLOSED",
    "order_id": 0,
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
    "order_id": 405895526,
    "volume": 0.1,
    "price": 1.13047,
    "bid": 1.13038,
    "ask": 1.13047,
    "function": "CRestApi::tradingModule"
}
```
