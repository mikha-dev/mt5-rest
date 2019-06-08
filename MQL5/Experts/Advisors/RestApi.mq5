#property version   "1.00"

#include <RestApi.mqh>

input string   host = "http://localhost";
input int      port = 6542;
input int      commandWaitTimeout = 10;

CRestApi api;

int OnInit() {

   api.Init(host, port, commandWaitTimeout);

   EventSetMillisecondTimer(1);
   
   return(INIT_SUCCEEDED);
}

void OnDeinit(const int reason) {

   EventKillTimer();
   
   //api.Deinit();
}

void OnTimer() {
   api.Processing();
}

void OnTradeTransaction(const MqlTradeTransaction& trans,
                        const MqlTradeRequest& request,
                        const MqlTradeResult& result) {
                        
   api.OnTradeTransaction( trans, request, result );
}