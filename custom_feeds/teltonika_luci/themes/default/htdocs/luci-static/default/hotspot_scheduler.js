 function change_color(obj) 
 {	
	
	var classN
	classN = "schedule_all";
	if (obj.className != classN) 
		obj.className = classN;
	else
		obj.className = "schedule_clear"; 
}

function MakeVar()
{
	var TableObj = document.getElementById("matrix");
	var TrObj = TableObj.getElementsByTagName("tr");
	var HiddenField;

	HiddenField= document.getElementById("schedule_matrix");
	//HiddenField.value = 0;
	
	for (var loop = 0; loop < TrObj.length; loop++)
	{
		var TdObj = TrObj[loop].getElementsByTagName("td");
		for (var loop2 = 0; loop2 < TdObj.length; loop2++)
		{
			if (TdObj[loop2].className == "schedule_all")
			{
				HiddenField.value = HiddenField.value + "1";
			}
			else if (TdObj[loop2].className == "schedule_clear")
			{
				HiddenField.value = HiddenField.value + "0";
			}
		}
		//HiddenField.value = HiddenField.value + "|";
	}
}

function Select_All_Cells(id)
{
	var TrObj = document.getElementById(id);
	var Style;
	var TdObj = TrObj.getElementsByTagName("td");
	if (TdObj[1].className != "schedule_clear")
		Style = "schedule_clear";
	else
	{
		Style = "schedule_all";
	}
	
	for (var loop2 = 1; loop2 < TdObj.length; loop2++)
	{
		TdObj[loop2].className = Style;
	}
}
function Select_column(num)
{
	var Row1 = document.getElementById("tr_Monday");
	var Row2 = document.getElementById("tr_Tuesday");
	var Row3 = document.getElementById("tr_Wednesday");
	var Row4 = document.getElementById("tr_Thursday");
	var Row5 = document.getElementById("tr_Friday");
	var Row6 = document.getElementById("tr_Saturday");
	var Row7 = document.getElementById("tr_Sunday");
	var TdObj1 = Row1.getElementsByTagName("td");
	var TdObj2 = Row2.getElementsByTagName("td");
	var TdObj3 = Row3.getElementsByTagName("td");
	var TdObj4 = Row4.getElementsByTagName("td");
	var TdObj5 = Row5.getElementsByTagName("td");
	var TdObj6 = Row6.getElementsByTagName("td");
	var TdObj7 = Row7.getElementsByTagName("td");
	var Style;
	var ColumnsCell = TdObj1[num+1]
	
	if (ColumnsCell.className != "schedule_clear")
		Style = "schedule_clear";
	else
	{
		Style = "schedule_all";
	}
	TdObj1[num+1].className = Style;
	TdObj2[num+1].className = Style;
	TdObj3[num+1].className = Style;
	TdObj4[num+1].className = Style;
	TdObj5[num+1].className = Style;
	TdObj6[num+1].className = Style;
	TdObj7[num+1].className = Style;
}
