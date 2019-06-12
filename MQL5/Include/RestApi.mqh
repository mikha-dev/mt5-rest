#property version   "1.00"

#include <Json.mqh>
#include <Trade/Trade.mqh>
#include <Strings/String.mqh>

#import "mt5-rest.dll"
   int Init( const uchar &url[], int port, int command_wait_timeout, const uchar&path[], const uchar &url[]);
   int GetCommand(uchar &data[]);
   int SetCallback(const uchar &url[], const uchar &format[]);
   int SetCommandResponse(const uchar &command[], const uchar &response[]);
   int RaiseEvent(const uchar &data[]);
   int SetAuthToken(const uchar &token[]);
   void Deinit();
#import

class CRestApi
  {

public:
                     CRestApi(void);
                    ~CRestApi(void);
   static string    GetErrorID(int error);
   static string    GetRetcodeID(int retcode);
   static int       Pub(string message) {
      uchar d[];
      StringToCharArray(message,d);
      
      return RaiseEvent( d );   
   };
   static int       SetCallback(string url, string format) {
      uchar u[], f[];
      StringToCharArray(url,u);
      StringToCharArray(format,f);
      
      return SetCallback( u, f );   
   };   
                    
   //---
   bool              Init(string _host, int _port, int commandWaitTimeout, string _url_swagger);
   bool              SetAuth(string token);
   void              Deinit(void);
   void              Processing(void);
   void              OnTradeTransaction(const MqlTradeTransaction &trans,
                        const MqlTradeRequest &request,
                        const MqlTradeResult &result);
   
private:
   string notImpemented(string command);
   string getAccountInfo();
   string getPositions();
   string getBalanceInfo();
   string getOrders();
   string getTransactions(CJAVal &dataObject);
   string tradingModule(CJAVal &dataObject);
   string orderDoneOrError(bool error, string funcName, CTrade &trade);
   string actionDoneOrError(int lastError, string funcName);
   string fromDateTime(datetime param);
private:
   bool debug;
};

CRestApi::CRestApi(void) {
   debug = true;
}
//+------------------------------------------------------------------+
//| Destructor                                                       |
//+------------------------------------------------------------------+
CRestApi::~CRestApi(void)
  {
  }
//+------------------------------------------------------------------+
//| Method Init.                                                     |
//+------------------------------------------------------------------+
bool CRestApi::Init(string _host, int _port, int _commandWaitTimeout, string _url_swagger) {
   uchar __host[], _path[], _url[];
   StringToCharArray(_host, __host);
   StringToCharArray(_url_swagger, _url);
   StringToCharArray(TerminalInfoString(TERMINAL_DATA_PATH) + "\\MQL5\\Libraries\\", _path);
   
   Init(__host,_port, _commandWaitTimeout, _path, _url);
   
   if(debug) Print("stated");
   
   ChartRedraw();
   return(true);   
}

bool CRestApi::SetAuth(string token) {
   uchar d[];
   StringToCharArray(token,d);
   int ret = SetAuthToken( d );
   
   return true;
}
//+------------------------------------------------------------------+
//| Method Deinit.                                                   |
//+------------------------------------------------------------------+
void CRestApi::Deinit(void) {
   Deinit();
}

void CRestApi::Processing(void) {
  
  uchar _command[8048];
  uchar _response[];
  string command = {0}, response = {0};
  
   int r = 0;
   r = GetCommand( _command );
   
   if(r == 1) {
      command = CharArrayToString( _command );
      Print("command: " + command);
      
      CJAVal jCommand;
      
      jCommand.Deserialize( command );
      
      string action = jCommand["command"].ToStr();
      
      if(action == "inited") {
         Print("Listening on: " + jCommand["options"].ToStr());
         Comment("Open " + jCommand["options"].ToStr() + " for docs");
         
         return;
      }      
      
      if(action == "failed") {
         Print("Failed to start, error: " + jCommand["options"].ToStr());
         Comment("Failed to start server: " + jCommand["options"].ToStr());
         
         return;
      }            
      
      if(action == "info") {
         response = getAccountInfo();
      }
      
      if(action == "orders") {
         response = getOrders();
      }      
      
      if(action == "balance") {
         response = getBalanceInfo();
      }            
      
      if(action == "positions") {
         response = getPositions();
      }                  
      
      if(action == "transactions") {
         response = getTransactions(jCommand);
      }                        
      
      if(action == "trade") {
         response = tradingModule(jCommand);
      }                        
      
      if(StringLen(response) < 1) {
         response = notImpemented(action);
//         Pub( "{\"test\":1}" );
      } 
      
      StringToCharArray(response,_response);
      SetCommandResponse( _command, _response );
   }
}

