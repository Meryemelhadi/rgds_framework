<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_lists_manage
{
	function NxControl_lists_manage()
	{
	}
	
	function toHTML()
	{
	}
	
	function toForm($value,&$desc)
	{
		// get choice list array (as "value" => "label")
		$list =& $desc->getList();
		$listArray =& $list->getListAsArray();
		$prompt=$desc->getPrompt();
		$prompt_value=$desc->getProperty('prompt_value','?',false);

		$style = $desc->getStyle("select","choice");
		$isMultiple=$desc->getProperty("multiple",false,false);
		// $fname = $desc->name;
		$fname = $desc->getAlias();
		
		$sel = array();
		if (isset($value))
		{
			$selected_options = explode($desc->sep,$value);
			foreach($selected_options as $option)
			{
				$sel[$option]=true;
			}
		}

		$msg=$desc->getString('Please select an option',null,'form_fields');
		$selection=$desc->getString('Selected values',null,'form_fields');
		$others=$desc->getString('Values you can add',null,'form_fields');
		$fun="_nxOption_$fname";
		
		static $script_added=false;
		if ($script_added)
			$script = '';
		else
		{
			$script_added=true;
			$script = <<<EOH
<script type="text/javascript">
			function manage_list_select(fname,event){
		var list1=document.getElementById(fname+'_1'),
		list2=document.getElementById(fname+'_2'),
		objRef=document.getElementById(fname),
		index2;

		// move all selected options
		do {
			index2=list2.options.selectedIndex;
			if(index2<0) return alert("Please select an option");
			list1.options[list1.options.length]=new Option(list2.options[index2].text,list2.options[index2].value);

			var len=objRef.options.length;
			objRef.options[len]=new Option(list2.options[index2].text,list2.options[index2].value);
			objRef.options[len].selected=true;

			list2.options[index2]=null;
		} while(list2.options.selectedIndex>=0)

		manage_list_sortList(list1);

		manage_list_message(fname);

		event.cancelBubble=true;
		event.returnValue=false;
		return false;
		}
	function manage_list_remove(fname,event){
		var list1=document.getElementById(fname+'_1'),
		list2=document.getElementById(fname+'_2'),
		objRef=document.getElementById(fname),
		index1;

		do {
			index1=list1.options.selectedIndex;
			if(index1<0) return alert("Please select an option");
			list2.options[list2.options.length]=new Option(list1.options[index1].text,list1.options[index1].value);

			var len=objRef.options.length;
			objRef.options[index1]=null;
			
			list1.options[index1]=null;
		} while(list1.options.selectedIndex>=0)

		manage_list_sortList(list2);
		manage_list_message(fname);

		event.cancelBubble=true;
		event.returnValue=false;
		return false;
		}

	function manage_list_add(id,fname,event) {
		var objRef=document.getElementById(fname);
		var list1=document.getElementById(fname+'_1');
		var list2=document.getElementById(fname+'_2');

		var valf=document.getElementById(id);
		var val=valf.value.replace(/^\s+|\s+$/g, "");

		if(val=="") return alert("Please add an option");
		valf.value='';
		
		// check if liste 2 contains exactly the value to add
		var add=true;
		var len=list2.options.length;
		if ((i=manage_list_isInList(val,list2,true))>=0) {
			manage_list_select(fname,event);
			add=false;
		}
		else if ((i=manage_list_isInList(val,list1,true))>=0) {
			manage_list_remove(fname,event);
			add=false;
		}

		// add new value ?
		if (add && confirm("Do you want to add '"+val+"' to the database?"))
		{
			// add to list 1
			var len=list1.options.length;
			list1.options[len]=new Option(val,val);

			// add to reference list
			var len=objRef.options.length;
			objRef.options[len]=new Option(val,val);
			objRef.options[len].selected=true;

			manage_list_message(fname);
		}
		
		// reset liste 2 filter
		manage_list_filter(id,fname,null,false);

		// sort list
		manage_list_sortList(list1);

		event.cancelBubble=true;
		event.returnValue=false;
		return false;
	}

	var manage_list_filter_bag=new Array();
	function manage_list_filter(id,fname,event,capture) {
		if (typeof(manage_list_filter_bag[fname])=="undefined")
			manage_list_filter_bag[fname]=new Array();

		var list1=document.getElementById(fname+'_1');
		var list2=document.getElementById(fname+'_2');
		var add_button=document.getElementById('add_button_'+fname)
		var valf=document.getElementById(id);
		var val=valf.value.replace(/^\s+|\s+$/g, "");

		// filter current list
		var len=list2.options.length;
		for (var i=len-1;i>=0;i--)
		{
			var option=list2.options[i];
			var v=option.text;
			if (val!='' && val!=null && !v.toUpperCase().match("^"+val.toUpperCase()))
			{
				// remove from list, add to bag
				var o={value:option.value,text:option.text};				
				manage_list_filter_bag[fname][manage_list_filter_bag[fname].length]=o;
				list2.options[i]=null;
			}
		}

		// check bag
		var len=manage_list_filter_bag[fname].length;
		for (i=len-1;i>=0;i--)
		{
			var option=manage_list_filter_bag[fname][i];
			if (option!=null && (val=='' || option.value.toUpperCase().match("^"+val.toUpperCase())))
			{
				// add to list, remove from bag
				list2.options[list2.options.length]=new Option(option.text,option.value);
				manage_list_filter_bag[fname][i]=null;
			}
		}

		if ((i=manage_list_isInList(val,list2,true))>=0) {
			add_button.value='<--';
			manage_list_message(fname,'select',list2[i].text);
		}
		else if ((i=manage_list_isInList(val,list1,true))>=0) {
			manage_list_message(fname,'remove',list1[i].text);
			add_button.value='-->';
		}
		else if (val!=''){
			manage_list_message(fname,'add',val);
			add_button.value='  +  ';
		}
		else {
			manage_list_message(fname);
		}

		if (list2.length==1) {
			// list not empty, sort it and hide add button
			manage_list_sortList(list2);
		}
		else
			manage_list_sortList(list2);

		if(val=='')
			add_button.style.display='none';
		else
			add_button.style.display='';

		if (capture){
			event.cancelBubble=true;
			event.returnValue=false;
			return false;
		}
		return true;
	}

	function manage_list_isInList(val,list,select) {
		var len=list.length;
		for (i=0;i<len;i++)
		{
			list[i].selected=false;
			if (list[i].text.toUpperCase()==val.toUpperCase())
			{
				if(select)
					list[i].selected=true;
				return i;
			}

		}
		return -1;
	}

	function manage_list_sortList(list) {
		if (list==null)	return;

		var compare= function(a,b) {
			if (a<b) {return -1;}
			if (a>b) {return 1;}
			return 0;
		}

		// sort option array
		var tmp=new Array();
		var len=list.options.length;
		for (var i=len-1;i>=0;i--)
		{
			var option=list.options[i];
			tmp[tmp.length]={value:option.value,text:option.text,selected:option.selected};
			list[i]=null;
		}
		var sorted=tmp.sort(function(a,b){return compare(a.text.toUpperCase(),b.text.toUpperCase());});

		// rebuild the list
		var len=sorted.length;
		for (i=0;i<len;i++)
		{
			var option=sorted[i];
			list[i]=new Option(option.text,option.value);
			if (option.selected)
				list[i].selected=true;
		}
		tmp=sorted=null;
		return list;
	}
	function manage_list_message(fname,msg,w) {
		var s;
		if (msg=='select') s= 'Click on "<strong>&lt;--</strong>" to add highlited value(s) <strong>'+w+'</strong>.<br/><br/>';
		else if (msg=='remove') s= 'Click on <strong>"--&gt;"</strong> to remove highlited values from the selection.<br/><br/>';
		else if (msg=='add') s='Click on "+" to add value "<strong>'+w+'</strong>" to your selection and to the database.';
		else s='Select an option in right list and click on <strong>"&lt;--"</strong> button to add it to selection.<br/>'+
		'Select an option in left list and click on <strong>"--&gt;"</strong> button to remove it to selection.</br>'+
		'Use text box on right side to filter values in right list or add a new value in list.';

		var msg=document.getElementById('option_message_'+fname);
		msg.innerHTML=s;
	}
</script>
EOH;
	
		}
	
		if ($isMultiple)
			$isMultiple = " multiple=\"true\"";
		else
			$isMultiple = "";

		$stylelayout = $desc->getProperty("style","height:150px;width:200px;");
		$stylelayout2 = $desc->getProperty("style","width:200px;");
		
		$list=$list1=$list2='';
		foreach (array_keys($listArray) as $k) 
		{
			$option="<option value=\"$k\">".htmlentities($listArray[$k])."</option>";
			if (isset($sel[$k]))
			{
				$list1 .= $option;
				$list .="<option value=\"$k\" selected=\"true\">".htmlentities($listArray[$k])."</option>";
			}	
			else
				$list2 .= $option;
		}
		
		return <<<EOH
{$script}<table>
<tr>
	<td style="vertical-align:top">$selection</td>
	<td style="vertical-align:top"><br/><input type="button" id="add_button_{$fname}" style="display:none" onclick="return manage_list_add('filter_val_{$fname}','{$fname}',event)" value=" + "></td>
	<td style="vertical-align:top">$others<br/>
		<div>
			<input type="text" style="width:200px;" onKeyUp="return manage_list_filter('filter_val_{$fname}','{$fname}',event,false)" onchange="return manage_list_filter('filter_val_{$fname}','{$fname}',event,true)" name="filter_val_{$fname}" id="filter_val_{$fname}">
		</div>
	</td>
</tr>
<tr>
<td style="vertical-align:top">
<select class="select" {$isMultiple} 
	name="{$fname}_1[]" id="{$fname}_1" style="$stylelayout;" ondoubleclick="return manage_list_remove('{$fname}',event)">
$list1</select>
</td>
<td style="vertical-align:middle;padding-top:0px;">
<input type="button" onclick="return manage_list_select('{$fname}',event)" value="<--" title="add to selection"><br/>
<input type="button" onclick="return manage_list_remove('{$fname}',event)" value="-->" title="remove from selection">
</td>
<td style="vertical-align:top">
<select class="select" {$isMultiple} 
	name="{$fname}_2[]" id="{$fname}_2" style="$stylelayout" ondoubleclick="return manage_list_select('{$fname}',event)">
$list2
</select><select class="select"  multiple="true" 
	name="{$fname}[]" id="{$fname}" style="visibilityx:hidden;display:none;">$list</select>
</td></tr>
<tr><td colspan="3" style="font-size:8pt;font-style:italic;height:45px;" id="option_message_{$fname}"><script>manage_list_message('{$fname}');</script></td></tr>
</table>
EOH;

	}

	function readForm(&$recData,&$desc)
	{
		// get values as array
		$fname = $desc->getAlias();
		
		if (!isset($recData[$fname]))
			return null;
		$values = $recData[$fname];
		
		// get choice list array (as "value" => "label")
		$list =& $desc->getList();
	
		// check if new values have been added in the form
		$new_choices=array();
		foreach (array_diff($values,array_keys($list->getListAsArray())) as $k)
		{
			$new_choices[$k]=$k;	
		}
		
		if (count($new_choices)>0)
		{
			// add new choices in DB
			$list->addChoices($new_choices);
			
			// refresh list cache
			$list->refreshList();
		}
		
		// set value for DB
		if (is_array($values))
			$value = implode($values,$desc->sep);
		else
			$value = $values;
			
		return $value;
	}
}

?>