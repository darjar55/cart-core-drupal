<?php

#################### mpgGlobals ###########################################


class MpiGlobals{
 
var $Globals=array(
                  MONERIS_PROTOCOL => 'https',
                  MONERIS_HOST => 'esqa.moneris.com',
                  MONERIS_PORT =>'443',
                  MONERIS_FILE => '/mpi/servlet/MpiServlet',
                  API_VERSION  =>'MPI Version 1.00(php)',
                  CLIENT_TIMEOUT => '60'
                 );

 function MpiGlobals()
 {
  // default
 }


 function getGlobals()
 {
  return($this->Globals);
 }

}//end class mpgGlobals



###################### MpiHttpsPost #########################################

class MpiHttpsPost{
 
 var $api_token;
 var $store_id;
 var $mpiRequest;
 var $mpiResponse;

 function MpiHttpsPost($storeid,$apitoken,$mpiRequestOBJ)
 {
  
  $this->store_id=$storeid;
  $this->api_token= $apitoken; 
  $this->mpiRequest=$mpiRequestOBJ;
  $dataToSend=$this->toXML();

  $g=new MPiGlobals();
  $gArray=$g->getGlobals();

  $url=$gArray[MONERIS_PROTOCOL]."://".
       $gArray[MONERIS_HOST].":".
       $gArray[MONERIS_PORT].
       $gArray[MONERIS_FILE];

  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL,$url);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER,1);
  curl_setopt ($ch, CURLOPT_HEADER, 0);
  curl_setopt($ch, CURLOPT_POST, 1);
  curl_setopt($ch, CURLOPT_POSTFIELDS,$dataToSend);
  curl_setopt($ch,CURLOPT_TIMEOUT,$gArray[CLIENT_TIMEOUT]);
  curl_setopt($ch,CURLOPT_USERAGENT,$gArray[API_VERSION]);

  $response=curl_exec ($ch);

  curl_close ($ch);

  if(!$response)
   {
        
     $response="<?xml version=\"1.0\"?>".
                "<MpiResponse>".
                "<type>null</type>".
                "<success>false</success>".
                "<message>null</message>".
                "<PaReq>null</PaReq>".
                "<TermUrl>null</TermUrl>".
                "<MD>null</MD>".
                "<ACSUrl>null</ACSUrl>".
                "<cavv>null</cavv>".
                "<PAResVerified>null</PAResVerified>".
                "</MpiResponse>";
   }

   // echo "$response";exit();

  $this->mpiResponse=new MpiResponse($response);
     
 }



 function getMpiResponse()
 {
  return $this->mpiResponse;

 }

 function toXML( )
 {
  
  $req=$this->mpiRequest ;
  $reqXMLString=$req->toXML();

  $xmlString .="<?xml version=\"1.0\"?>".
               "<MpiRequest>".
               "<store_id>$this->store_id</store_id>".
               "<api_token>$this->api_token</api_token>".
                $reqXMLString.
                "</MpiRequest>";
 
  return ($xmlString); 
 
 }

}//end class mpiHttpsPost



############# MpiResponse #####################################################


class MpiResponse{

 var $responseData;
 
 var $p; //parser

 var $currentTag;  
 var $receiptHash = array();
 var $currentTxnType; 

 var $ACSUrl;

 function MpiResponse($xmlString)
 {

  $this->p = xml_parser_create();
  xml_parser_set_option($this->p,XML_OPTION_CASE_FOLDING,0);
  xml_parser_set_option($this->p,XML_OPTION_TARGET_ENCODING,"UTF-8");
  xml_set_object($this->p,$this);
  xml_set_element_handler($this->p,"startHandler","endHandler");
  xml_set_character_data_handler($this->p,"characterHandler");
  xml_parse($this->p,$xmlString);
  xml_parser_free($this->p);


 }//end of constructor

             

//vbv start

function getMpiMessage(){
 
 return ($this->responseData['message']);

}


function getMpiSuccess(){

 return ($this->responseData['success']);

}

function getMpiPAResVerified(){

	return ($this->responseData['PAResVerified']);
}

function getMpiAcsUrl(){

    return ($this->responseData['ACSUrl']);
}

function getMpiPaReq(){

    return ($this->responseData['PaReq']);
}
function getMpiTermUrl(){

    return ($this->responseData['TermUrl']);
}

function getMpiMD(){

    return ($this->responseData['MD']);    
}


function getMpiCavv(){

    return ($this->responseData['cavv']);
}