string CRestApi::notImpemented(string command) {
   CJAVal info;
   
   info["error"] = "Not implemented";
   info["command"] = command;
   
   string t = info.Serialize();
   
   if(debug) Print(t);
   
   return t;
}

string CRestApi::getAccountInfo() {  
   CJAVal info;
   
   info["broker"] = AccountInfoString(ACCOUNT_COMPANY);
   info["currency"] = AccountInfoString(ACCOUNT_CURRENCY);
   info["server"] = AccountInfoString(ACCOUNT_SERVER); 
   //info["trading_allowed"] = TerminalInfoInteger(TERMINAL_TRADE_ALLOWED);
   //info["bot_trading"] = AccountInfoInteger(ACCOUNT_TRADE_EXPERT);   
   info["balance"] = AccountInfoDouble(ACCOUNT_BALANCE);
   info["equity"] = AccountInfoDouble(ACCOUNT_EQUITY);
   info["margin"] = AccountInfoDouble(ACCOUNT_MARGIN);
   info["margin_free"] = AccountInfoDouble(ACCOUNT_MARGIN_FREE);
   info["leverage"] = AccountInfoInteger(ACCOUNT_LEVERAGE);
   info["margin_level"] = AccountInfoDouble(ACCOUNT_MARGIN_LEVEL);
   info["positions_total"] = PositionsTotal();
   if (HistorySelect(0,TimeCurrent())) {
      info["orders_total"] = OrdersTotal();   
   } else {
      info["orders_total"] = 0;
   }
      
   string t=info.Serialize();
   if(debug) Print(t);
   return t;
}

//+------------------------------------------------------------------+
//| Balance information                                              |
//+------------------------------------------------------------------+
string CRestApi::getBalanceInfo() {  
   CJAVal info;
   info["balance"] = AccountInfoDouble(ACCOUNT_BALANCE);
   info["equity"] = AccountInfoDouble(ACCOUNT_EQUITY);
   info["margin"] = AccountInfoDouble(ACCOUNT_MARGIN);
   info["margin_free"] = AccountInfoDouble(ACCOUNT_MARGIN_FREE);
   
   info["positions_total"] = PositionsTotal();
   if (HistorySelect(0,TimeCurrent())) {
      info["deal_total"] = HistoryDealsTotal();
      info["orders_total"] = OrdersTotal();   
   } else {
      info["deal_total"] = 0;
      info["orders_total"] = 0;
   }
   
   string t = info.Serialize();
   if(debug) Print(t);   
   
   return t;
}

//+------------------------------------------------------------------+
//| Fetch positions information                               |
//+------------------------------------------------------------------+
string CRestApi::getPositions() {
      CPositionInfo myposition;
      CJAVal data, position;
   
      // Get positions  
      int positionsTotal=PositionsTotal();
      // Create empty array if no positions
      if(!positionsTotal) data.Add(position);
      // Go through positions in a loop
      for(int i=0;i<positionsTotal;i++)
        {
         ResetLastError();
         
         if(myposition.Select(PositionGetSymbol(i)))
            {
              position["id"]=PositionGetInteger(POSITION_IDENTIFIER);
              position["magic"]=PositionGetInteger(POSITION_MAGIC);
              position["symbol"]=PositionGetString(POSITION_SYMBOL);
              position["type"]=EnumToString(ENUM_POSITION_TYPE(PositionGetInteger(POSITION_TYPE)));
              position["time_setup"] = fromDateTime(PositionGetInteger(POSITION_TIME));
              position["open"]=PositionGetDouble(POSITION_PRICE_OPEN);
              position["stoploss"]=PositionGetDouble(POSITION_SL);
              position["takeprofit"]=PositionGetDouble(POSITION_TP);
              position["volume"]=PositionGetDouble(POSITION_VOLUME);
            
              data.Add(position);
            }
          // Error handling    
          else actionDoneOrError(ERR_TRADE_POSITION_NOT_FOUND, __FUNCTION__);
         }
      string t=data.Serialize();
      if(debug) Print(t);
      
      return t;
}

