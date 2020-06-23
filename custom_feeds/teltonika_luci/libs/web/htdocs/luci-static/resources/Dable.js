!(function (define) {
	define([], function () {
		return function (tableOrId) {
			var $export = {
				id: '',
				columns: [],
				rows: [],
				rowObjects: [],
				visibleRows: [],
				visibleRowObjects: [],
				hiddenColumns: [],
				currentFilter: '',
				sortColumn: null,
				sortOrder: 'descending',
				minimumSearchLength: 1,
				columnData: [],
				pageNumber: 0,
				pageSize: 10,
				pageSizes: [10, 25, 50, 100],
				pagerSize: 0,
				pagerIncludeFirstAndLast: false,
				async: false,
				asyncData: {},
				asyncStart: 0,
				asyncLength: 1000,
				tfoothtml: '',
				//Basic Styling
				style: 'none',
				evenRowColor: '#fff',
				oddRowColor: 'white',
				//Classes
				dableClass: '',
				headerClass: '',
				tableClass: '',
				sortClass: 'table-sort',
				evenRowClass: 'table-row-even',
				oddRowClass: 'table-row-odd',
				footerClass: '',
				pagerButtonsClass: 'table-page'
			};

			$export.RowCount = function () {
			  return $export.rows.length;
			}
			$export.VisibleRowCount = function () {
			  return $export.visibleRows.length;
			}
			$export.NumberOfPages = function () {
			  var n = $export.VisibleRowCount() / $export.pageSize;
			  return Math.ceil(n);
			};
			$export.GetPageForRow = function (row) {
			  return Math.ceil(row / $export.pageSize);
			}
			
			$export.asyncRequest = function (start,
			                                 filter,
			                                 sortColumn,
			                                 ascending,
			                                 asynchronous) {
			  if (typeof asynchronous == 'undefined') {
			    asynchronous = false;
			  }
				var dableRequest = new XMLHttpRequest();
				dableRequest.onreadystatechange = function () {
					if (dableRequest.readyState == 4 && dableRequest.status == 200) {
						var data = JSON.parse(dableRequest.responseText);
						var actualData = JSON.parse(data.d);    //stupid json
						var actualRows = actualData.rows;
						//create empty rows for the rest of the set
						actualRows.reverse();
						for (var i = 0; i < start; ++i) {
							actualRows.push([]);
						}
						actualRows.reverse();
						for (var i = (start + $export.asyncLength);
						     i < actualData.includedRowCount;
						     ++i) {
							actualRows.push([]);
						}
						//update
						$export.SetDataAsRows(actualRows);
						$export.RowCount = function () { return actualData.rowCount; };
						$export.VisibleRowCount = function () {
							return actualData.includedRowCount;
						};
						if (asynchronous != false
						    && !!(asynchronous
						          && asynchronous.call
						          && asynchronous.apply)) {
							asynchronous();
						}
					}
				}
				dableRequest.open('POST', $export.async, asynchronous != false);
				dableRequest.setRequestHeader('content-type', 'application/json');
				var requestObject = JSON.parse(JSON.stringify($export.asyncData));
				requestObject['start'] = start;
				$export.asyncStart = start;
				requestObject['count'] = $export.asyncLength;
				requestObject['filter'] = filter;
				requestObject['sortColumn'] = sortColumn == null ? -1 : sortColumn;
				requestObject['ascending'] = ascending;
				dableRequest.send(JSON.stringify(requestObject));
			}
			$export.searchFunc = function (event) {
				var searchBox = this;
				if (searchBox.id != $export.id + '_search') {
					return false;
				}
				if (!searchBox.value
				    || searchBox.value.length < $export.minimumSearchLength) {
				  $export.currentFilter = '';
				}
				else {
				  var searchText = searchBox.value;
				  $export.currentFilter = searchText;
				}
				if ($export.async) {
					var ascending = true;
					if ($export.sortOrder.length > 3
					    && $export.sortOrder.substr(0, 4).toLowerCase() == 'desc') {
						ascending = false;
					}
					//search is wired up to be async so the user can keep typing,
					//but it creates a race condition that is not conducive to
					//fast typing, so I'll have to figure out a fix
					$export.asyncRequest(
						0,
						$export.currentFilter,
						$export.sortColumn,
						ascending);
					var body = document.getElementById($export.id + '_body');
					$export.UpdateDisplayedRows(body);
					$export.UpdateStyle(document.getElementById($export.id));
				}
				else {
					var includedRows = [];
					var includedRowObjects = [];
					if ($export.currentFilter) {
						for (var i = 0; i < $export.filters.length; ++i) {
							for (var j = 0; j < $export.rows.length; ++j) {
								if (ArrayContains(includedRows, $export.rows[j])) {
									continue;
								}
								for (var k = 0; k < $export.rows[j].length; ++k) {
									if ($export.filters[i](
											$export.currentFilter,
											$export.rows[j][k])) {
										includedRows.push($export.rows[j]);
										includedRowObjects.push($export.rowObjects[j]);
										break;
									}
								}
							}
						}
					}
					else {
						includedRows = $export.rows;
						includedRowObjects = $export.rowObjects;
					}
					$export.visibleRows = includedRows;
					$export.visibleRowObjects = includedRowObjects;
					var body = document.getElementById($export.id + '_body');
					$export.UpdateDisplayedRows(body);
					$export.UpdateStyle(document.getElementById($export.id));
				}
			};
			$export.sortFunc = function (event) {
				var tag = this.tagName;
				//prevent sorting from some form elements
				if(tag != 'INPUT'
				   && tag != 'BUTTON'
				   && tag != 'SELECT'
				   && tag != 'TEXTAREA') {
					var columnCell = this;  //use this here, as the event.srcElement
																	//is probably a <span>
					var sortSpan = columnCell.querySelector('.' + $export.sortClass);
					var columnTag = columnCell.getAttribute('data-tag');
					var columnIndex = -1;
	
					for (var i = 0; i < $export.columnData.length; ++i) {
						if ($export.columnData[i].Tag.toLowerCase() ==
							columnTag.toLowerCase()) {
							columnIndex = i;
							break;
						}
					}
					if (columnIndex == -1) {
						return false;
					}
					$export.sortColumn = columnIndex;
					var ascend = false;
					if ($export.sortOrder.length > 3
					    && $export.sortOrder.substr(0, 4).toLowerCase() == 'desc') {
						ascend = true;  //switching from descending to ascending
					}
					if (ascend) {
						$export.sortOrder = 'asc';
						sortSpan.innerHTML = '<img src="/luci-static/resources/cbi/up.gif" />';
					}
					else {
						$export.sortOrder = 'desc';
						sortSpan.innerHTML = '<img src="/luci-static/resources/cbi/down.gif" />';
					}
	                
					if ($export.async) {
					  $export.asyncRequest(
							$export.asyncStart,
							$export.currentFilter,
							columnIndex, ascend);
					}
					else if ($export.columnData[columnIndex].CustomSortFunc) {
						$export.visibleRowObjects = $export.columnData[columnIndex]
							.CustomSortFunc(columnIndex, ascend, $export.visibleRowObjects);
					}
					else {
						$export.visibleRowObjects = $export
							.baseSort(columnIndex, ascend, $export.visibleRowObjects);
					}
					
					$export.visibleRows = $export.CreateRowsFromObjects($export.visibleRowObjects);
					$export.UpdateDisplayedRows(
						document.getElementById($export.id + '_body'));
					$export.UpdateStyle();
				}
			};
			
			$export.baseSort = function (columnIndex, ascending, currentRowObjects) {
				var isInt = true;
				var isDate = true;
				var newRowObjects = currentRowObjects.slice(0);
				for (var i = 0; i < currentRowObjects.length; ++i) {
					//simple 2/21/2010 style dates parse cleanly to int, so we can drop out
					//if this won't parse
					if (parseInt(currentRowObjects[i].Row[columnIndex]).toString()
						.toLowerCase() == 'nan') {
						isInt = false;
					}
					//check for dates
					var dateString = currentRowObjects[i].Row[columnIndex].toString();
					var splitDate = dateString.split(/-|\:|(?:,| )+/);
					if (splitDate.length != 6
							|| (splitDate[1].length < 1 || splitDate[1].length > 2)
							|| (splitDate[2].length < 1 || splitDate[2].length > 2)
							|| (splitDate[0].length != 2 && splitDate[0].length != 4)) {
						isDate = false;
					}
				}

				if (isDate) {
					newRowObjects = newRowObjects.sort(function (a, b) {
						//default to US Date schema
						var splitDateA = a.Row[columnIndex].split(/-|\:|(?:,| )+/);
						var yearA = splitDateA[0].toString();
						var monthA = splitDateA[1].toString();
						var dayA = splitDateA[2].toString();
						var hourA = splitDateA[3].toString();
						var minuteA = splitDateA[4].toString();
						var secondA = splitDateA[5].toString();
						if (yearA.length == 2) {
							yearA = "20" + yearA;
							//don't guess, let them use 4 digits if they want 19xx
						}
						if (monthA.length == 1) {
							monthA = "0" + monthA;
						}
						if (dayA.length == 1) {
							dayA = "0" + dayA;
						}
						if (hourA.length == 1) {
							hourA = "0" + hourA;
						}
						if (minuteA.length == 1) {
							minuteA = "0" + minuteA;
						}
						if (secondA.length == 1) {
							secondA = "0" + secondA;
						}
						var yearMonthDayA = yearA + monthA + dayA + hourA + minuteA + secondA;
						var splitDateB = b.Row[columnIndex].split(/-|\:|(?:,| )+/);
						var yearB = splitDateB[0].toString();
						var monthB = splitDateB[1].toString();
						var dayB = splitDateB[2].toString();
						var hourB = splitDateB[3].toString();
						var minuteB = splitDateB[4].toString();
						var secondB = splitDateB[5].toString();
						if (yearB.length == 2) {
							yearB = "20" + yearB;
							//don't guess, let them use 4 digits if they want 19xx
						}
						if (monthB.length == 1) {
							monthB = "0" + monthB;
						}
						if (dayB.length == 1) {
							dayB = "0" + dayB;
						}
						if (hourB.length == 1) {
							hourB = "0" + hourB;
						}
						if (minuteB.length == 1) {
							minuteB = "0" + minuteB;
						}
						if (secondB.length == 1) {
							secondB = "0" + secondB;
						}
						var yearMonthDayB = yearB + monthB + dayB + hourB + minuteB + secondB;
						return parseInt(yearMonthDayA) - parseInt(yearMonthDayB);
					});
				}
				else if (isInt) {
					newRowObjects = newRowObjects.sort(function (a, b) {
						return parseInt(a.Row[columnIndex]) - parseInt(b.Row[columnIndex]);
					});
				}
				else {
					newRowObjects = newRowObjects.sort(function (a, b) {
						if (a.Row[columnIndex] > b.Row[columnIndex]) {
							return 1;
						}
						else if (a.Row[columnIndex] < b.Row[columnIndex]) {
							return -1;
						}
						else {
							return 0;
						}
					});
				}

				if (!ascending) {
					newRowObjects = newRowObjects.reverse();
				}

				return newRowObjects;
			};
			$export.filters = [
				//PHRASES FILTER
				function (searchText, value) {
					searchText = searchText.toString().toLowerCase();
					value = value.toString().toLowerCase();
					var phrases = [];
					var regex = /\s*".*?"\s*/g;
					var match;
					while(match = regex.exec(searchText)) {
						var phrase = match[0].replace(/"/g, '').trim();
						phrases.push(phrase);
						searchText = searchText.replace(match[0], " ");
					}

					for (var i = 0; i < phrases.length; ++i) {
						if (value.indexOf(phrases[i]) > -1) {
							return true;
						}
					}
					return false;
				},
				//WORDS FILTER, IGNORING PHRASES
				function (searchText, value) {
					searchText = searchText.toString().toLowerCase();
					value = value.toString().toLowerCase();
					var regex = /\s*".*?"\s*/g;
					var match;
					while (match = regex.exec(searchText)) {
						searchText = searchText.replace(match[0], ' ');
					} //remove phrases
					var splitText = searchText.split(' ');
					for (var i = 0; i < splitText.length; ++i) {
						if (!splitText[i]) {  //clear out empty strings
							splitText.splice(i, 1);
							--i;
						}
					}

					for (var i = 0; i < splitText.length; ++i) {
						if (value.indexOf(splitText[i]) > -1) {
							return true;
						}
					}
					return false;
				}
			];

			$export.SetColumnNames = function (columnNames) {
				if (!columnNames) {
					return false;
				};
				
				for (var i = 0; i < columnNames.length; ++i) {
					if ($export.columnData.length <= i) {
						$export.columnData.push({
							Tag: columnNames[i],
							FriendlyName: columnNames[i],
							CustomSortFunc: null,
							CustomRendering: null
						});
					}
					else {
						$export.columnData[i].Name = columnNames[i];
					}
				}
			};
			
			$export.DeleteRow = function(rowNumber) {
				for (var i = 0; i < $export.rowObjects.length; ++i) {
					if ($export.rowObjects[i].RowNumber == rowNumber) {
						$export.rowObjects.splice(i, 1);
						$export.rows = $export.CreateRowsFromObjects($export.rowObjects);
						break;
					}
				}
				for (var i = 0; i < $export.visibleRowObjects.length; ++i) {
					if ($export.visibleRowObjects[i].RowNumber == rowNumber) {
						$export.visibleRowObjects.splice(i, 1);
						$export.visibleRows = $export.CreateRowsFromObjects($export.visibleRowObjects);
					}
				}
				var event = document.createEvent('KeyboardEvent');
				event.initEvent('keyup', true, true, window, false, false, false, false, 38, 38);
				document.querySelector('#' + $export.id + '_search').dispatchEvent(event);
			};
			
			$export.AddRow = function(row) {
				$export.rows.push(row);
				$export.rowObjects.push({ Row: row, RowNumber: $export.rowObjects[$export.rowObjects.length - 1].RowNumber + 1 });
				
				var event = document.createEvent('KeyboardEvent');
				event.initEvent('keyup', true, true, window, false, false, false, false, 38, 38);
				document.querySelector('#' + $export.id + '_search').dispatchEvent(event);
			};
			
			$export.CreateObjectsFromRows = function(rows) {
				var rowObjects = [];
				
				for (var i = 0; i < rows.length; ++i) {
					rowObjects.push({ Row: rows[i], RowNumber: i});
				}
				return rowObjects;
			};
			$export.CreateRowsFromObjects = function(objects) {
				var rows = [];
				
				for (var i = 0; i < objects.length; ++i) {
					rows.push(objects[i].Row);
				}
				
				return rows;
			}
			$export.SetDataAsColumns = function (columns) {
				if (!columns) {
					return false;
				}

				var tableRows = [];
				for (var i = 0; i < columns.length; ++i) {
					while (tableRows.length < columns[i].length) {
						tableRows.push([]);
					}
					for (var j = 0; j < columns[i].length; ++j) {
						tableRows[j][i] = columns[i][j];
					}
				};

				$export.columns = columns;
				$export.rows = tableRows;
				$export.rowObjects = $export.CreateObjectsFromRows(tableRows);
				$export.visibleRows = rows.slice(0);
				$export.visibleRowObjects = $export.rowObjects.slice(0);
			};
			$export.SetDataAsRows = function (rows) {
				if (!rows) {
					return false;
				}

				var tableColumns = [];
				for (var i = 0; i < rows.length; ++i) {
					while (tableColumns.length < rows[i].length) {
						tableColumns.push([]);
					}
					for (var j = 0; j < rows[i].length; ++j) {
						tableColumns[j][i] = rows[i][j];
					}
				}

				$export.columns = tableColumns;
				$export.rows = rows;
				$export.rowObjects = $export.CreateObjectsFromRows(rows);
				$export.visibleRows = rows.slice(0);;
				$export.visibleRowObjects = $export.rowObjects.slice(0);
			};

			$export.UpdateDisplayedRows = function (body) {
				if (!body) {
					body = document.getElementById($export.id + '_body');
					if (!body) {
						return false;
					}
				}
				var tempBody = body.cloneNode(false);
				while (tempBody.firstChild) {
				  tempBody.removeChild(tempBody.firstChild);
				}
				var displayedRows = [];
				var row = document.createElement('tr');
				var cell = document.createElement('td');
				//get the display start id
				var pageDisplay = ($export.pageNumber * $export.pageSize);
				if ($export.VisibleRowCount() <= pageDisplay) {
					//if this is too big, go back to page 1
					$export.pageNumber = 0;
					pageDisplay = 0;
				}
				//get the display end id
				var length = pageDisplay + $export.pageSize;
				if (pageDisplay + $export.pageSize >= $export.VisibleRowCount()) {
					//if this is too big, only show remaining rows
					length = $export.VisibleRowCount();
				}
			  //loop through the visible rows and display this page
				var rows = [];
				for (var i = pageDisplay; i < length; ++i) {
					var tempRow = row.cloneNode(false);
					if (i % 2 == 0) {
						tempRow.setAttribute('class', $export.evenRowClass);
					}
					else {
						tempRow.setAttribute('class', $export.oddRowClass);
					}

					for (var j = 0; j < $export.visibleRows[i].length; ++j) {
						var tempCell = cell.cloneNode(false);
						var text = $export.visibleRows[i][j];
						if ($export.columnData[j] && $export.columnData[j].CustomRendering != null) {
							text = $export.columnData[j].CustomRendering(text, $export.visibleRowObjects[i].RowNumber);
						}
						tempCell.innerHTML = text;
						tempRow.appendChild(tempCell);
					}
					tempBody.appendChild(tempRow);
				}
				
				if (body.parentElement) {
				  body.parentElement.replaceChild(tempBody, body);
				}
				body = tempBody;

				var footer = document.getElementById($export.id + '_footer');
				$export.UpdateFooter(footer);
				return body;
			};
			$export.UpdateFooter = function (footer) {
				if (!footer) {
					return false;
				}
				var start = ($export.pageNumber * $export.pageSize) + 1;
				var end = start + $export.pageSize - 1;
				if (end > $export.VisibleRowCount()) {
					end = $export.VisibleRowCount();
				}
				
				var showing = footer.querySelector('#' + $export.id + '_showing');
				if (showing) {
					showing.innerHTML = "Showing " + start + " to " + end + " of " +
						($export.VisibleRowCount()) + " entries";
					if ($export.VisibleRowCount() != $export.RowCount()) {
						showing.innerHTML += " (filtered from " + ($export.RowCount()) +
							" total entries)";
					}
				}
				var right = footer.querySelector('#' + $export.id +
					'_page_prev').parentElement;
				footer.replaceChild($export.BuildPager(), right);

				return footer;
			};
			$export.UpdateStyle = function (tableDiv, style) {
				if (!tableDiv) {
					tableDiv = document.getElementById($export.id);
					if (!tableDiv) {
						return false;
					}
				}
				if (!style) {
					style = $export.style;
				}
				$export.style = style;

				//initial style cleanup
				$export.RemoveStyles(tableDiv);
        //clear is a style option to completely avoid any styling so you can
				//roll your own
				if (style.toLowerCase() != 'clear') {
				  //base styles for 'none', the other styles sometimes build on these
					//so we apply them beforehand
				  $export.ApplyBaseStyles(tableDiv);

				  if (style.toLowerCase() == 'none') {
				    return true;
				  }
					else {
						if (style.toLowerCase() == 'jqueryui') {
							$export.ApplyJqueryUIStyles(tableDiv);
						}
						else if (style.toLowerCase() == 'bootstrap') {
							$export.ApplyBootstrapStyles(tableDiv);
						}
					}
				}
			};
			
			$export.RemoveStyles = function (tableDiv) {
				tableDiv.removeAttribute('class');
				var children = tableDiv.children;
				for (var i = 0; i < children.length; ++i) {
					children[i].removeAttribute('class');
				}
				var header = children[0];
				var headerChildren = header.children;
				for (var i = 0; i < headerChildren.length; ++i) {
					headerChildren[i].removeAttribute('class');
				}
				
				var table = children[1];
				var thead = table.children[0];
				thead.removeAttribute('class');
				thead.children[0].removeAttribute('class');
				var theadCells = thead.children[0].children;
				for (var i = 0; i < theadCells.length; ++i) {
					theadCells[i].removeAttribute('class');
				}
				var sorts = tableDiv.querySelectorAll('.' + $export.sortClass);
				for (var i = 0; i < sorts.length; ++i) {
					sorts[i].innerHTML = '<img src="/luci-static/resources/cbi/up.gif" />';
					sorts[i].setAttribute('class', $export.sortClass);
					if (i == $export.sortColumn) {
						if ($export.sortOrder.toLowerCase().substr(0, 4) == 'desc') {
							sorts[i].innerHTML = '<img src="/luci-static/resources/cbi/down.gif" />';
						}
					}
				}
				
				var tbody = table.children[1];
				tbody.removeAttribute('class');
				
				var footer = children[2];
				var footerChildren = footer.children;
				var leftChildren = footerChildren[0].children;
				for (var i = 0; i < leftChildren.length; ++i) {
					leftChildren[i].removeAttribute('class');
				}
				var right = footer.querySelector('#' + $export.id + '_page_prev')
					.parentElement;
				footer.replaceChild($export.BuildPager(), right);

				//basically, don't remove style from tfoot, in case user added it
				tableDiv.removeAttribute('style');
				RemoveStyle(children[0]);
				children[1].removeAttribute('style');
				RemoveStyle(children[2]);
				RemoveStyle(thead);
				RemoveStyle(tbody);
			}
			$export.ApplyBaseStyles = function (tableDiv) {
				if ($export.dableClass) {
					tableDiv.setAttribute('class', $export.dableClass);
				}
				var table = tableDiv.querySelector('table');
				table.setAttribute('style', 'width: 100%;');
				if ($export.tableClass) {
					table.setAttribute('class', $export.tableClass);
				}
				
				var oddRows = tableDiv.querySelectorAll('.' + $export.oddRowClass);
				for (var i = 0; i < oddRows.length; ++i) {
					oddRows[i].setAttribute('style', 'background-color: ' +
						$export.oddRowColor);
				}
				var evenRows = tableDiv.querySelectorAll('.' + $export.evenRowClass);
				for (var i = 0; i < evenRows.length; ++i) {
					evenRows[i].setAttribute('style', 'background-color: ' +
						$export.evenRowColor);
				}
				var cells = tableDiv.querySelectorAll('tbody td');
				for (var i = 0; i < cells.length; ++i) {
					cells[i].setAttribute('style', '');
				}

				var headCells = tableDiv.querySelectorAll('th');
				for (var i = 0; i < headCells.length; ++i) {
					headCells[i].setAttribute('style', '');
					var headCellLeft = headCells[i].children[0];
					headCellLeft.setAttribute('style', 'float: left;');
					if ($export.columnData[i].CustomSortFunc !== false) {
						var headCellRight = headCells[i].children[1];
						headCellRight.setAttribute('style', '');
						var headCellClear = headCells[i].children[2];
						headCellClear.setAttribute('style', 'clear: both;');
						
						headCells[i].onmouseover = function () {
							this.setAttribute('style', ' cursor: pointer');
						};
						headCells[i].onmouseout = function () {
							this.setAttribute('style', ' cursor: default');
						};
					}
					else {
						var headCellClear = headCells[i].children[1];
						headCellClear.setAttribute('style', 'clear: both;');
					}
				}

				var header = tableDiv.querySelector('#' + $export.id + '_header');
				header.setAttribute('style', '');
				if ($export.headerClass) {
					header.setAttribute('class', $export.headerClass);
				}
				var headLeft = header.children[0];
				headLeft.setAttribute('style', 'float: left; margin: 5px 0 5px 10px;');
				var headRight = header.children[1];
				headRight.setAttribute('style', 'float: right; margin: 5px 10px 5px 0px;');
				var headClear = header.children[2];
				headClear.setAttribute('style', 'clear: both;');

				var footer = tableDiv.querySelector('#' + $export.id + '_footer');
				footer.setAttribute('style', 'padding: 5px;');
				if ($export.footerClass) {
					footer.setAttribute('class', $export.footerClass);
				}
				var footLeft = footer.children[0];
				footLeft.setAttribute('style', 'float: left;');
				var footClear = footer.children[2];
				footClear.setAttribute('style', 'clear: both;');
				var footRight = footer.children[1];
				footRight.setAttribute('style', 'float: right; list-style: none;');
				var footRightItems = footRight.querySelectorAll('li');
				for (var i = 0; i < footRightItems.length; ++i) {
					footRightItems[i].setAttribute(
						'style',
						'display: inline; margin-right: 20px;');
				}
			}
			$export.ApplyJqueryUIStyles = function (tableDiv) {
				if (!tableDiv) {
					return false;
				}
				var header = tableDiv.querySelector('#' + $export.id + '_header');
				var footer = tableDiv.querySelector('#' + $export.id + '_footer');
				var span = document.createElement('span');

				header.setAttribute(
					'class',
					'fg-toolbar ui-widget-header ui-corner-tl ui-corner-tr ui-helper-clearfix');

				var headCells = tableDiv.querySelectorAll('th');
				for (var i = 0; i < headCells.length; ++i) {
					headCells[i].setAttribute('class', 'ui-state-default');
					var sort = headCells[i].querySelector('.' + $export.sortClass);
					if (sort) {
						if (sort.innerHTML == 'v') {
							sort.setAttribute('class', $export.sortClass +
								' ui-icon ui-icon-triangle-1-s');
						}
						else {
							sort.setAttribute('class', $export.sortClass +
								' ui-icon ui-icon-triangle-1-n');
						}
						sort.innerHTML = '';
					}
				}

				var pagerItems = footer.querySelectorAll('li');
				for (var i = 0; i < pagerItems.length; ++i) {
					RemoveStyle(pagerItems[i]);
				}
				footer.setAttribute(
					'class',
					'fg-toolbar ui-widget-header ui-corner-bl ui-corner-br ui-helper-clearfix');
				var pageClass = 'fg-button ui-button ui-state-default ui-corner-left ' +
					$export.pagerButtonsClass;

				var pageButtons = footer.querySelectorAll('.' +
					$export.pagerButtonsClass);
				for (var i = 0; i < pageButtons.length; ++i) {
					pageButtons[i].setAttribute('class', pageClass);
				}

				var pageLeft = footer.querySelector('#' + $export.id + '_page_prev');
				pageLeft.innerHTML = '';
				var pageLeftSpan = span.cloneNode(false);
				pageLeftSpan.setAttribute('class', 'ui-icon ui-icon-circle-arrow-w');
				pageLeft.appendChild(pageLeftSpan);
				if (pageLeft.getAttribute('disabled')) {
					pageLeft.setAttribute('class', pageClass + ' ui-state-disabled');
				}
				var pageRight = footer.querySelector('#' + $export.id + '_page_next');
				pageRight.innerHTML = '';
				var pageRightSpan = span.cloneNode(false);
				pageRightSpan.setAttribute('class', 'ui-icon ui-icon-circle-arrow-e');
				pageRight.appendChild(pageRightSpan);
				if (pageRight.getAttribute('disabled')) {
					pageRight.setAttribute('class', pageClass + ' ui-state-disabled');
				}

				if ($export.pagerIncludeFirstAndLast) {
				    var pageFirst = footer.querySelector('#' + $export.id +
							'_page_first');
				    var pageLast = footer.querySelector('#' + $export.id +
							'_page_last');
				    pageFirst.innerHTML = '';
				    var pageFirstSpan = span.cloneNode(false);
				    pageFirstSpan.setAttribute(
							'class',
							'ui-icon ui-icon-arrowthickstop-1-w');
				    pageFirst.appendChild(pageFirstSpan);
				    pageLast.innerHTML = '';
				    var pageLastSpan = span.cloneNode(false);
				    pageLastSpan.setAttribute(
							'class',
							'ui-icon ui-icon-arrowthickstop-1-e');
				    pageLast.appendChild(pageLastSpan);
				}
			};
			$export.ApplyBootstrapStyles = function (tableDiv) {
				if (!tableDiv) {
					return false;
				}
				var div = document.createElement('div');
				var span = document.createElement('span');
				var header = tableDiv.querySelector('#' + $export.id + '_header');
				var footer = tableDiv.querySelector('#' + $export.id + '_footer');
				var table = tableDiv.querySelector('table');
				table.setAttribute('class', 'table table-bordered table-striped');
				table.setAttribute('style', 'width: 100%; margin-bottom: 0;');
				header.setAttribute('class', 'panel-heading');
				footer.setAttribute('class', 'panel-footer');
				tableDiv.setAttribute('class', 'panel panel-info');
				tableDiv.setAttribute('style', 'margin-bottom: 0;');

				var tableRows = table.querySelectorAll('tbody tr');
				for (var i = 0; i < tableRows.length; ++i) {    //remove manual striping
					tableRows[i].removeAttribute('style');
				}

				var headCells = table.querySelectorAll('th');
				for (var i = 0; i < headCells.length; ++i) {
					var sort = headCells[i].querySelector('.' + $export.sortClass);
					if (sort) {
						if (sort.innerHTML == 'v') {
							sort.setAttribute('class', $export.sortClass +
								' glyphicon glyphicon-chevron-down');
						}
						else {
							sort.setAttribute('class', $export.sortClass +
								' glyphicon glyphicon-chevron-up');
						}
						sort.innerHTML = '';
					}
				}

				var pageClass = 'btn btn-default ' + $export.pagerButtonsClass;
				var pageLeft = footer.querySelector('#' + $export.id + '_page_prev');
				var pageRight = footer.querySelector('#' + $export.id + '_page_next');
				var pageParent = pageLeft.parentElement;
				
				var pagerItems = footer.querySelectorAll('li');
				for (var i = 0; i < pagerItems.length; ++i) {
					RemoveStyle(pagerItems[i]);
				}
				
				pageParent.setAttribute('class', 'btn-group');

				pageLeft.innerHTML = '';
				var pageLeftSpan = span.cloneNode(false);
				pageLeftSpan.setAttribute('class', 'glyphicon glyphicon-arrow-left');
				pageLeft.appendChild(pageLeftSpan);
				
				pageRight.innerHTML = '';
				var pageRightSpan = span.cloneNode(false);
				pageRightSpan.setAttribute('class', 'glyphicon glyphicon-arrow-right');
				pageRight.appendChild(pageRightSpan);

				if ($export.pagerIncludeFirstAndLast) {
					var pageFirst = footer.querySelector('#' + $export.id +
						'_page_first');
					var pageLast = footer.querySelector('#' + $export.id +
						'_page_last');
					pageFirst.innerHTML = '';
					var pageFirstSpan = span.cloneNode(false);
					pageFirstSpan.setAttribute(
						'class',
						'glyphicon glyphicon-fast-backward');
					pageFirst.appendChild(pageFirstSpan);
					pageLast.innerHTML = '';
					var pageLastSpan = span.cloneNode(false);
					pageLastSpan.setAttribute(
						'class',
						'glyphicon glyphicon-fast-forward');
					pageLast.appendChild(pageLastSpan);
				}

				var pageButtons = footer.querySelectorAll('.' +
					$export.pagerButtonsClass);
				for (var i = 0; i < pageButtons.length; ++i) {
						pageButtons[i].setAttribute('class', pageClass);
				}
			};
			
			$export.CheckForTable = function (input) {//Check for existing table
				if (input) {
					if (input.nodeType && input.nodeName.toLowerCase() == 'div') {
						if (input.hasAttribute('id')) {
							$export.id = input.getAttribute('id');
						}
						else {
							$export.id = 'Dable1';
							input.setAttribute('id', 'Dable1');
						}
					}
					else if (window.jQuery 
					         && input instanceof jQuery
					         && input[0].nodeType) {
						//jquery object
						if (input[0].hasAttribute('id')) {
							$export.id = input[0].getAttribute('id');
						}
						else {
							$export.id = 'Dable1';
							input[0].setAttribute('id', 'Dable1');
						}
					}
					else {
						$export.id = input.toString()
					}
					var tableDiv = document.getElementById($export.id);
					if (tableDiv && $export.rows && $export.rows.length < 1) {
						var table = tableDiv.querySelector('table');
						if (table) {
							if (tableDiv.hasAttribute('class')) {
								$export.dableClass = tableDiv.getAttribute('class');
							}
							var newTable = $export.GenerateTableFromHtml(table);
							//Make it a Dable!
							return newTable;
						}
					}
					return $export.id;
				}
				return false;
			}
			$export.GenerateTableFromHtml = function (tableNode) {
				if (!tableNode) {
					console.log("Dable Error: No HTML table to generate dable from");
					return false;
				}
				if (tableNode.hasAttribute('class')) {
					$export.tableClass = tableNode.getAttribute('class');
				}
				var thead = tableNode.querySelector('thead');
				if (!thead) {
					console.log("Dable Error: No thead element in table");
					return false;
				}
				var headers = thead.querySelectorAll('tr th');
				var tfoot = tableNode.querySelector('tfoot');
				if (tfoot) {
					$export.tfoothtml = tfoot.innerHTML;
				}
				var colNames = [];
				for (var i = 0; i < headers.length; ++i) {	//add our column names
					colNames.push(headers[i].innerHTML);
				}
				$export.SetColumnNames(colNames);
				
				var rowsHtml = tableNode.querySelectorAll('tbody tr');
				var allRows = [];
				if (rowsHtml.length > 1
				    && rowsHtml[0].hasAttribute('class')
				    && rowsHtml[1].hasAttribute('class')) {
					$export.evenRowClass = rowsHtml[0].getAttribute('class');
					$export.oddRowClass = rowsHtml[1].getAttribute('class');
				}
				for (var i = 0; i < rowsHtml.length; ++i) {
					allRows.push([]);
					var rowCells = rowsHtml[i].children;
					for (var j = 0; j < rowCells.length; ++j) {
						allRows[i].push(rowCells[j].innerHTML);
					}
				}
				$export.SetDataAsRows(allRows);
				
				var parentDiv = tableNode.parentElement;
				parentDiv.innerHTML = '';
				
				return parentDiv.id;
			};
			
			$export.Exists = function (tableDiv) {
				var result = false;
				
				var checkId = '';
				if (!tableDiv) {
					checkId = $export.id;
				}
				else if (tableDiv
					       && tableDiv.nodeType
					       && tableDiv.nodeName.toLowerCase() == 'div') {
					checkId = tableDiv.id;
				}
				else if (tableDiv
					       && window.jQuery 
					       && tableDiv instanceof jQuery
					       && tableDiv[0].nodeType) {
					checkId = tableDiv[0].id;
				}
				else if (tableDiv) {
					checkId = tableDiv;
				}
				checkId += '_header';
				var headerElement = document.getElementById(checkId);
				if (headerElement) {
					result = true;
				}
				
				return result;
			};
			
			$export.BuildAll = function (tableInput) {
				var tableId = $export.CheckForTable(tableInput);
				if (!tableId) {
					return false;
				}
				var tableDiv = document.getElementById(tableId);
				if (!tableDiv) {
					return false;
				}

				if ($export.async) {
				    $export.asyncRequest(0, '', -1, true);
				}
				
				tableDiv.innerHTML = '';

				var header = $export.BuildHeader(tableDiv);
				var table = $export.BuildTable(tableDiv);
				var footer = $export.BuildFooter(tableDiv);

				tableDiv.appendChild(header);
				tableDiv.appendChild(table);
				tableDiv.appendChild(footer);

				$export.UpdateStyle(tableDiv);
			};
			$export.BuildHeader = function (tableDiv) {
				if (!tableDiv) {
					return false;
				}
				var div = document.createElement('div');
				var span = document.createElement('span');
				var select = document.createElement('select');
				var option = document.createElement('option');
				var input = document.createElement('input');

				var left = div.cloneNode(false);
				var show = span.cloneNode(false);
				check_if_exists = document.getElementById('legend');
				if(check_if_exists != null){
					if (check_if_exists.innerText == "SMS Messages")
						show.innerHTML = 'SMS per page ';
					else
						show.innerHTML = 'Packages per page ';
				}else{
					show.innerHTML = 'Events per page ';
				}
				left.appendChild(show);
				var entryCount = select.cloneNode(false);
				for (var i = 0; i < $export.pageSizes.length; ++i) {
					var tempOption = option.cloneNode(false);
					tempOption.innerHTML = $export.pageSizes[i];
					tempOption.setAttribute('value', $export.pageSizes[i]);
					entryCount.appendChild(tempOption);
				}
				entryCount.onchange = function () {
					var entCnt = this;
					var value = entCnt.value;
					$export.pageSize = parseInt(value);
					$export.UpdateDisplayedRows(document.getElementById($export.id +
						'_body'));
					$export.UpdateStyle(tableDiv);
				};
				var options = entryCount.querySelectorAll('option');
				for (var i = 0; i < options.length; ++i) {
					if (options[i].value == $export.pageSize) {
						options[i].selected = true;
						break;
					}
				}
				left.appendChild(entryCount);

				var right = div.cloneNode(false);
				var search = span.cloneNode(false);
				search.innerHTML = 'Search ';
				right.appendChild(search);
				var inputSearch = input.cloneNode(false);
				inputSearch.setAttribute('id', $export.id + '_search');
				inputSearch.onkeyup = $export.searchFunc;
				right.appendChild(inputSearch);

				var clear = div.cloneNode(false);

				var head = div.cloneNode(false);
				head.id = $export.id + '_header';
				head.appendChild(left);
				head.appendChild(right);
				head.appendChild(clear);

				return head;
			};
			$export.BuildTable = function (tableDiv) {
				if (!tableDiv) {
					return false;
				}
				//all the elements we need to build a neat table
				var table = document.createElement('table');
				var head = document.createElement('thead');
				var headCell = document.createElement('th');
				var body = document.createElement('tbody');
				var row = document.createElement('tr');
				var span = document.createElement('span');
				//The thead section contains the column names
				var headRow = row.cloneNode(false);
				for (var i = 0; i < $export.columnData.length; ++i) {
					var tempCell = headCell.cloneNode(false);
					var nameSpan = span.cloneNode(false);
					nameSpan.innerHTML = $export.columnData[i].FriendlyName + ' ';
					tempCell.appendChild(nameSpan);

					if ($export.columnData[i].CustomSortFunc !== false) {
						var sortSpan = span.cloneNode(false);
						sortSpan.setAttribute('class', $export.sortClass);
						sortSpan.innerHTML = 'v';
						tempCell.appendChild(sortSpan);
						tempCell.onclick = $export.sortFunc;
					}

					var clear = span.cloneNode(false);
					tempCell.appendChild(clear);

					tempCell.setAttribute('data-tag', $export.columnData[i].Tag);
					headRow.appendChild(tempCell);
				}
				head.appendChild(headRow);
				table.appendChild(head);
				
				$export.visibleRows = $export.rows.slice(0);
				$export.visibleRowObjects = $export.rowObjects.slice(0);
				body = $export.UpdateDisplayedRows(body);
				body.id = $export.id + '_body';
				table.appendChild(body);
				if ($export.tfoothtml) {
					var foot = document.createElement('tfoot');
					foot.innerHTML = $export.tfoothtml;
					table.appendChild(foot);
				}

				return table;
			};
			$export.BuildFooter = function (tableDiv) {
				if (!tableDiv) {
					return false;
				}
				var div = document.createElement('div');
				var span = document.createElement('span');
				var button = document.createElement('button');

				var left = div.cloneNode(false);
				var showing = span.cloneNode(false);
				showing.id = $export.id + '_showing';
				left.appendChild(showing);

				var right = $export.BuildPager(footer);

				var clear = div.cloneNode(false);

				var footer = div.cloneNode(false);
				footer.id = $export.id + '_footer';
				footer.innerHTML = '';
				footer.appendChild(left);
				footer.appendChild(right);
				footer.appendChild(clear);
				return $export.UpdateFooter(footer);
			};
			$export.BuildPager = function () {
				var ul = document.createElement('ul');
				var li = document.createElement('li');
				var anchor = document.createElement('a');
				var right = ul.cloneNode(false);

				if ($export.pagerIncludeFirstAndLast) {
						var pageFirst = li.cloneNode(false);
				var pageFirstAnchor = anchor.cloneNode(false);
						pageFirstAnchor.innerHTML = 'First';
						pageFirst.setAttribute('class', $export.pagerButtonsClass);
						pageFirst.id = $export.id + '_page_first';
						pageFirst.onclick = $export.FirstPage;
				if ($export.pageNumber <= 0) {
					pageFirst.setAttribute('disabled', 'disabled');
					pageFirst.onclick = function () {};	//disable onclick
				}
				pageFirst.appendChild(pageFirstAnchor);
						right.appendChild(pageFirst);
				}

				var pageLeft = li.cloneNode(false);
				var pageLeftAnchor = anchor.cloneNode(false);
				pageLeftAnchor.innerHTML = '<< Prev';
				pageLeft.setAttribute('class', $export.pagerButtonsClass);
				pageLeft.id = $export.id + '_page_prev';
				pageLeft.onclick = $export.PreviousPage;
				if ($export.pageNumber <= 0) {
						pageLeft.setAttribute('disabled', 'disabled');
				pageLeft.onclick = function () {};	//disable onclick
				}
				pageLeft.appendChild(pageLeftAnchor);
				right.appendChild(pageLeft);

				if ($export.pagerSize > 0) {
					var start = $export.pageNumber - parseInt($export.pagerSize / 2);
					var length = start + $export.pagerSize;
					if ($export.pageNumber <= ($export.pagerSize / 2)) {
						// display from beginning
						length = $export.pagerSize;
						start = 0;
						if (length > $export.NumberOfPages()) {
							length = $export.NumberOfPages();
						}   //very small tables
					}
					else if (($export.NumberOfPages() - $export.pageNumber) <=
					         ($export.pagerSize / 2)) {
						//display the last five pages
						length = $export.NumberOfPages();
						start = $export.NumberOfPages() - $export.pagerSize;
					}

					for (var i = start; i < length; ++i) {
						var liNode = li.cloneNode(false);
						var liNodeAnchor = anchor.cloneNode(false);
						liNodeAnchor.innerHTML = (i + 1).toString();
						var page = i;
						liNode.onclick = function(j) {
							return function() {
								$export.GoToPage(j);
							}
						}(i);
						liNode.setAttribute('class', $export.pagerButtonsClass);
						if (i == $export.pageNumber) {
							liNode.setAttribute('disabled', 'disabled');
							liNode.onclick = function () {};	//disable onclick
						}
						liNode.appendChild(liNodeAnchor);
						right.appendChild(liNode);
					}
				}

				var pageRight = li.cloneNode(false);
				var pageRightAnchor = anchor.cloneNode(false);
				pageRightAnchor.innerHTML = 'Next >>';
				pageRight.setAttribute('class', $export.pagerButtonsClass);
				pageRight.id = $export.id + '_page_next';
				pageRight.onclick = $export.NextPage;
				if ($export.NumberOfPages() - 1 == $export.pageNumber) {
					pageRight.setAttribute('disabled', 'disabled');
					pageRight.onclick = function () {};	//disable onclick
				}
				pageRight.appendChild(pageRightAnchor);
				right.appendChild(pageRight);

				if ($export.pagerIncludeFirstAndLast) {
					var pageLast = li.cloneNode(false);
					var pageLastAnchor = anchor.cloneNode(false);
					pageLastAnchor.innerHTML = 'Last';
					pageLast.setAttribute('class', $export.pagerButtonsClass);
					pageLast.id = $export.id + '_page_last';
					pageLast.onclick = $export.LastPage;
					if ($export.NumberOfPages() - 1 == $export.pageNumber) {
						pageLast.setAttribute('disabled', 'disabled');
						pageLast.onclick = function () {};	//disable onclick
					}
					pageLast.appendChild(pageLastAnchor);
					right.appendChild(pageLast);
				}

				return right;
			};
			
			$export.FirstPage = function() {
				$export.pageNumber = 0;
				$export.GoToPage($export.pageNumber);
			};
			
			$export.PreviousPage = function() {
				$export.pageNumber -= 1;
				$export.GoToPage($export.pageNumber);
			};
			
			$export.GoToPage = function (page) {
				$export.pageNumber = page;
				if ($export.async &&
				($export.asyncStart > $export.pageNumber * $export.pageSize
				|| $export.pageNumber * $export.pageSize >=
					 $export.asyncStart + $export.asyncLength)) {
					var newStart = $export.pageNumber * $export.pageSize;
					var ascending = true;
					if ($export.sortOrder.length > 3
							&& $export.sortOrder.substr(0, 4).toLowerCase() ==
								 'desc') {
						ascending = false;
					}
					$export.asyncRequest(
						newStart,
						$export.currentFilter,
						$export.sortColumn,
						ascending);
				}
				$export.UpdateDisplayedRows(document.getElementById($export.id +
					'_body'));
				$export.UpdateStyle();
			};
			
			$export.NextPage = function() {
				$export.pageNumber += 1;
				$export.GoToPage($export.pageNumber);
			};
			
			$export.LastPage = function() {
				$export.pageNumber = $export.NumberOfPages() - 1;
				//page number is 0 based
				if ($export.async
						&& ($export.asyncStart > $export.pageNumber * $export.pageSize
								|| $export.pageNumber * $export.pageSize >
									 $export.asyncStart + $export.asyncLength)) {
					var newStart = 0;
					var pages = (1000 / $export.pageSize) - 1;
					//-1 for the page number and -1 to include current page
					if ($export.pageNumber - pages > -1) {
						newStart = ($export.pageNumber - pages) * $export.pageSize;
					}
					var ascending = true;
					if ($export.sortOrder.length > 3
							&& $export.sortOrder.substr(0, 4).toLowerCase() == 'desc') {
						ascending = false;
					}
					$export.asyncRequest(
						newStart,
						$export.currentFilter,
						$export.sortColumn,
						ascending);
				}
				$export.UpdateDisplayedRows(document.getElementById($export.id +
					'_body'));
				$export.UpdateStyle();
			};

			//Utility functions
			function ArrayContains(array, object) {
				for (var i = 0; i < array.length; ++i) {
					if (array[i] === object) {
						return true;
					}
				}
				return false;
			}
			function RemoveStyle(node) {
				node.removeAttribute('style');
				var childNodes = node.children;
				if (childNodes && childNodes.length > 0) {
					for (var i = 0; i < childNodes.length; ++i) {
						RemoveStyle(childNodes[i]);
					}
				}
			}
			
			//IE 8 Console.log fix
			if (typeof console === "undefined" || typeof console.log === "undefined") {
				console = {"log":function(){}};
			}
			
			$export.BuildAll(tableOrId);
			return $export;
		};
	});
}(typeof define === 'function' && define.amd ? define : function (deps, factory) {
	if (typeof module !== 'undefined' && module.exports) { //Node
		module.exports = factory();
	} else {
		window['Dable'] = factory();
	}
}));
