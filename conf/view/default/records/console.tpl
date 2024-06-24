{!-- css file dependency --}
{@require:css/console.css:skin and device}

{!-- console main operations --}
{button:back} {button:newRecord}

{!-- records display --}
{records}
<table class="console" cellspacing="1">
{record}
{if:record.is_first}
<tr>
{fields}
	<th>{field.label}</th>
{/fields}
  	<th>operations</th>
	</tr>
{/if}
{if:record.is_even_index}
	<tr class="row_even">
{else}
	<tr class="row_odd">
{/if}
{fields}
	<td>{field}</td>
{/fields}
<td class="operations">
{button:viewRecord:record.url_key}&nbsp;{button:editRecord:record.url_key}&nbsp;{button:deleteRecord:record.url_key}
</td>
</tr>
{/record}
</table>
{/records}
{no_record}
	<table border="0" cellspacing="0" cellpadding="0" class="console">
		<tr><td>{string:"Empty table"}</td></tr>
	</table>
{/no_record}