//+------------------------------------------------------------------+
//| Fetch transactions information                               |
//+------------------------------------------------------------------+
string CRestApi::getTransactions(CJAVal &dataObject) {
      ResetLastError();
      
      ulong ticket;
      CJAVal data, deal;
      
      int offset = (int)dataObject["offset"].ToInt();
      int limit = (int)dataObject["limit"].ToInt();
      
      if (HistorySelect(0,TimeCurrent()))   
         {    
            int dealsTotal = HistoryDealsTotal();
            
            if(limit == 0 )
               limit = dealsTotal-1;
            else
               limit = limit-1;
               
            if(offset > dealsTotal-1)
               offset = dealsTotal-1;
               
            if(!dealsTotal) {
              data.Add(deal);
            }
            
            for(int i = MathMin(offset + limit, dealsTotal-1);i>=offset;i--) {
   
               if((ticket = HistoryDealGetTicket(i))>0) {   
                  deal["id"] = (string)ticket;
                  deal["price"] = HistoryDealGetDouble(ticket,DEAL_PRICE);
                  deal["commission"] = HistoryDealGetDouble(ticket,DEAL_COMMISSION);
                  deal["time"]= fromDateTime(HistoryDealGetInteger(ticket,DEAL_TIME));
                  deal["symbol"]=HistoryDealGetString(ticket,DEAL_SYMBOL);
                  deal["type"]=EnumToString(ENUM_DEAL_TYPE(HistoryDealGetInteger(ticket,DEAL_TYPE)));                  
                  deal["profit"] = HistoryDealGetDouble(ticket,DEAL_PROFIT);       
                  deal["volume"] = HistoryDealGetDouble(ticket,DEAL_VOLUME);                         
                  
                  data.Add(deal);
                } 
             }
         }
         
       string t=data.Serialize();
       if(debug) {Print(t);}
       
       return t;
}

//+------------------------------------------------------------------+
//| Fetch orders information                               |
//+------------------------------------------------------------------+
string CRestApi::getOrders() {
      ResetLastError();
      
      COrderInfo myorder;
      CJAVal data, order;
      
      // Get orders
      if (HistorySelect(0,TimeCurrent()))   
         {    
            int ordersTotal = OrdersTotal();
            // Create empty array if no orders
            if(!ordersTotal) {
              data.Add(order);
            }
            
            for(int i=0;i<ordersTotal;i++)
             {
   
               if (myorder.Select(OrderGetTicket(i))) 
                {   
                  order["id"]=(string) myorder.Ticket();
                  order["magic"]=OrderGetInteger(ORDER_MAGIC); 
                  order["symbol"]=OrderGetString(ORDER_SYMBOL);
                  order["type"]=EnumToString(ENUM_ORDER_TYPE(OrderGetInteger(ORDER_TYPE)));
                  order["time_setup"]=fromDateTime(OrderGetInteger(ORDER_TIME_SETUP));
                  order["open"]=OrderGetDouble(ORDER_PRICE_OPEN);
                  order["stoploss"]=OrderGetDouble(ORDER_SL);
                  order["takeprofit"]=OrderGetDouble(ORDER_TP);
                  order["volume"]=OrderGetDouble(ORDER_VOLUME_INITIAL);
                  
                  data.Add(order);
                } 
             }
         }
         
       string t=data.Serialize();
       if(debug) {Print(t);}
       
       return t;
 }

