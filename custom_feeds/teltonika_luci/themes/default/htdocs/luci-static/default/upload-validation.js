//----------------------------------------------------------------------------------------------------
//----------------------------------------------File size upload validation --------------------------
//----------------------------------------------------------------------------------------------------


function CheckFilesize(Fieldid, size, sizetext, sizetextempty){
	if ((typeof(size) !== 'undefined') && (size !== null) && (size !== "")) {
		if(window.ActiveXObject){
			var fso = new ActiveXObject("Scripting.FileSystemObject");
        	var filepath = document.getElementById(Fieldid).value;
            var thefile = fso.getFile(filepath);
            var sizeinbytes = thefile.size;
		}else{
			
			if(document.getElementById(Fieldid).files[0]){
				var sizeinbytes = document.getElementById(Fieldid).files[0].size;
			}else{
				var success = document.getElementById("maincontent");
				if(!document.getElementById("err_message")){
					success.insertAdjacentHTML("afterBegin", '<div id="err_message" class="alert-message error"> No file selected, please select a file</div>');
				}
				return false;
			}
		}
			var fSExt = new Array('Bytes', 'KB', 'MB', 'GB');
			fSize = sizeinbytes; i=0;while(fSize>900){fSize/=1024;i++;}
			var maxsize = size
			if (sizeinbytes == 0){
				document.getElementById(Fieldid).value = ""
				var checkerror = document.getElementById("err_message");
				if (checkerror !== null){
					document.getElementById("err_message").style.display = "none";
				}
				if ((typeof(sizetextempty) !== 'undefined') && (sizetextempty !== null) && (sizetextempty !== "")){
					var success = document.getElementById("maincontent")
					success.insertAdjacentHTML("afterBegin", '<div id="err_message" class="alert-message error"> ' + sizetextempty + ' </div>')
					document.body.scrollTop = document.documentElement.scrollTop = 0;
					return false;
				}else{
					var success = document.getElementById("maincontent")
					success.insertAdjacentHTML("afterBegin", '<div id="err_message" class="alert-message error"> <%=translate("Selected file is empty")%> </div>')
					document.body.scrollTop = document.documentElement.scrollTop = 0;
					return false;
				}				
			}else{ 
				if (sizeinbytes>size){
					document.getElementById(Fieldid).value = ""
					var checkerror = document.getElementById("err_message");
					if (checkerror !== null){
						document.getElementById("err_message").style.display = "none";

					}
					if ((typeof(sizetext) !== 'undefined') && (sizetext !== null) && (sizetext !== "")){
						var success = document.getElementById("maincontent")
						success.insertAdjacentHTML("afterBegin", '<div id="err_message" class="alert-message error"> ' + sizetext + ' </div>')
						document.body.scrollTop = document.documentElement.scrollTop = 0;
						return false;
						//alert((Math.round(fSize*100)/100)+' '+fSExt[i]);
					}else{
						//default error text
						var success = document.getElementById("maincontent")
						success.insertAdjacentHTML("afterBegin", '<div id="err_message" class="alert-message error"> <%=translate("Selected file is too large")%> </div>')
						document.body.scrollTop = document.documentElement.scrollTop = 0;
						return false;
					}
				}else{
					var checkerror = document.getElementById("err_message");
					if (checkerror !== null){
						document.getElementById("err_message").style.display = "none";
					}
					
					if (Fieldid === "cert"){
						alert("File uploaded successfully");
					}
						
					return true;
				}
			}
	}else{
		return false;
	}
}


//----------------------------------------------------------------------------------------------------
//----------------------------------------Disable enter key-------------------------------------------
//----------------------------------------------------------------------------------------------------
function stopRKey(evt) {
  var evt = (evt) ? evt : ((event) ? event : null);
  var node = (evt.target) ? evt.target : ((evt.srcElement) ? evt.srcElement : null);
  if ((evt.keyCode == 13) && (node.type=="text"))  {return false;}
}

document.onkeypress = stopRKey; 

//----------------------------------------------------------------------------------------------------
