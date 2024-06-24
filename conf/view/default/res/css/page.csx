@include(css/variables,skin and device)
@include(css/button,skin and device)

body {
	background-color:#e7e7ce;
	color:#000000;
	overflow:hidden;
}

body,p,td,div {
	color:#333333;
	font-family:Arial, Helvetica, sans-serif;
	font-size:small;
	font-weight:normal;
}

div.mainBox
{
	height:100%; 
	width:100%; 
	border:solid 1 black;
	overflow:auto;
	background-color:#ffffff;
}

div.mainBox .siteBanner
{
	text-align:center;
	background-color:@variable(banner_bgCol);
	color:@variable(banner_fgCol);
	font-size:large;
}

div.mainBox h2
{
	color:@variable(h2_col)
}

div.topmenu a
{
	color:@variable(a_col);
}

div.content {
	margin:10px;
}