//+------------------------------------------------------------------+
//| Trading module                                                   |
//+------------------------------------------------------------------+
string CRestApi::tradingModule(CJAVal &dataObject) {
      ResetLastError();
      CTrade trade;
      
      string   actionType = dataObject["actionType"].ToStr();
      string   symbol=dataObject["symbol"].ToStr();
      // Check if symbol the same
      if(!(symbol==_Symbol)) actionDoneOrError(ERR_MARKET_UNKNOWN_SYMBOL, __FUNCTION__);
      
      int      idNimber=(int)dataObject["id"].ToInt();
      double   volume=dataObject["volume"].ToDbl();
      double   SL=dataObject["stoploss"].ToDbl();
      double   TP=dataObject["takeprofit"].ToDbl();
      double   price=NormalizeDouble(dataObject["price"].ToDbl(),_Digits);
      datetime expiration=TimeTradeServer()+PeriodSeconds(PERIOD_D1);
      double   deviation=dataObject["deviation"].ToDbl();  
      string   comment=dataObject["comment"].ToStr();
      
      // Market orders
      if(actionType=="ORDER_TYPE_BUY" || actionType=="ORDER_TYPE_SELL")
         {  
            ENUM_ORDER_TYPE orderType;
            
            if( actionType=="ORDER_TYPE_BUY" )
               orderType = ORDER_TYPE_BUY;
            else
              orderType = ORDER_TYPE_SELL;
               
            price = SymbolInfoDouble(symbol,SYMBOL_ASK);                                        
            if(orderType==ORDER_TYPE_SELL) price=SymbolInfoDouble(symbol,SYMBOL_BID);
            
            if(trade.PositionOpen(symbol,orderType,volume,price,SL,TP,comment)) {
               return orderDoneOrError(false, __FUNCTION__, trade);
             }
           }
      
      // Pending orders
      else if(actionType=="ORDER_TYPE_BUY_LIMIT" || actionType=="ORDER_TYPE_SELL_LIMIT" || actionType=="ORDER_TYPE_BUY_STOP" || actionType=="ORDER_TYPE_SELL_STOP")
         {  
            if(actionType=="ORDER_TYPE_BUY_LIMIT") 
               {
                  if(trade.BuyLimit(volume,price,symbol,SL,TP,ORDER_TIME_GTC,expiration,comment))
                     {return orderDoneOrError(false, __FUNCTION__, trade);}
               }
            else if(actionType=="ORDER_TYPE_SELL_LIMIT")
               {
                  if(trade.SellLimit(volume,price,symbol,SL,TP,ORDER_TIME_GTC,expiration,comment))
                     {return orderDoneOrError(false, __FUNCTION__, trade);}
               }
            else if(actionType=="ORDER_TYPE_BUY_STOP")
               {
                  if(trade.BuyStop(volume,price,symbol,SL,TP,ORDER_TIME_GTC,expiration,comment))
                     {return orderDoneOrError(false, __FUNCTION__, trade);}
               }
            else if (actionType=="ORDER_TYPE_SELL_STOP")
               {
                  if(trade.SellStop(volume,price,symbol,SL,TP,ORDER_TIME_GTC,expiration,comment))
                     {return orderDoneOrError(false, __FUNCTION__, trade);}
               }
          }
      // Position modify    
      else if(actionType=="POSITION_MODIFY")
         {
            if(trade.PositionModify(idNimber,SL,TP)) 
               {return orderDoneOrError(false, __FUNCTION__, trade);}
         }
      // Position close partial   
      else if(actionType=="POSITION_PARTIAL")
         {
            if(trade.PositionClosePartial(idNimber,volume)) 
               {return orderDoneOrError(false, __FUNCTION__, trade);}
         }
      // Position close by id       
      else if(actionType=="POSITION_CLOSE_ID")
         {
            if(trade.PositionClose(idNimber)) 
               {return orderDoneOrError(false, __FUNCTION__, trade);}
         }
      // Position close by symbol
      else if(actionType=="POSITION_CLOSE_SYMBOL")
         {
            if(trade.PositionClose(symbol)) 
               {return orderDoneOrError(false, __FUNCTION__, trade);}
         }
      // Modify pending order
      else if(actionType=="ORDER_MODIFY")
         {  
            if(trade.OrderModify(idNimber,price,SL,TP,ORDER_TIME_GTC,expiration))
               {return orderDoneOrError(false, __FUNCTION__, trade);}
        }
      // Cancel pending order  
      else if(actionType=="ORDER_CANCEL")
         {
            if(trade.OrderDelete(idNimber))
               {return orderDoneOrError(false, __FUNCTION__, trade);}
         }
      // Action type dosen't exist
      else return actionDoneOrError(65538, __FUNCTION__);
      
      // Order is not compleated
      return orderDoneOrError(true, __FUNCTION__, trade);
   }