 function getMpiResponseData(){

   return($this->responseData);

}

function getMpiPopUpWindow(){
	
$popUpForm ='<html><head><title>Title for Page</title></head><SCRIPT LANGUAGE="Javascript" >' . 
"<!--
function OnLoadEvent()
{
window.name='mainwindow';
//childwin = window.open('about:blank','popupName','height=400,width=390,status=yes,dependent=no,scrollbars=yes,resizable=no');
//document.downloadForm.target = 'popupName';
document.downloadForm.submit();
}
-->
</SCRIPT>" .
'<body onload="OnLoadEvent()">
<form name="downloadForm" action="' . $this->getMpiAcsUrl() . 
'" method="POST">
<noscript>
<br>
<br>
<center>
<h1>Processing your 3-D Secure Transaction</h1>
<h2>
JavaScript is currently disabled or is not supported
by your browser.<br>
<h3>Please click on the Submit button to continue
the processing of your 3-D secure
transaction.</h3>
<input type="submit" value="Submit">
</center>
</noscript>
<input type="hidden" name="PaReq" value="' . $this->getMpiPaReq() . '">
<input type="hidden" name="MD" value="' . $this->getMpiMD() . '">
<input type="hidden" name="TermUrl" value="' . $this->getMpiTermUrl() .'">
</form>
</body>
</html>';

return $popUpForm; 

}


function getMpiInLineForm(){
	
$inLineForm ='<html><head><title>Title for Page</title></head><SCRIPT LANGUAGE="Javascript" >' . 
"<!--
function OnLoadEvent()
{
document.downloadForm.submit();
}
-->
</SCRIPT>" .
'<body onload="OnLoadEvent()">
<form name="downloadForm" action="' . $this->getMpiAcsUrl() . 
'" method="POST">
<noscript>
<br>
<br>
<center>
<h1>Processing your 3-D Secure Transaction</h1>
<h2>
JavaScript is currently disabled or is not supported
by your browser.<br>
<h3>Please click on the Submit button to continue
the processing of your 3-D secure
transaction.</h3>
<input type="submit" value="Submit">
</center>
</noscript>
<input type="hidden" name="PaReq" value="' . $this->getMpiPaReq() . '">
<input type="hidden" name="MD" value="' . $this->getMpiMD() . '">
<input type="hidden" name="TermUrl" value="' . $this->getMpiTermUrl() .'">
</form>
</body>
</html>';

return $inLineForm; 
}

function characterHandler($parser,$data){

    
    $this->responseData[$this->currentTag] .=trim($data);

}//end characterHandler



function startHandler($parser,$name,$attrs){
    
    $this->currentTag=$name;

}


function endHandler($parser,$name){

}


}//end class MpiResponse


################## mpgRequest ###########################################################

class MpiRequest{

 var $txnTypes =array(

                     txn =>array('xid', 'amount', 'pan', 'expdate','MD',
					'merchantUrl','accept','userAgent','currency','recurFreq',
					'recurEnd','install'),
                      acs=> array('PaRes','MD')
                    );
var $txnArray;

function MpiRequest($txn){

 if(is_array($txn))
   {
    $this->txnArray = $txn;
   }
 else
   {
    $temp[0]=$txn;
    $this->txnArray=$temp;
   }  
}

function toXML(){
 
 $tmpTxnArray=$this->txnArray;
 $txnArrayLen=count($tmpTxnArray); //total number of transactions

 for($x=0;$x < $txnArrayLen;$x++)  
 {
    $txnObj=$tmpTxnArray[$x];
    $txn=$txnObj->getTransaction();

    $txnType=array_shift($txn);
    $tmpTxnTypes=$this->txnTypes; 
    $txnTypeArray=$tmpTxnTypes[$txnType];
    $txnTypeArrayLen=count($txnTypeArray); //length of a specific txn type
    
    $txnXMLString="";
    for($i=0;$i < $txnTypeArrayLen ;$i++) 
    {
      $txnXMLString  .="<$txnTypeArray[$i]>"   //begin tag
                       .$txn[$txnTypeArray[$i]] // data
                       . "</$txnTypeArray[$i]>"; //end tag 
      
    }
     
   $txnXMLString = "<$txnType>$txnXMLString";

   $txnXMLString .="</$txnType>";

   $xmlString .=$txnXMLString;

 } 

 return $xmlString;

}//end toXML

}//end class



class MpiTransaction{

 var $txn;

 function MpiTransaction($txn){

  $this->txn=$txn; 

 }

function getTransaction(){

 return $this->txn;
} 

}//end class