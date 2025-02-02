<!doctype html public "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title> Select an image </title>
<meta name="Generator" content="EditPlus">
<meta name="Author" content="">
<meta name="Keywords" content="">
<meta name="Description" content="">
<script type="text/javascript" src="../../res/js/NxDialogControl.js"></script>
<script type="text/javascript" src="../../res/js/NxHTML.js"></script>
<script type="text/javascript" src="../../res/js/NxImageChooser.js"></script>
<style type="text/css" >
div.thumbnails {
	border:solid 1px black;
	width:150px;
	height:100%;
	overflow-y:scroll;
	overflow-x:hide;
}

img.thumbnail {
	border:solid 1px black;
}

div.preview {
	border:solid 1px black;
	width:400px;
	height:400px;
	background-color:#eeeeee;
}

img.preview {
	border:solid 1px black;
}

.button {
	border:solid 1px black;
	height:25px;
	width:60px;
	background-color:#eeeeee;
	text-align:center;
}

.button a {
	color:black;
	text-decoration:none;
}
</style>
</head>

<!-- script after head because template rewrite other scripts at the end of head -->
<script>
var thumbBox="100x100";
var previewBox="300x300";

// --------- configuration ---------
var sizeList = new SizeList(
	[
		{text:"original",value:""},
		{text:"100x100",value:"100x100"},
		{text:"200x200",value:"200x200"},
		{text:"300x300",value:"300x300"}
	]
);

var imageList = new ImageList(
	[
	/*
		{url:"images/wdoctor1.jpg"},
		{url:"images/wdoctor2.jpg"},
		{url:"images/venise.jpg"}
	*/
{records}
{record}
		{ {field:image:"url"} }{if: not record.is_last},{/if}
{/record}
{/records}
	
	],
	thumbBox,previewBox,sizeList
);

function getImageInfo() {
	var im=globals.ImagePreview;
	var box=im.getBox();
	var url=im.getUrl();
	var sizes=sizeList;
	var alignE=new NxSelect("align_img");
	var borderE=new NxInput("border_img");
	var border=borderE.getValue();
	border=parseInt(border);
	if (isNaN(border))
		border=0;

	return {url:url,size:sizes.getSize(),align:alignE.getValue(),border:border};
}

function resizeWin() {
	var box = new NxHtmlElt("dialogBox");
	var size = box.getSize();
	dialog.resizeTo(size.w+10, size.h+30);
}

</script>

<body onload="resizeWin()" SCROLL="NO">

<span id="dialogBox">
<form>
<table border="0">
<tr>
	<td valign="top">
		<div class="thumbnails"><script>imageList.display();</script></div>
	</td>
	<td align="left" valign="top" height="20">
		<table width="100%">
			<tr>
				<td>size:<script>sizeList.display();</script>
				align:<select id="align_img">
					<option value="inline">inline</option>
					<option value="top">top</option>
					<option value="left">left</option>
					<option value="middle">middle</option>
					<option value="right">right</option>
					<option value="bottom">bottom</option>
				</select>
				border:<input id="border_img" size="2" type="text" value="0">
				</td>
			</tr>
		</table>
		<div class="preview"><script>imageList.displayPreview(0);</script></div>
	</td>
</tr>
<tr>
	<td colspan="2" valign="top"align="right">
		<table>
			<tr>
				<td align="right" class="button"><a href="#">Add</a></td>
				<td align="right"><input type="file"></td>
				<td align="right" width="100">&nbsp;&nbsp;&nbsp;</td>
				<td align="right" class="button"><a href="#" onclick="window.close()">Cancel</a></td>
				<td align="right" class="button"><a href="#" onclick="dialog.returnValue(getImageInfo())">Select</a></td>
			</tr>
		</table>
	</td>
</tr>
</table>

</form>
</span>

<!-- div id="trace" style="height:100px;overflow:scroll">trace:</div -->

</body>
</html>