//+------------------------------------------------------------------+ 
//| TradeTransaction function                                        | 
//+------------------------------------------------------------------+ 
void CRestApi::OnTradeTransaction(const MqlTradeTransaction &trans,
                        const MqlTradeRequest &request,
                        const MqlTradeResult &result) {
      ENUM_TRADE_TRANSACTION_TYPE  trans_type=trans.type;
      switch(trans.type)
        {
         // case  TRADE_TRANSACTION_POSITION: {}  break;
         // case  TRADE_TRANSACTION_DEAL_ADD: {}  break;
         case  TRADE_TRANSACTION_REQUEST:
            {
               CJAVal data, req, res;
               
               req["action"]=EnumToString(request.action);
               req["order_id"]=(int) request.order;
               req["symbol"]=(string) request.symbol;
               req["volume"]=(double) request.volume;
               req["price"]=(double) request.price;
               req["stoplimit"]=(double) request.stoplimit;
               req["sl"]=(double) request.sl;
               req["tp"]=(double) request.tp;
               req["deviation"]=(int) request.deviation;
               req["type"]=EnumToString(request.type);
               req["type_filling"]=EnumToString(request.type_filling);
               req["type_time"]=EnumToString(request.type_time);
               req["expiration"]=(int) request.expiration;
               req["comment"]=(string) request.comment;
               req["position"]=(int) request.position;
               req["position_by"]=(int) request.position_by;
               
               res["retcode"]=(int) result.retcode;
               res["result"]=(string) CRestApi::GetRetcodeID(result.retcode);
               res["deal"]=(int) result.order;
               res["order_id"]=(int) result.order;
               res["volume"]=(double) result.volume;
               res["price"]=(double) result.price;
               res["comment"]=(string) result.comment;
               res["request_id"]=(int) result.request_id;
               res["retcode_external"]=(int) result.retcode_external;

               data["request"].Set(req);
               data["result"].Set(res);
               
               string t=data.Serialize();
               Print("event: " + t);
               
               uchar d[];
               StringToCharArray(t,d);
               int ret = RaiseEvent( d );
               
               Print("Event status: " + IntegerToString( ret ));
               //return t;
            }
         break;
         default: {} break;
        }
}

string CRestApi::orderDoneOrError(bool error, string funcName, CTrade &trade) {
      CJAVal conf;
      
      conf["error"]=(int) trade.ResultRetcode();
      conf["desription"]=(string) CRestApi::GetRetcodeID(trade.ResultRetcode());
      // conf["deal"]=(int) trade.ResultDeal(); 
      conf["order_id"]=(int) trade.ResultOrder();
      conf["volume"]=(double) trade.ResultVolume();
      conf["price"]=(double) trade.ResultPrice();
      conf["bid"]=(double) trade.ResultBid();
      conf["ask"]=(double) trade.ResultAsk();
      conf["function"]=(string) funcName;
      
      string t=conf.Serialize();
      if(debug) Print(t);
      
      return t;
}

//+------------------------------------------------------------------+
//| Action confirmation                                              |
//+------------------------------------------------------------------+
string CRestApi::actionDoneOrError(int lastError, string funcName) {
      CJAVal conf;
      
      conf["error"]=(string) lastError;
      conf["description"]=GetErrorID(lastError);
      conf["function"]=(string) funcName;
      
      string t=conf.Serialize();
      if(debug) Print(t);
      
      return t;
}

