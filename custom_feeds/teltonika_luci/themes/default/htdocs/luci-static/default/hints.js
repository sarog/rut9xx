//----------------------------------------------------------------------------------------------------
//---------------------------------------------- Hints -----------------------------------------------
//----------------------------------------------------------------------------------------------------
var Container = null;
var iPosX = 0;
var iPosXTemp = 0;
var iPosY = 0;
var TempObj;

function ShowHint(Field, HintString)
	{
		if (HintString=="")
			return;
	Field.style.color="#808080";
	myVar = setTimeout(function(){

		iPosX = iPosY = 0;
		TempObj = Field;

			if(TempObj.offsetParent)
				{
					do
						{
							iPosX = iPosX + TempObj.offsetLeft;
							iPosY = iPosY + TempObj.offsetTop;
						} 
						while( (TempObj = TempObj.offsetParent) );
				}

			if(Container == null)
				{
					Container = document.createElement("div");
					Container.className = "hint";
					Container.style.top = 0;
					Container.style.left = 0;
					Container.onmouseover = function(){Container.style.display = "none";};
					document.body.appendChild(Container);
				}
			Container.innerHTML = HintString;
			Container.style.display = "block";

			if((Container.offsetWidth + iPosX) > document.body.offsetWidth)
				{
					iPosXTemp = iPosX;
					iPosX = iPosX - ((Container.offsetWidth + iPosX) - document.body.offsetWidth) - 2;
					if((iPosX + Container.offsetWidth) < iPosXTemp)
						iPosX = iPosXTemp - Container.offsetWidth;
				}
			Container.style.top = (Field.offsetHeight + iPosY + 2) + "px";  
			Container.style.left = iPosX + "px";
	},500)
}

function HideHint(Field)
	{
		Field.style.color="#404040";
		if(typeof myVar != 'undefined')
			clearTimeout(myVar)
		if(Container)
			Container.style.display = "none";
	}
//----------------------------------------------------------------------------------------------------
//----------------------------------------Disable enter key-------------------------------------------
//----------------------------------------------------------------------------------------------------
function stopRKey(evt) {
	evt = (evt) ? evt : ((event) ? event : null);
	var node = (evt.target) ? evt.target : ((evt.srcElement) ? evt.srcElement : null);
	if ((evt.keyCode == 13) && (node.type=="text"))  {return false;}
}

document.onkeypress = stopRKey; 

//----------------------------------------------------------------------------------------------------

function alert_message(id, val, msg) {
	if (msg) {
		var e = document.getElementById(id).checked
		if (e && val == "1" ){
			alert(msg);
		}
		else if ( !e && val == "0") {
			alert(msg);
		}
	}
}

function confirm_message(id, val, msg) {
	if (msg) {
		var e = document.getElementById(id);
		if (e.checked && val == "1"){
			if (!confirm(msg))
				e.checked = false;
		}
		else if (!e.checked && val == "0") {
			if (!confirm(msg))
				e.checked = true;
		}
	}
}