string CRestApi::GetRetcodeID(int retcode) { 
   switch(retcode) 
     { 
      case 10004: return("TRADE_RETCODE_REQUOTE");             break; 
      case 10006: return("TRADE_RETCODE_REJECT");              break; 
      case 10007: return("TRADE_RETCODE_CANCEL");              break; 
      case 10008: return("TRADE_RETCODE_PLACED");              break; 
      case 10009: return("TRADE_RETCODE_DONE");                break; 
      case 10010: return("TRADE_RETCODE_DONE_PARTIAL");        break; 
      case 10011: return("TRADE_RETCODE_ERROR");               break; 
      case 10012: return("TRADE_RETCODE_TIMEOUT");             break; 
      case 10013: return("TRADE_RETCODE_INVALID");             break; 
      case 10014: return("TRADE_RETCODE_INVALID_VOLUME");      break; 
      case 10015: return("TRADE_RETCODE_INVALID_PRICE");       break; 
      case 10016: return("TRADE_RETCODE_INVALID_STOPS");       break; 
      case 10017: return("TRADE_RETCODE_TRADE_DISABLED");      break; 
      case 10018: return("TRADE_RETCODE_MARKET_CLOSED");       break; 
      case 10019: return("TRADE_RETCODE_NO_MONEY");            break; 
      case 10020: return("TRADE_RETCODE_PRICE_CHANGED");       break; 
      case 10021: return("TRADE_RETCODE_PRICE_OFF");           break; 
      case 10022: return("TRADE_RETCODE_INVALID_EXPIRATION");  break; 
      case 10023: return("TRADE_RETCODE_ORDER_CHANGED");       break; 
      case 10024: return("TRADE_RETCODE_TOO_MANY_REQUESTS");   break; 
      case 10025: return("TRADE_RETCODE_NO_CHANGES");          break; 
      case 10026: return("TRADE_RETCODE_SERVER_DISABLES_AT");  break; 
      case 10027: return("TRADE_RETCODE_CLIENT_DISABLES_AT");  break; 
      case 10028: return("TRADE_RETCODE_LOCKED");              break; 
      case 10029: return("TRADE_RETCODE_FROZEN");              break; 
      case 10030: return("TRADE_RETCODE_INVALID_FILL");        break; 
      case 10031: return("TRADE_RETCODE_CONNECTION");          break; 
      case 10032: return("TRADE_RETCODE_ONLY_REAL");           break; 
      case 10033: return("TRADE_RETCODE_LIMIT_ORDERS");        break; 
      case 10034: return("TRADE_RETCODE_LIMIT_VOLUME");        break; 
      case 10035: return("TRADE_RETCODE_INVALID_ORDER");       break; 
      case 10036: return("TRADE_RETCODE_POSITION_CLOSED");     break; 
      case 10038: return("TRADE_RETCODE_INVALID_CLOSE_VOLUME");break; 
      case 10039: return("TRADE_RETCODE_CLOSE_ORDER_EXIST");   break; 
      case 10040: return("TRADE_RETCODE_LIMIT_POSITIONS");     break;  
      case 10041: return("TRADE_RETCODE_REJECT_CANCEL");       break; 
      case 10042: return("TRADE_RETCODE_LONG_ONLY");           break;
      case 10043: return("TRADE_RETCODE_SHORT_ONLY");          break;
      case 10044: return("TRADE_RETCODE_CLOSE_ONLY");          break;
      
      default: 
         return("TRADE_RETCODE_UNKNOWN="+IntegerToString(retcode)); 
         break; 
     } 
  }
  
//+------------------------------------------------------------------+
//| Get error message by error id                                    |
//+------------------------------------------------------------------+ 
static string CRestApi::GetErrorID(int error) { 
   switch(error) { 
      case 0:     return("ERR_SUCCESS");                       break; 
      case 4301:  return("ERR_MARKET_UNKNOWN_SYMBOL");         break;  
      case 4303:  return("ERR_MARKET_WRONG_PROPERTY");         break;
      case 4752:  return("ERR_TRADE_DISABLED");                break;
      case 4753:  return("ERR_TRADE_POSITION_NOT_FOUND");      break;
      case 4754:  return("ERR_TRADE_ORDER_NOT_FOUND");         break; 
      // Custom errors
      case 65537: return("ERR_DESERIALIZATION");               break;
      case 65538: return("ERR_WRONG_ACTION");                  break;
      case 65539: return("ERR_WRONG_ACTION_TYPE");             break;
      
      default: 
         return("ERR_CODE_UNKNOWN="+IntegerToString(error)); 
         break; 
     } 
}

string CRestApi::fromDateTime(datetime param) {
   CString s_iso8601;
   
   s_iso8601.Assign(TimeToString(param, TIME_DATE|TIME_MINUTES|TIME_SECONDS));
   s_iso8601.Replace(" ", "T");
   s_iso8601.Replace(".", "-");
   s_iso8601.Append(".000Z");
   return s_iso8601.Str();
}